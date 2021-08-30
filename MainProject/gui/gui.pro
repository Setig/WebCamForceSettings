TEMPLATE = app

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../commons/build_rules.pri)
include(../commons/use_lib.pri)
include(../commons/side_projects.pri)

TARGET = $$FS_GUI_NAME

LIBS += \
    -ladvapi32

SOURCES += \
        fsabstractcamerasettingsmodel.cpp \
        fsautostart.cpp \
        fscameradefaultsettingsmodel.cpp \
        fscamerapresetsmodel.cpp \
        fscamerausersettingsdialog.cpp \
        fscamerausersettingsmodel.cpp \
        fschangeusercamerasettingdialog.cpp \
        fsitemdelegate.cpp \
        fscamerasettingsdialog.cpp \
        fsrenamepresetnamedialog.cpp \
        fssettings.cpp \
        fscamerasstorage.cpp \
        fsiconcreator.cpp \
        fslockpropertiesmanager.cpp \
        fssettingsdialog.cpp \
        fssystemtrayicon.cpp \
        fstranslationshelper.cpp \
        main.cpp

HEADERS += \
    fsabstractcamerasettingsmodel.h \
    fsautostart.h \
    fscameradefaultsettingsmodel.h \
    fscamerapresetsmodel.h \
    fscamerausersettingsdialog.h \
    fscamerausersettingsmodel.h \
    fschangeusercamerasettingdialog.h \
    fsitemdelegate.h \
    fscamerasettingsdialog.h \
    fsiconcreator.h \
    fsrenamepresetnamedialog.h \
    fssettings.h \
    fscamerasstorage.h \
    fslockpropertiesmanager.h \
    fssettingsdialog.h \
    fssystemtrayicon.h \
    ../include/WebCamFS/AbstractCameraSettingsModel \
    ../include/WebCamFS/CameraDefaultSettingsModel \
    ../include/WebCamFS/CameraPresetsModel \
    ../include/WebCamFS/CameraSettingsDialog \
    ../include/WebCamFS/CameraUserSettingsDialog \
    ../include/WebCamFS/CameraUserSettingsModel \
    ../include/WebCamFS/IconCreator \
    ../include/WebCamFS/ItemDelegate \
    ../include/WebCamFS/CamerasStorage \
    ../include/WebCamFS/LockPropertiesManager \
    ../include/WebCamFS/Settings \
    ../include/WebCamFS/SettingsDialog \
    ../include/WebCamFS/SystemTrayIcon \
    ../include/WebCamFS/AutoStart \
    ../include/WebCamFS/TranslationsHelper \
    ../include/WebCamFS/RenamePresetNameDialog \
    ../include/WebCamFS/ChangeUserCameraSettingDialog \
    fstranslationshelper.h

FORMS += \
    fscamerasettingsdialog.ui \
    fscamerausersettingsdialog.ui \
    fschangeusercamerasettingdialog.ui \
    fsrenamepresetnamedialog.ui \
    fssettingsdialog.ui

OTHER_FILES += \
    WebCamForceSettings.png \
    WebCamForceSettings.ico \
    WebCamForceSettings.rc

TRANSLATIONS += \
    translations/wcfs_ru.ts \
    translations/wcfs_en.ts

RC_ICONS = WebCamForceSettings.ico
RC_FILE  = WebCamForceSettings.rc
