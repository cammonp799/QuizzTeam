QT += core widgets network

CONFIG += c++17
CONFIG += sdk_no_version_check

TARGET = QuizzGame
TEMPLATE = app

# Force le bon compilateur et SDK
QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
QMAKE_MAC_SDK_VERSION = 15.2

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    game.cpp \
    networkmanager.cpp \
    question.cpp

HEADERS += \
    mainwindow.h \
    game.h \
    networkmanager.h \
    question.h

FORMS += \
    mainwindow.ui