cmake_minimum_required(VERSION 3.16)
project(UAVTelemViewer LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(SOURCES
    main.cpp
    mainwindow.cpp        # <─ MUTLAKA var
)

set(HEADERS
    mainwindow.h
)

set(UIS
    mainwindow.ui
)

add_executable(${PROJECT_NAME}
    ${SOURCES} ${HEADERS} ${UIS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Widgets
    Qt6::Network
)
