cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME Image_Processing)
project(${PROJECT_NAME})

message(STATUS "CMake version: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(OpenCV_DIR "./lib/opencv/")
set(Qt5_DIR "C://Qt/5.15.2/msvc2019_64/lib/cmake/Qt5/")
#include_directories($(CMAKE_CURRENT_SOURCE_DIR)/include)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME} base_proc.cpp MainWindow.cpp main.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets)