QT -= gui

TEMPLATE = lib
DEFINES += FS_LIB_LIBRARY

CONFIG += dll

include(../commons/build_rules.pri)

TARGET = $$FS_LIB_NAME

LIBS += \
    -luuid \
    -lOle32 \
    -lOleAut32 \
    -lStrmiids

SOURCES += \
    fscorecamerasstorage.cpp \
    fscoresettings.cpp \
    libfswebcamforcesettings.cpp \
    fs_structs.cpp \
    fscamera.cpp 

HEADERS += \
    fs_app_arguments.h \
    fs_global.h \
    fscorecamerasstorage.h \
    fscoresettings.h \
    libfswebcamforcesettings.h \
    fs_structs.h \
    fscamera.h \
    ../include/WebCamFS/Lib \
    ../include/WebCamFS/Global \
    ../include/WebCamFS/Camera \
    ../include/WebCamFS/CoreCamerasStorage \
    ../include/WebCamFS/CoreSettings \
    ../include/WebCamFS/Structs \
    ../include/WebCamFS/AppArguments
