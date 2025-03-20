#include "openCVCamera.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>

OpenCVCamera::OpenCVCamera(QObject* parent)
    : ICameraType(parent)
    , isRunning(false)
{
    currentFilter = getFilterFromSettings();
}

void OpenCVCamera::stop() {
    if (isRunning) {
        isRunning = false;
        if (processingThread.joinable()) {
            processingThread.join();
        }
    }
}

OpenCVCamera::~OpenCVCamera()
{
    stop();
    camera.release();
}

void OpenCVCamera::selectCam(int index)
{
    stop();
    if (camera.isOpened()) {
        camera.release();
    }
    if(index == 0){
        stop();
        return;
    }

    if (camera.open(index-1, cv::CAP_DSHOW)) {
        isRunning = true;
        processingThread = std::thread(&OpenCVCamera::processFrames, this);
    } else {
        qDebug() << "Не удалось открыть камеру с индексом:" << index;
    }
}

void OpenCVCamera::processFrames()
{
    cv::Mat frame;
    if (!camera.isOpened()) {
        qDebug() << "Не удалось открыть камеру";
        return;
    }

    while (isRunning) {
        camera >> frame;
        if (frame.empty()) {
            qDebug() << "Получен пустой кадр";
            continue;
        }

        cv::Mat processedFrame = getFilteredFrame(frame);
        if (processedFrame.empty()) {
            qDebug() << "Обработанный кадр пустой";
            continue;
        }

        lastFrame = MatToQImage(processedFrame);
        if (lastFrame.isNull()) {
            qDebug() << "Преобразованный кадр пустой";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(framesMutex);
            emit frameReady(lastFrame);
        }

        qDebug() << "Кадр обработан и сигнал испущен";
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

QImage OpenCVCamera::getCurrentFrame()
{
    std::lock_guard<std::mutex> lock(framesMutex);
    QImage lastFrame = frameImgsVector.back();
    frameImgsVector.clear();
    return lastFrame;
}

void OpenCVCamera::setFilter(FilterType filter)
{
    std::lock_guard<std::mutex> lock(framesMutex);
    currentFilter = filter;
}
