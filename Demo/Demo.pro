TEMPLATE = app

QT += core
QT -= gui

CONFIG += c++11  warning_clean exceptions console
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS QT_ASCII_CAST_WARNINGS QT_USE_QSTRINGBUILDER

TARGET = demo

include(../qcliparser.pri)

SOURCES += main.cpp
