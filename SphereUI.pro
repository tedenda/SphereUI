#-------------------------------------------------
#
# Project created by QtCreator 2014-04-21T19:27:03
#
#-------------------------------------------------

QT       += core gui serialport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SphereUI
TEMPLATE = app


SOURCES += main.cpp\
        sphereui.cpp \
    txthread.cpp \
    serial.cpp

HEADERS  += sphereui.h \
    txthread.h \
    serial.h

FORMS    += sphereui.ui

#RC_ICONS = icon/icon.png
