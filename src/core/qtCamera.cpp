#include <QVideoWidget>
#include <QCamera>
#include <QVideoSink>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <opencv2/opencv.hpp>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include "qtCamera.h"

QtCamera::QtCamera(QObject* parent)
    : ICameraType(parent)
    , camera(new QCamera())
    , mediaCaptureSession(new QMediaCaptureSession())
    , stopProcessing(false)
    , widget(new QVideoWidget())
{
    qDebug() << "Qt Camera";
    currentFilter = getFilterFromSettings();
    processingThread = std::thread(&QtCamera::processFrames, this);
}

QtCamera::~QtCamera() 
{
    stopProcessing = true;
    frameAvailable.notify_all();
    if (processingThread.joinable()) {
        processingThread.join();
    }

    delete camera;
    delete mediaCaptureSession;
}

void QtCamera::selectCam(int index)
{
    if (camera->isActive()) {
        camera->stop();
    }

    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    const QCameraDevice& selectedCam = cameras.at(index - 1);

    camera->setCameraDevice(selectedCam);
    mediaCaptureSession->setCamera(camera);
    mediaCaptureSession->setVideoOutput(widget);
    sink = mediaCaptureSession->videoSink();

    connect(sink, &QVideoSink::videoFrameChanged, this, &QtCamera::onFrameChanged);

    camera->start();
}

void QtCamera::start()
{
    selectCam(1);
}

void QtCamera::onFrameChanged(const QVideoFrame& frame)
{
    if (!frame.isValid()) {
        qWarning() << "Failed to get a QVideoFrame!";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        frameQueue.push(frame);
    }

    frameAvailable.notify_one();
}


void QtCamera::processFrames()
{
    while(!stopProcessing) {
        QVideoFrame frame;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            frameAvailable.wait(lock, [this] { return !frameQueue.empty() || stopProcessing; });

            if (stopProcessing && frameQueue.empty()) {
                break;
            }

            frame = frameQueue.back();
            while (!frameQueue.empty()) {
                frameQueue.pop();
            }
        }

        cv::Mat processedFrame = getFilteredFrame(frame);
        saveFrame(processedFrame);
    }
}

cv::Mat QtCamera::getFilteredFrame(const QVideoFrame &frame)
{ 
    QImage frameImg = frame.toImage();
    if (frameImg.isNull()) {
        qWarning() << "Failed to convert QVideoFrame to QImage!";
        return cv::Mat();
    }

    QImage::Format startingFormat = frameImg.format();
    if (startingFormat == QImage::Format_RGBA8888 ||
        startingFormat == QImage::Format_RGBA8888_Premultiplied) {
        frameImg = frameImg.convertToFormat(QImage::Format_RGB888);
    }

    cv::Mat mat;
    if (frameImg.format() == QImage::Format_RGB888) {
        mat = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC3,
                      const_cast<uchar*>(frameImg.constBits()),
                      static_cast<size_t>(frameImg.bytesPerLine()));
    } else {
        qWarning() << "Unsupported image format for OpenCV processing";
        return mat;
    }

    if (currentFilter == NoFilter) return mat;

    cv::Mat processedFrame;
    switch (currentFilter) {
    case Bilateral:
        cv::bilateralFilter(mat, processedFrame, 9, 75, 75);
        break;
    case Gauss:
        cv::GaussianBlur(mat, processedFrame, cv::Size(15, 15), 0);
        break;
    default:
        processedFrame = mat.clone();
        break;
    }

    return processedFrame;
}

void QtCamera::saveFrame(const cv::Mat& frame)
{
    static int frameCounter = 1;
    std::string folderPath = "../../frames/";
    std::string fileName = folderPath + "frame_" + std::to_string(frameCounter++) + ".jpg";
    cv::imwrite(fileName, frame);
}
