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
    main.cpp
    mainWindow.cpp
    mainWindow.h
    mainWindow.ui
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CamFilter
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        .gitignore
    )
else()
    add_executable(CamFilter
        ${PROJECT_SOURCES}
        .gitignore
    )
endif()

target_link_libraries(CamFilter PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    ${OpenCV_LIBRARIES}
)

# Установите свойства для macOS и Windows
set_target_properties(CamFilter PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Установите параметры для macOS (идентификатор пакета и версия)
if(APPLE)
    set_target_properties(CamFilter PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CamFilter
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# Установите параметры для установки
include(GNUInstallDirs)
install(TARGETS CamFilter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Завершите настройку для Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CamFilter)
endif()
