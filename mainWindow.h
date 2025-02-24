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
    void getCameras();
    void selectCam();
    void onFrameChanged(const QVideoFrame& frame);

    void applyBilateralFilter();
    void applyGaussFilter();
    void applyNoFilter();

private:
    Ui::MainWindow *ui;

    QCamera* currentCam;
    QVideoSink* sink;
    QMediaCaptureSession* mediaCaptureSession;

    FilterType currentFilter;

    // Многопоточность
    std::queue<QVideoFrame> frameQueue; // Очередь кадров
    std::mutex queueMutex;             // Мьютекс для синхронизации
    std::condition_variable frameAvailable; // Условная переменная
    std::atomic<bool> stopProcessing;  // Флаг остановки потока
    std::thread processingThread;      // Поток обработки

    void processFrames();
    QVideoFrame applyFilter(const QVideoFrame &frame);


};

#endif // MAINWINDOW_H
