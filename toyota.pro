#-------------------------------------------------
#
# Project created by QtCreator 2019-03-21T17:50:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = toyota
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    videoplayer.cpp \
    interfacewindow.cpp \
    serialwatcher.cpp

HEADERS += \
    videoplayer.h \
    interfacewindow.h \
    serialwatcher.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += /usr/local/include/opencv4
INCLUDEPATH += /home/$$(USER)/astra/include
INCLUDEPATH += /home/$$(USER)/astra/samples/common


LIBS +=  -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_video -lopencv_videoio

LIBS += -L"/home/$$(USER)/astra/bin/lib" -lastra -lastra_core -lastra_core_api
