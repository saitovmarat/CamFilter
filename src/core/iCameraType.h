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
    explicit ICameraType(QObject* parent = nullptr)
        : QObject(parent)
        , currentFilter(NoFilter) {}

    virtual ~ICameraType() = default;
    virtual void selectCam(int index) = 0;
    virtual QWidget* getWidget() const = 0;

    FilterType currentFilter;
};
