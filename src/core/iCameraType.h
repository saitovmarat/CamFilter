#pragma once
#include <QObject>
#include <QSettings>

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
    virtual void start() = 0;

    FilterType currentFilter;

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

        qDebug() << "Current Filter: " << defaultFilter;
        return filter;
    }

};
