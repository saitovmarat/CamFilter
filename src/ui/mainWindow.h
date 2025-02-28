#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <queue>
#include <mutex>
#include <QMainWindow>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QCamera>
#include <QVideoWidget>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <opencv2/opencv.hpp>

#include <QLabel>
#include <QTimer>

enum FilterType {
    NoFilter,
    Bilateral,
    Gauss
};

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void applyBilateralFilter();
    void applyGaussFilter();
    void applyNoFilter();

#ifdef USE_QT_CAMERA
    void getCameras();
    void selectCam();
    void onFrameChanged(const QVideoFrame& frame);

#else
    void updateFrame();
#endif
private:
    Ui::MainWindow *ui;
    FilterType currentFilter;

#ifdef USE_QT_CAMERA
    QVideoWidget* cameraWidget;
    QCamera* currentCam;
    QVideoSink* sink;
    QMediaCaptureSession* mediaCaptureSession;

    void processFrames();
    QVideoFrame applyFilter(const QVideoFrame &frame);

    std::queue<QVideoFrame> frameQueue;
    std::mutex queueMutex;
    std::condition_variable frameAvailable;
    std::atomic<bool> stopProcessing;
    std::thread processingThread;
#else
    cv::VideoCapture camera;
    QTimer* timer;
    QLabel* cameraWidget;
#endif

};

#endif // MAINWINDOW_H
