#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(ICameraType* cameraType, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(cameraType)
    , currentFilter(NoFilter)
    , cameraWidget(camera->getWidget())
{
    ui->setupUi(this);
    setWindowTitle("Camera Filter");
    setWindowIcon(QIcon(":/camera_icon.png"));

    ui->verticalLayout->addWidget(cameraWidget);
    setFilterFromSettings();

    getCameras();
    connect(ui->camerasComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::selectCam);

    connect(ui->BilateralFilterButton, &QPushButton::clicked, this, &MainWindow::applyBilateralFilter);
    connect(ui->GaussFilterButton, &QPushButton::clicked, this, &MainWindow::applyGaussFilter);
    connect(ui->NoFilterButton, &QPushButton::clicked, this, &MainWindow::applyNoFilter);
}

MainWindow::~MainWindow()
{
    delete camera;
    delete cameraWidget;
    delete ui;
}

void MainWindow::getCameras()
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
    camera->currentFilter = Bilateral;
}

void MainWindow::applyGaussFilter()
{
    camera->currentFilter = Gauss;
}

void MainWindow::applyNoFilter()
{
    camera->currentFilter = NoFilter;
}

void MainWindow::setFilterFromSettings()
{
    QString settingsPath = QCoreApplication::applicationDirPath() + "/settings.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);
    QString defaultFilter = settings.value("Settings/DefaultFilter", "NoFilter").toString();

    if (defaultFilter == "Bilateral") {
        applyBilateralFilter();
    }
    else if (defaultFilter == "Gauss") {
        applyGaussFilter();
    }
    else {
        applyNoFilter();
    }
}
