#-------------------------------------------------
#
# Project created by QtCreator 2013-03-13T09:38:32
#
#-------------------------------------------------

QT       += core

TARGET = AithonProgrammer
CONFIG   += console

TEMPLATE = app


SOURCES += main.cpp listener.cpp

include(../qextserialport-1.2rc/src/qextserialport.pri)

HEADERS += listener.h
