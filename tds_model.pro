#-------------------------------------------------
#
# Project created by QtCreator 2014-11-12T08:42:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += printsupport

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -g

TARGET = distrib_test
TEMPLATE = app

LIBS+= -lOpenCL

SOURCES += main.cpp\
        tds_model.cpp \
    qcustomplot.cpp \
    computation.cpp

HEADERS  += tds_model.h \
    qcustomplot.h \
    computation.h

FORMS    += tds_model.ui
