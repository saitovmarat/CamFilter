#include "mainWindow.h"
#include "ICameraType.h"
#include <QApplication>

#ifdef USE_QT_CAMERA
#include "qtCamera.h"
using CameraType = QtCamera;

#else
#include "openCVCamera.h"
using CameraType = OpenCVCamera;

#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w(new CameraType());
    w.show();
    return a.exec();
}
