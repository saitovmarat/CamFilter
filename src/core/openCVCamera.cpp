#include "openCVCamera.h"

OpenCVCamera::OpenCVCamera(QObject* parent)
    : ICameraType(parent)
    , timer(new QTimer())
    , widget(new QLabel())
{
    widget->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Expanding);
    widget->setAlignment(Qt::AlignCenter);
    connect(timer, &QTimer::timeout, this, &OpenCVCamera::updateFrame);
    timer->start(30);
}

OpenCVCamera::~OpenCVCamera() 
{
    camera.release();
    delete timer;
    delete widget;
}

void OpenCVCamera::selectCam(int index)
{
    if (camera.isOpened()) {
        camera.release();
    }

    if(index == 0) return;

    camera.open(index-1, cv::CAP_DSHOW);

}

void OpenCVCamera::updateFrame()
{
    cv::Mat frame;
    camera >> frame;
    if (frame.empty()) return;

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    cv::Mat processedFrame;
    switch (currentFilter) {
    case Bilateral:
        cv::bilateralFilter(frame, processedFrame, 9, 75, 75);
        break;
    case Gauss:
        cv::GaussianBlur(frame, processedFrame, cv::Size(15, 15), 0);
        break;
    default:
        processedFrame = frame.clone();
        break;
    }

    QImage image(processedFrame.data,
                 processedFrame.cols,
                 processedFrame.rows,
                 static_cast<int>(processedFrame.step),
                 QImage::Format_RGB888);

    // QSize labelSize = cameraWidget->size(); 
    // QImage scaledImage = image.scaled(labelSize, 
    //                                   Qt::KeepAspectRatio, 
    //                                   Qt::SmoothTransformation);

    widget->setPixmap(QPixmap::fromImage(image));
}

QWidget* OpenCVCamera::getWidget() const {
    return widget;
}

