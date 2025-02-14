#include "mainWindow.h"
#include "./ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QVBoxLayout* verticalLayout = new QVBoxLayout(ui->centralwidget);
    verticalLayout->setStretch(0, 1);
    verticalLayout->setStretch(1, 0);

    comboBox = new QComboBox();
    currentCam = new QCamera();
    videoWidget = new QVideoWidget();
    mediaCaptureSession = new QMediaCaptureSession();

    verticalLayout->addWidget(videoWidget);
    verticalLayout->addWidget(comboBox);

    getCameras();
    connect(comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::selectCam);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getCameras()
{
    comboBox->addItem("<None>");
    cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice& camera : cameras) {
        comboBox->addItem(camera.description());
    }
}

void MainWindow::selectCam()
{
    if (currentCam->isActive()) {
        currentCam->stop();
    }

    for (const QCameraDevice& camera : cameras) {
        if (camera.description() == comboBox->currentText()) {
            currentCam->setCameraDevice(camera);
            mediaCaptureSession->setCamera(currentCam);
            mediaCaptureSession->setVideoOutput(videoWidget);

            qDebug() << "Selected Cam: " << camera.description();

            currentCam->start();
            break;
        }
    }
}
