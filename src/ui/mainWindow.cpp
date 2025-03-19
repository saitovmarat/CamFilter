#include "mainWindow.h"
#include "ui_mainWindow.h"

#include <QCameraDevice>
#include <QMediaDevices>
#include <QSettings>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(ICameraType* cameraType, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(cameraType)
    , currentFilter(NoFilter)
{
    ui->setupUi(this);
    setWindowTitle("Camera Filter");
    setWindowIcon(QIcon(":/camera_icon.png"));

    addCamsToCB();
    connect(ui->camerasComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::selectCam);

    connect(ui->BilateralFilterButton, &QPushButton::clicked, this, &MainWindow::applyBilateralFilter);
    connect(ui->GaussFilterButton, &QPushButton::clicked, this, &MainWindow::applyGaussFilter);
    connect(ui->NoFilterButton, &QPushButton::clicked, this, &MainWindow::applyNoFilter);

    connect(camera, &ICameraType::frameReady, this, &MainWindow::updateFrame);
}

MainWindow::~MainWindow()
{
    camera->stop();
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
    camera->setFilter(Bilateral);
}

void MainWindow::applyGaussFilter()
{
    camera->setFilter(Gauss);
}

void MainWindow::applyNoFilter()
{
    camera->setFilter(NoFilter);
}

void MainWindow::updateFrame()
{
    QImage frame = camera->getCurrentFrame();
    if (!frame.isNull()) {
        ui->cameraLabel->setPixmap(QPixmap::fromImage(frame));
        qDebug() << "Кадр обновлен";
    } else {
        qDebug() << "Получен пустой кадр в updateFrame";
    }
}
