#include "openCVCamera.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>

OpenCVCamera::OpenCVCamera(QObject* parent)
    : ICameraType(parent)
    , stopProcessing(true)
{
    currentFilter = getFilterFromSettings();
}

OpenCVCamera::~OpenCVCamera()
{
    stopCamera();
    camera.release();
}

void OpenCVCamera::selectCam(int index)
{
    stopCamera();
    if (camera.isOpened()) {
        camera.release();
    }
    if(index == 0){
        return;
    }

    if (camera.open(index-1, cv::CAP_DSHOW)) {
        stopProcessing = false;
        processingThread = std::thread(&OpenCVCamera::processFrames, this);
    } else {
        qWarning() << "OpenCVCamera::selectCam - Falied to open camera with index " << index;
    }
}

void OpenCVCamera::processFrames()
{
    cv::Mat frame;
    if (!camera.isOpened()) {
        qWarning() << "OpenCVCamera::processFrames - Falied to open the camera!";
        return;
    }

    while (!stopProcessing) {
        camera >> frame;
        if (frame.empty()) {
            qWarning() << "OpenCVCamera::processFrames - Got an empty frame!";
            continue;
        }

        cv::Mat processedFrame = getFilteredFrame(frame);
        if (processedFrame.empty()) {
            qWarning() << "OpenCVCamera::processFrames - Failed to get filtered frame!";
            continue;
        }

        QImage convertedFrame = MatToQImage(processedFrame);
        if (convertedFrame.isNull()) {
            qWarning() << "OpenCVCamera::processFrames - Failed to get converted frame! processedFrame.type:" << processedFrame.type();
            continue;
        }

        QImage lastFrame = convertedFrame;
        if (lastFrame.isNull()) {
            qWarning() << "OpenCVCamera::processFrames - Failed to convert lastFrame to RGB_32! No signal was emitted!";
        }

        emit frameReady(lastFrame);
    }
}

cv::Mat OpenCVCamera::getFilteredFrame(const cv::Mat& frame)
{
    cv::Mat processedFrame;
    switch (currentFilter) {
    case FilterType::Bilateral:
        cv::bilateralFilter(frame, processedFrame, 9, 75, 75);
        break;
    case FilterType::Gauss:
        cv::GaussianBlur(frame, processedFrame, cv::Size(15, 15), 0);
        break;
    default:
        processedFrame = frame.clone();
        break;
    }

    return processedFrame;
}

void OpenCVCamera::setFilter(FilterType filter)
{
    std::lock_guard<std::mutex> lock(framesMutex);
    currentFilter = filter;
}

void OpenCVCamera::stopCamera() {
    if (!stopProcessing) {
        stopProcessing = true;
        if (processingThread.joinable()) {
            processingThread.join();
        }
    }
}
