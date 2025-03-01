#pragma once

#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QVideoWidget>
#include <QVideoSink>
#include <queue>
#include <mutex>
#include <opencv2/opencv.hpp>

#include "ICameraType.h"

class QtCamera : public ICameraType {
    Q_OBJECT

public:
    explicit QtCamera(QObject* parent = nullptr);
    ~QtCamera() override;

    void selectCam(int index) override;
    QWidget* getWidget() const override;

private:
    QVideoWidget* widget;
    void onFrameChanged(const QVideoFrame& frame);

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


};
