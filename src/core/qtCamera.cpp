#include "qtCamera.h"

QtCamera::QtCamera(QObject* parent)
    : ICameraType(parent)
    , currentCam(new QCamera())
    , mediaCaptureSession(new QMediaCaptureSession())
    , stopProcessing(false)
    , widget(new QVideoWidget())
{
    processingThread = std::thread(&QtCamera::processFrames, this);
}

QtCamera::~QtCamera() 
{
    stopProcessing = true;
    frameAvailable.notify_all();
    if (processingThread.joinable()) {
        processingThread.join();
    }

    delete currentCam;
    delete mediaCaptureSession;
}

void QtCamera::selectCam(int index)
{
    if (currentCam->isActive()) {
        currentCam->stop();
    }

    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    const QCameraDevice& camera = cameras.at(index - 1);

    currentCam->setCameraDevice(camera);
    mediaCaptureSession->setCamera(currentCam);
    mediaCaptureSession->setVideoOutput(widget);
    sink = mediaCaptureSession->videoSink();

    connect(sink, &QVideoSink::videoFrameChanged, this, &QtCamera::onFrameChanged);

    currentCam->start();
}

void QtCamera::onFrameChanged(const QVideoFrame& frame)
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


void QtCamera::processFrames()
{
    while(!stopProcessing) {
        QVideoFrame frame;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            frameAvailable.wait(lock, [this] { return !frameQueue.empty() || stopProcessing; });

            if (stopProcessing && frameQueue.empty()) {
                break;
            }

            frame = frameQueue.back();
            while (!frameQueue.empty()) {
                frameQueue.pop();
            }
        }

        QVideoFrame filteredFrame = applyFilter(frame);
        QMetaObject::invokeMethod(sink, [this, filteredFrame]() {
            sink->setVideoFrame(filteredFrame);
        });
    }
}

QVideoFrame QtCamera::applyFilter(const QVideoFrame &frame)
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

    QImage filteredImg(processedFrame.data,
                       processedFrame.cols,
                       processedFrame.rows,
                       static_cast<int>(processedFrame.step),
                       QImage::Format_RGB888);

    return QVideoFrame(filteredImg);
}

QWidget* QtCamera::getWidget() const {
    return widget;
}
