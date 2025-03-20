#pragma once

#include <opencv2/opencv.hpp>
#include <thread>
#include "iCameraType.h"

class OpenCVCamera : public ICameraType {
    Q_OBJECT

public:
    explicit OpenCVCamera(QObject* parent = nullptr);
    ~OpenCVCamera() override;

    void selectCam(int index) override;
    void stop() override;
    QImage getCurrentFrame() override;
    void setFilter(FilterType filter) override;

private:
    void processFrames() override;
    cv::Mat getFilteredFrame(const cv::Mat& frame);

    cv::VideoCapture camera;
    bool isRunning;
    std::mutex framesMutex;
    std::thread processingThread;

    QImage lastFrame;
};
