#include "mainWindow.h"
#include "ui_mainWindow.h"

#include <QCameraDevice>
#include <QMediaDevices>
#include <QSettings>
#include <QThread>
#include <QMetaObject>
#include <opencv2/opencv.hpp>
#include <QBuffer>
#include <QFile>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>

MainWindow::MainWindow(ICameraType* cameraType, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(cameraType)
    , currentFilter(FilterType::NoFilter)
    , isRunning(true)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setWindowTitle("Camera Filter");
    setWindowIcon(QIcon(":/camera_icon.png"));

    addCamsToCB();
    connect(ui->camerasComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::selectCam);

    connect(ui->BilateralFilterButton, &QPushButton::clicked, this, &MainWindow::applyBilateralFilter);
    connect(ui->GaussFilterButton, &QPushButton::clicked, this, &MainWindow::applyGaussFilter);
    connect(ui->NoFilterButton, &QPushButton::clicked, this, &MainWindow::applyNoFilter);

    connect(camera, &ICameraType::frameReady, this, &MainWindow::onFrameReady, Qt::QueuedConnection);
    connect(this, &MainWindow::sendFrameToServer, this, &MainWindow::fetchImageFromServer);

    processingThread = std::thread(&MainWindow::processFrames, this);
}

MainWindow::~MainWindow()
{
    isRunning = false;
    frameCondition.wakeOne();
    if (processingThread.joinable()) {
        processingThread.join();
    }

    delete camera;
    delete ui;
}

void MainWindow::addCamsToCB()
{
    ui->camerasComboBox->addItem("<None>");
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice& camera : cameras) {
        ui->camerasComboBox->addItem(camera.description());
    }
}

void MainWindow::selectCam(int index)
{
    camera->selectCam(index);
}

void MainWindow::applyBilateralFilter()
{
    camera->setFilter(FilterType::Bilateral);
}

void MainWindow::applyGaussFilter()
{
    camera->setFilter(FilterType::Gauss);
}

void MainWindow::applyNoFilter()
{
    camera->setFilter(FilterType::NoFilter);
}

void MainWindow::onFrameReady(const QImage& frame)
{
    QMutexLocker locker(&framesMutex);
    if (!frame.isNull()) {
        frames.push_back(frame);
        frameCondition.wakeOne();
    }
    else {
        qWarning() << "MainWindow::onFrameReady - Got empty frame!";
    }
}

void MainWindow::processFrames()
{
    while (isRunning) {
        QImage frame;
        {
            QMutexLocker locker(&framesMutex);
            frameCondition.wait(&framesMutex);
            if (!frames.empty()) {
                frame = frames.back();
                frames.clear();
            }
        }
        if (!frame.isNull()) {
            emit sendFrameToServer(frame);
        }
        else {
            qWarning() << "MainWindow::processFrames - Got an empty frame!";
        }
    }
}

void MainWindow::fetchImageFromServer(const QImage &frame) {
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QString imageFilePath = "image.jpg";
    frame.save(imageFilePath);

    QFile* file = new QFile(imageFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть файл";
        delete multiPart;
        return;
    }

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/jpeg");
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"image.jpg\""));
    imagePart.setBody(file->readAll());
    multiPart->append(imagePart);

    QUrl url("http://localhost:5000/upload");
    QNetworkRequest request(url);

    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply);

    QObject::connect(reply, &QNetworkReply::finished, this, [reply, file, this]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QImage responseImage;
            responseImage.loadFromData(responseData);

            QMetaObject::invokeMethod(this, [this, responseImage]() {
                ui->cameraLabel->setPixmap(QPixmap::fromImage(responseImage));
                ui->cameraLabel->setScaledContents(true);
                ui->cameraLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
            });
        } else {
            qDebug() << "Ошибка:" << reply->errorString();
        }
        file->close();
        reply->deleteLater();
        file->deleteLater();
    });
}
