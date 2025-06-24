cmake_minimum_required(VERSION 3.5)

project(quiz_game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)
qt_standard_project_setup()

set(SOURCES
    main.cpp
    mainwindow.cpp
    game.cpp
    networkmanager.cpp
)

set(HEADERS
    mainwindow.h
    game.h
    networkmanager.h
    question.h
)

# On enlève la variable UIS et ne la passe plus ici
qt_add_executable(quiz_game
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(quiz_game PRIVATE Qt6::Widgets Qt6::Network)
