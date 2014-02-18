#-------------------------------------------------
#
# Project created by QtCreator 2013-03-13T09:38:32
#
#-------------------------------------------------

QT       += core

TARGET = AithonProgrammer
CONFIG   += console


SOURCES += main.cpp ../avrdude/ser_posix.c

include(../qextserialport-1.2rc/src/qextserialport.pri)

HEADERS +=

CONFIG += static
QMAKE_LFLAGS += -stdlib=libstdc++
