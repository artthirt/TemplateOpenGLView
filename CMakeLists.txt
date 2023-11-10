cmake_minimum_required(VERSION 3.15)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

set(PROJECT_NAME test-distortion)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX C)

find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

find_package(OpenCV REQUIRED PATHS d:/devs/opencv/build/x64/vc16/lib)

set(SRC
    main.cpp
    MainWindow.h MainWindow.cpp MainWindow.ui
    ViewOpenGL.h ViewOpenGL.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBRARIES}
    Qt5::Widgets Qt5::OpenGL
)
