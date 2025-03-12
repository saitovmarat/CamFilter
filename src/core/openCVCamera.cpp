#include "openCVCamera.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>

OpenCVCamera::OpenCVCamera(QObject* parent)
    : ICameraType(parent)
{
    currentFilter = getFilterFromSettings();
}

OpenCVCamera::~OpenCVCamera() 
{
    camera.release();
}

void OpenCVCamera::selectCam(int index)
{
    if (camera.isOpened()) {
        camera.release();
    }

    if(index == 0) return;

    camera.open(index-1, cv::CAP_DSHOW);
}

void OpenCVCamera::start()
{
    selectCam(1);
    if (!camera.isOpened()) {
        qDebug() << "Ошибка: Не удалось открыть камеру!";
        exit(1);
    }
    cv::Mat frame;
    while (true) {
        camera >> frame;
        if (frame.empty()) break;

        cv::Mat processedFrame = getFilteredFrame(frame);
        saveFrame(processedFrame);
    }
}

cv::Mat OpenCVCamera::getFilteredFrame(const cv::Mat& frame)
{
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

    return processedFrame;
}

void OpenCVCamera::saveFrame(const cv::Mat& frame)
{
    static int frameCounter = 1;
    std::string folderPath = "../../frames/";
    std::string fileName = folderPath + "frame_" + std::to_string(frameCounter++) + ".jpg";
    cv::imwrite(fileName, frame);
}
