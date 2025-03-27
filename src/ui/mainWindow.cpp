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
            // fetchImageFromServer(frame);
            frame = frame.mirrored(true, false);
            QMetaObject::invokeMethod(this, [this, frame]() {
                ui->cameraLabel->setPixmap(QPixmap::fromImage(frame));
                ui->cameraLabel->setScaledContents(true);
                ui->cameraLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
            });
        }
        else {
            qWarning() << "MainWindow::processFrames - Got an empty frame!";
        }
    }
}

void MainWindow::fetchImageFromServer(const QImage& frame)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::onImageReceived);

    QNetworkRequest request(QUrl("http://127.0.0.1:5000/upload"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG");
    qDebug() << "Sending POST request with data size:" << imageData.size();

    manager->post(request, imageData);
}


void MainWindow::onImageReceived(QNetworkReply* reply)
{
    qDebug() << "Entered - MainWindow::onImageReceived";
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();

        QImage img = QImage::fromData(imageData);
        if (img.isNull()) {
            qWarning() << "MainWindow::onImageReceived - Failed to convert QByteArray to QImage";
            return;
        }

        img = img.mirrored(true, false);
        QMetaObject::invokeMethod(this, [this, img]() {
            ui->cameraLabel->setPixmap(QPixmap::fromImage(img));
            ui->cameraLabel->setScaledContents(true);
            ui->cameraLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        });
    } else {
        qWarning() << "MainWindow::onImageReceived - Error fetching image:" << reply->errorString();
    }

    reply->deleteLater();
}



