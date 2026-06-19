cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME Image_Processing)
project(${PROJECT_NAME} LANGUAGES CXX)

message(STATUS "CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(IMAGE_PROCESSING_BUILD_CLI "Build the command line image processor" ON)
option(IMAGE_PROCESSING_BUILD_QT_APP "Build the Qt visual demo application" ON)

if(WIN32 AND NOT DEFINED OpenCV_DIR AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/opencv/OpenCVConfig.cmake")
    set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/opencv")
endif()
find_package(OpenCV REQUIRED)

add_library(image_processing_ops image_ops.cpp image_ops.h)
target_include_directories(image_processing_ops PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS})
target_link_libraries(image_processing_ops PUBLIC ${OpenCV_LIBS})

if(IMAGE_PROCESSING_BUILD_CLI)
    add_executable(image_cli image_cli.cpp)
    target_link_libraries(image_cli PRIVATE image_processing_ops)
endif()

if(IMAGE_PROCESSING_BUILD_QT_APP)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    if(WIN32 AND NOT DEFINED Qt5_DIR AND EXISTS "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")
        set(Qt5_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")
    endif()

    find_package(Qt5 COMPONENTS
            Core
            Gui
            Widgets
            REQUIRED)

    add_executable(${PROJECT_NAME}
            base_proc.cpp
            MainWindow.cpp
            MainWindow.h
            mainwindow.ui
            main.cpp)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${OpenCV_LIBS}
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets)
endif()
