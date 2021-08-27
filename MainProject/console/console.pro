TEMPLATE = app

QT -= gui

CONFIG += console
CONFIG -= app_bundle

include(../commons/build_rules.pri)
include(../commons/use_lib.pri)

TARGET = $$FS_CONSOLE_NAME

SOURCES += main.cpp
