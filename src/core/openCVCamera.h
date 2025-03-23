#pragma once

#include "iCameraType.h"

#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>

class OpenCVCamera : public ICameraType {
    Q_OBJECT

public:
    explicit OpenCVCamera(QObject* parent = nullptr);
    ~OpenCVCamera() override;

    void selectCam(int index) override;
    void setFilter(FilterType filter) override;

private:
    void processFrames() override;
    cv::Mat getFilteredFrame(const cv::Mat& frame);

    cv::VideoCapture camera;
    std::atomic<bool> stopProcessing;
    std::mutex framesMutex;
    std::thread processingThread;
};
