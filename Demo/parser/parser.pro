TEMPLATE = app

QT += core
QT -= gui

CONFIG += c++17 warning_clean exceptions console
CONFIG -= app_bundle
DEFINES += QT_DEPRECATED_WARNINGS QT_ASCII_CAST_WARNINGS QT_USE_QSTRINGBUILDER

TARGET = parser-demo

include(../../qcliparser.pri)

SOURCES += main.cpp

!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
