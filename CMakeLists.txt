cmake_minimum_required(VERSION 3.16)
project(QuizzGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

qt6_standard_project_setup()

set(SOURCES
    main.cpp
    mainwindow.cpp
    game.cpp
    networkmanager.cpp
    question.h
)

set(HEADERS
    mainwindow.h
    game.h
    networkmanager.h
)

qt6_add_executable(QuizzGame ${SOURCES} ${HEADERS})

qt6_add_resources(QuizzGame "resources"
    PREFIX "/"
    FILES
        mainwindow.ui
)

target_link_libraries(QuizzGame Qt6::Core Qt6::Widgets Qt6::Network)