!versionAtLeast(QT_VERSION, 5.15.0) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt 5.15.0 or newer")
}

MOC_DIR     = ./moc
OBJECTS_DIR = ./objects
RCC_DIR     = ./rcc
UI_DIR      = ./ui

DESTDIR = $$clean_path($$OUT_PWD/..)

FS_LIBRARY_DIR=$$DESTDIR

equals(TEMPLATE, app)|equals(TEMPLATE, lib) {
    isEmpty(VERSION) VERSION = 0.9.9 # Global project version
    DEFINES += "FS_PROJECT_GLOBAL_VERSION=\\\"$${VERSION}\\\""
}


FS_CONSOLE_NAME = wcfs
DEFINES += "FS_CONSOLE_NAME=\\\"$${FS_CONSOLE_NAME}\\\""

FS_GUI_NAME = WebCamForceSettings
DEFINES += "FS_GUI_NAME=\\\"$${FS_GUI_NAME}\\\""

FS_LIB_NAME = libWebCamForceSettings
DEFINES += "FS_LIB_NAME=\\\"$${FS_LIB_NAME}\\\""


CONFIG -= debug_and_release debug_and_release_target
CONFIG += skip_target_version_ext

CONFIG += c++17

INCLUDEPATH += ../include
