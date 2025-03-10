#pragma once
#include <QObject>

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
};
