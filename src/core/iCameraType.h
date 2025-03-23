#pragma once

#include <QObject>
#include <QSettings>
#include <QImage>
#include <opencv2/opencv.hpp>

enum class FilterType {
    Bilateral,
    Gauss,
    NoFilter
};

class ICameraType : public QObject {
    Q_OBJECT
public:
    explicit ICameraType(QObject* parent = nullptr) {}

    virtual ~ICameraType() = default;
    virtual void selectCam(int index) = 0;
    virtual void processFrames() = 0;
    virtual void setFilter(FilterType filter) = 0;

    FilterType currentFilter;

    FilterType getFilterFromSettings() {
        QString settingsPath = "../../settings.ini";
        QSettings settings(settingsPath, QSettings::IniFormat);
        QString defaultFilter = settings.value("Settings/DefaultFilter", "NoFilter").toString().toLower();

        FilterType filter = FilterType::NoFilter;
        if (defaultFilter == "bilateral") {
            filter = FilterType::Bilateral;
        }
        else if (defaultFilter == "gauss") {
            filter = FilterType::Gauss;
        }
        return filter;
    }

    void saveFrame(const cv::Mat& frame)
    {
        static int frameCounter = 1;
        std::string folderPath = "../../frames/";
        std::string fileName = folderPath + "frame_" + std::to_string(frameCounter++) + ".jpg";
        cv::imwrite(fileName, frame);
    }


    QImage MatToQImage(const cv::Mat& mat)
    {
        if (mat.empty()) {
            return QImage();
        }

        if (mat.type() == CV_8UC1) {
            return QImage(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_Grayscale8).convertToFormat(QImage::Format_RGB32);
        } else if (mat.type() == CV_8UC3) {
            cv::Mat rgbMat;
            cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
            return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step[0], QImage::Format_RGB888).convertToFormat(QImage::Format_RGB32);
        } else {
            return QImage();
        }
    }

signals:
    void frameReady(const QImage &frame);

};
