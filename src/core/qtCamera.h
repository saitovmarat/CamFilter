#pragma once

#include <QVideoFrame>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include "iCameraType.h"

class QVideoWidget;
class QCamera;
class QVideoSink;
class QMediaCaptureSession;

class QtCamera : public ICameraType {
    Q_OBJECT

public:
    explicit QtCamera(QObject* parent = nullptr);
    ~QtCamera() override;

    void selectCam(int index) override;
    QWidget* getWidget() const override;

private:
    void onFrameChanged(const QVideoFrame& frame);
    void processFrames();
    QVideoFrame applyFilter(const QVideoFrame &frame);

    QVideoWidget* widget;
    QCamera* currentCam;
    QVideoSink* sink;
    QMediaCaptureSession* mediaCaptureSession;

    std::queue<QVideoFrame> frameQueue;
    std::mutex queueMutex;
    std::condition_variable frameAvailable;
    std::atomic<bool> stopProcessing;
    std::thread processingThread;
};
