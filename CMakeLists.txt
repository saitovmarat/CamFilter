cmake_minimum_required(VERSION 3.16)

project(CamFilter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets Multimedia MultimediaWidgets)
find_package(OpenCV REQUIRED)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/mainWindow.cpp
    src/mainWindow.h
    src/mainWindow.ui
    main.cpp
)

include_directories(src)

qt_add_resources(RESOURCES resources/resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CamFilter
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCES}
    )
else()
    add_executable(CamFilter
        ${PROJECT_SOURCES}
        ${RESOURCES}
    )
endif()

target_link_libraries(CamFilter PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    ${OpenCV_LIBRARIES}
)

set_target_properties(CamFilter PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    set_target_properties(CamFilter PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CamFilter
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CamFilter)
endif()
