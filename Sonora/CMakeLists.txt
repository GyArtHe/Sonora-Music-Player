cmake_minimum_required(VERSION 3.16)
project(Sonora VERSION 1.0.0 LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

qt_standard_project_setup()

set(APP_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    resources.qrc
)

if(WIN32)
    list(APPEND APP_SOURCES app.rc)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    qt_add_executable(Sonora ${APP_SOURCES})
else()
    qt_add_executable(Sonora WIN32 ${APP_SOURCES})
endif()

target_link_libraries(Sonora PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
)

if(WIN32)
    target_link_libraries(Sonora PRIVATE ole32)
endif()