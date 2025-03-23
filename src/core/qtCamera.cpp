#include "qtCamera.h"

#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QVideoSink>
#include <QMediaDevices>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>

QtCamera::QtCamera(QObject* parent)
    : ICameraType(parent)
    , camera(new QCamera())
    , mediaCaptureSession(new QMediaCaptureSession())
    , stopProcessing(false)
    , widget(new QVideoWidget())
{
    currentFilter = getFilterFromSettings();
    processingThread = std::thread(&QtCamera::processFrames, this);
}

QtCamera::~QtCamera()
{
    if (!stopProcessing) {
        stopProcessing = true;
        frameAvailable.notify_all();
        if (processingThread.joinable()) {
            processingThread.join();
        }
    }
    delete camera;
    delete mediaCaptureSession;
}

void QtCamera::selectCam(int index)
{
    if (camera->isActive()) {
        camera->stop();
    }
    if(index == 0){
        if (!stopProcessing) {
            stopProcessing = true;
            frameAvailable.notify_all();
            if (processingThread.joinable()) {
                processingThread.join();
            }
        }
        return;
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

void QtCamera::onFrameChanged(const QVideoFrame& frame)
{
    if (!frame.isValid()) {
        qWarning() << "QtCamera::onFrameChanged - Failed to get a QVideoFrame!";
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
        if (processedFrame.empty()) {
            qWarning() << "QtCamera::processFrames - Failed to get filtered frame!";
            continue;
        }

        QImage convertedFrame = MatToQImage(processedFrame);
        if (convertedFrame.isNull()) {
            qWarning() << "QtCamera::processFrames - Failed to get converted frame! processedFrame.type: " << processedFrame.type();
            continue;
        }

        QImage lastFrame = convertedFrame;
        if (!lastFrame.isNull()) {
            emit frameReady(lastFrame);
        } else {
            qWarning() << "QtCamera::processFrames - Failed to convert lastFrame to RGB_32! No signal was emitted!";
        }

    }
}

cv::Mat QtCamera::getFilteredFrame(const QVideoFrame &frame)
{ 
    QImage frameImg = frame.toImage();
    if (frameImg.isNull()) {
        qWarning() << "QtCamera::getFilteredFrame - Failed to convert QVideoFrame to QImage!";
        return cv::Mat();
    }

    QImage::Format startingFormat = frameImg.format();
    cv::Mat mat;
    if (startingFormat == QImage::Format_RGBA8888 ||
        startingFormat == QImage::Format_RGBA8888_Premultiplied)
    {
        frameImg = frameImg.convertToFormat(QImage::Format_RGB888);
        mat = cv::Mat(frameImg.height(), frameImg.width(), CV_8UC3,
                      const_cast<uchar*>(frameImg.constBits()),
                      static_cast<size_t>(frameImg.bytesPerLine()));
    } else {
        qWarning() << "QtCamera::getFilteredFrame - Got unsupported image format for OpenCV processing";
        return mat;
    }

    cv::Mat processedFrame;
    switch (currentFilter) {
        case FilterType::Bilateral:
            cv::bilateralFilter(mat, processedFrame, 9, 75, 75);
            break;
        case FilterType::Gauss:
            cv::GaussianBlur(mat, processedFrame, cv::Size(15, 15), 0);
            break;
        default:
            processedFrame = mat.clone();
            break;
    }
    cv::cvtColor(processedFrame, processedFrame, cv::COLOR_RGB2BGR);
    return processedFrame;
}

void QtCamera::setFilter(FilterType filter)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    currentFilter = filter;
}

