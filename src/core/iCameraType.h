#pragma once

#include <QObject>
#include <QSettings>
#include <QImage>
#include <opencv2/opencv.hpp>

enum FilterType {
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
    virtual void stop() = 0;
    virtual QImage getCurrentFrame() = 0;
    virtual void setFilter(FilterType filter) = 0;

    FilterType currentFilter;
    std::vector<QImage> frameImgsVector;

    FilterType getFilterFromSettings() {
        QString settingsPath = "../../settings.ini";
        QSettings settings(settingsPath, QSettings::IniFormat);
        QString defaultFilter = settings.value("Settings/DefaultFilter", "NoFilter").toString().toLower();

        FilterType filter = NoFilter;
        if (defaultFilter == "bilateral") {
            filter = Bilateral;
        }
        else if (defaultFilter == "gauss") {
            filter = Gauss;
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
            qDebug() << "Входная матрица пустая";
            return QImage();
        }

        qDebug() << "Тип матрицы:" << mat.type() << ", Размеры:" << mat.cols << "x" << mat.rows;

        if (mat.type() == CV_8UC1) {
            qDebug() << "CV_8UC1\n";
            return QImage(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_Grayscale8);
        } else if (mat.type() == CV_8UC3) {
            cv::Mat rgb;
            cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
            qDebug() << "CV_8UC3\n";
            return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step[0], QImage::Format_RGB888);
        } else if (mat.type() == CV_8UC4) {
            cv::Mat rgba;
            cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
            qDebug() << "CV_8UC4\n";
            return QImage(rgba.data, rgba.cols, rgba.rows, rgba.step[0], QImage::Format_RGBA8888);
        }

        qDebug() << "Тип матрицы не поддерживается";
        return QImage();
    }

signals:
    void frameReady(const QImage &frame);

};
