#pragma once

#include "iCameraType.h"

#include <QMainWindow>
#include <QWaitCondition>
#include <QMutex>

class QNetworkReply;
class QNetworkAccessManager;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ICameraType* camera, QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void sendFrameToServer(const QImage& frame);

private slots:
    void onFrameReady(const QImage& frame);

private:
    void addCamsToCB();
    void selectCam(int index);
    void applyBilateralFilter();
    void applyGaussFilter();
    void applyNoFilter();
    void processFrames();
    void fetchImageFromServer(const QImage& frame);

    Ui::MainWindow* ui;
    ICameraType* camera;
    FilterType currentFilter;

    std::vector<QImage> frames;
    std::atomic<bool> isRunning;
    QMutex framesMutex;
    std::thread processingThread;
    QWaitCondition frameCondition;

    QNetworkAccessManager* manager;
};
