#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentCam(new QCamera())
    , mediaCaptureSession(new QMediaCaptureSession())
    , currentFilter(NoFilter)
    , stopProcessing(false)
{
    ui->setupUi(this);

    getCameras();

    connect(ui->camerasComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::selectCam);

    connect(ui->BilateralFilterButton, &QPushButton::clicked, this, &MainWindow::applyBilateralFilter);
    connect(ui->GaussFilterButton, &QPushButton::clicked, this, &MainWindow::applyGaussFilter);
    connect(ui->NoFilterButton, &QPushButton::clicked, this, &MainWindow::applyNoFilter);

    processingThread = std::thread(&MainWindow::processFrames, this);
}

MainWindow::~MainWindow()
{
    stopProcessing = true;
    frameAvailable.notify_all(); // Уведомляем поток
    if (processingThread.joinable()) {
        processingThread.join(); // Ждем завершения потока
    }

    delete currentCam;
    delete mediaCaptureSession;
    delete ui;
}


void MainWindow::getCameras()
{
    ui->camerasComboBox->addItem("<None>");
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice& camera : cameras) {
        ui->camerasComboBox->addItem(camera.description());
    }
}

void MainWindow::selectCam()
{
    if (currentCam->isActive()) {
        currentCam->stop();
    }

    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice& camera : cameras) {
        if (camera.description() == ui->camerasComboBox->currentText()) {
            currentCam->setCameraDevice(camera);
            mediaCaptureSession->setCamera(currentCam);
            mediaCaptureSession->setVideoOutput(ui->cameraWidget);
            sink = mediaCaptureSession->videoSink();

            connect(sink, &QVideoSink::videoFrameChanged, this, &MainWindow::onFrameChanged);
            qDebug() << "Selected Cam: " << camera.description();

            currentCam->start();
            break;
        }
    }
}


void MainWindow::onFrameChanged(const QVideoFrame& frame)
{
    if (!frame.isValid()) {
        qWarning() << "Failed to get a QVideoFrame!";
        return;
    }


    {
        std::lock_guard<std::mutex> lock(queueMutex);
        frameQueue.push(frame);
    }

    frameAvailable.notify_one();
}

void MainWindow::processFrames()
{
    while(!stopProcessing) {
        QVideoFrame frame;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            frameAvailable.wait(lock, [this] { return !frameQueue.empty() || stopProcessing; });

            if (stopProcessing && frameQueue.empty()) {
                break;
            }

            frame = frameQueue.front();
            frameQueue.pop();
        }

        QVideoFrame filteredFrame = applyFilter(frame);
        QMetaObject::invokeMethod(this, [this, filteredFrame]() {
            sink->setVideoFrame(filteredFrame);
        });
    }
}

// Frame Pixel format: Format_NV12
// QImage format: QImage::Format_RGBA8888_Premultiplied

QVideoFrame MainWindow::applyFilter(const QVideoFrame &frame)
{
    if (currentFilter == NoFilter) return frame;

    QImage frameImg = frame.toImage();
    if (frameImg.isNull()) {
        qWarning() << "Failed to convert QVideoFrame to QImage!";
        return frame;
    }

    QImage::Format startingFormat = frameImg.format();
    if (startingFormat == QImage::Format_RGBA8888 ||
        startingFormat == QImage::Format_RGBA8888_Premultiplied) {
        frameImg = frameImg.convertToFormat(QImage::Format_RGB888);
    }

    // Создание cv::Mat из QImage
    cv::Mat mat;
    if (frameImg.format() == QImage::Format_RGB888) {
        mat = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC3,
                      const_cast<uchar*>(frameImg.constBits()),
                      static_cast<size_t>(frameImg.bytesPerLine()));
    } else {
        qWarning() << "Unsupported image format for OpenCV processing";
        return frame;
    }

    if (mat.empty()) {
        qWarning() << "cv::Mat is empty!";
        return frame;
    }

    // Применение фильтра
    cv::Mat processedFrame;
    switch (currentFilter) {
    case Bilateral:
        cv::bilateralFilter(mat, processedFrame, 9, 75, 75);
        break;
    case Gauss:
        cv::GaussianBlur(mat, processedFrame, cv::Size(15, 15), 0);
        break;
    default:
        processedFrame = mat.clone();
        break;
    }

    // Создание QImage из обработанного cv::Mat
    QImage filteredImg(processedFrame.data,
                       processedFrame.cols,
                       processedFrame.rows,
                       static_cast<int>(processedFrame.step),
                       QImage::Format_RGB888);

    return QVideoFrame(filteredImg);
}

void MainWindow::applyBilateralFilter()
{
    currentFilter = Bilateral;
}

void MainWindow::applyGaussFilter()
{
    currentFilter = Gauss;
}

void MainWindow::applyNoFilter()
{
    currentFilter = NoFilter;
}


