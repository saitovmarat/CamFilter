#pragma once

#include "ICameraType.h"

#include <QMainWindow>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ICameraType* camera, QWidget *parent = nullptr);
    ~MainWindow();

private slots:   
    void getCameras();
    void selectCam(int index);

    void applyBilateralFilter();
    void applyGaussFilter();
    void applyNoFilter();

private:
    Ui::MainWindow* ui;
    ICameraType* camera;

    void setFilterFromSettings();
    FilterType currentFilter;

    QWidget* cameraWidget;
};
