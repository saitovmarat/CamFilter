#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QComboBox>
#include <QVBoxLayout>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>

QT_BEGIN_NAMESPACE
namespace Ui{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QList<QCameraDevice> cameras;
    QComboBox* comboBox;
    QCamera* currentCam;
    QVideoWidget* videoWidget;
    QMediaCaptureSession* mediaCaptureSession;

private slots:
    void selectCam();

private:
    Ui::MainWindow *ui;
    void getCameras();
};

#endif // MAINWINDOW_H
