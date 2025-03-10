#pragma once

#include <opencv2/opencv.hpp>
#include "iCameraType.h"

class OpenCVCamera : public ICameraType {
    Q_OBJECT

public:
    explicit OpenCVCamera(QObject* parent = nullptr);
    ~OpenCVCamera() override;

    void selectCam(int index) override;
    void start() override;

private:
    cv::VideoCapture camera;

    cv::Mat getFilteredFrame(const cv::Mat& frame);
    void saveFrame(const cv::Mat& frame);
};
