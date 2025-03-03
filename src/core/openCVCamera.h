#pragma once

#include <opencv2/opencv.hpp>
#include <QTimer>
#include <QLabel>

#include "ICameraType.h"

class OpenCVCamera : public ICameraType {
    Q_OBJECT

public:
  explicit OpenCVCamera(QObject* parent = nullptr);
  ~OpenCVCamera() override;

  void selectCam(int index) override;
  QWidget* getWidget() const override;

private:
  cv::VideoCapture camera;
  QTimer* timer;
  QLabel* widget;

  void updateFrame();
};
