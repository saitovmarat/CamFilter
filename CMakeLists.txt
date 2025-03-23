cmake_minimum_required(VERSION 3.16)

project(CamFilter VERSION 0.1 LANGUAGES CXX)

add_definitions(-DUSE_QT_CAMERA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM "win")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(FATAL_ERROR "macOS is not supported for this project.")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
    set(ARCHITECTURE "x64")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TYPE_FOLDER "debug")
    set(BUILD_TYPE_POSTFIX "d")
else()
    set(BUILD_TYPE_FOLDER "release")
    set(BUILD_TYPE_POSTFIX "")
endif()

set(QT_PATH "${CMAKE_SOURCE_DIR}/lib/${PLATFORM}/${ARCHITECTURE}/qt/${BUILD_TYPE_FOLDER}")
set(OPENCV_PATH "${CMAKE_SOURCE_DIR}/lib/${PLATFORM}/${ARCHITECTURE}/opencv/${BUILD_TYPE_FOLDER}")

set(OpenCV_LIBS
    opencv_core4120${BUILD_TYPE_POSTFIX}
    opencv_imgproc4120${BUILD_TYPE_POSTFIX}
    opencv_highgui4120${BUILD_TYPE_POSTFIX}
    opencv_videoio4120${BUILD_TYPE_POSTFIX}
    opencv_imgcodecs4120${BUILD_TYPE_POSTFIX}
)

if(NOT EXISTS ${QT_PATH})
    message(FATAL_ERROR "Required library path does not exist: ${QT_PATH}")
endif()

if(NOT EXISTS ${OPENCV_PATH})
    message(FATAL_ERROR "OpenCV directory does not exist: ${OPENCV_PATH}")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets Multimedia MultimediaWidgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets Multimedia MultimediaWidgets Core)
find_package(OpenCV REQUIRED)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/ui/mainWindow.cpp
    src/ui/mainWindow.h
    src/ui/mainWindow.ui

    src/core/iCameraType.h
    src/core/qtCamera.h
    src/core/qtCamera.cpp
    src/core/openCVCamera.h
    src/core/openCVCamera.cpp

    main.cpp
)

include_directories(src/ui)
include_directories(src/core)

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
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    ${OpenCV_LIBS}
)

set_target_properties(CamFilter PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CamFilter)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/Deploy")
    add_custom_command(TARGET CamFilter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CamFilter> ${DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT_PATH} ${DEPLOY_DIR}/Qt
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${OPENCV_PATH} ${DEPLOY_DIR}/OpenCV
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${DEPLOY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/settings.ini ${DEPLOY_DIR}
        COMMENT "Deploying application to ${DEPLOY_DIR}"
    )
endif()
