# pri file for side projects. (see file "WebCamForceSettings/SideProjects/README.txt")


SIDE_PROJECT_DIRECTORY_PATH=$$clean_path($$PWD/../../SideProjects)


# QtSingleApplication
QT_SINGLE_APPLICATION_PROJECT_PATH=$$SIDE_PROJECT_DIRECTORY_PATH/qtsingleapplication

exists($$QT_SINGLE_APPLICATION_PROJECT_PATH) {
    !contains(DEFINES, BUILD_WITHOUT_QT_SINGLE_APPLICATION) {
        DEFINES+=BUILD_WITH_QT_SINGLE_APPLICATION

        contains(TEMPLATE, app) {
            include($$QT_SINGLE_APPLICATION_PROJECT_PATH/common.pri)
            INCLUDEPATH += $$QT_SINGLE_APPLICATION_PROJECT_PATH/src
            LIBS += -L$$QTSINGLEAPPLICATION_LIBDIR -l$$QTSINGLEAPPLICATION_LIBNAME
        }
    } else {
        message("Build without QtSingleApplication project.");
    }
} else {
    message("Directory for QtSingleApplication project not exist!")
}


# QEasySettings
Q_EASY_SETTINGS_PROJECT_PATH=$$SIDE_PROJECT_DIRECTORY_PATH/QEasySettings

exists($$Q_EASY_SETTINGS_PROJECT_PATH) {
    !contains(DEFINES, BUILD_WITHOUT_Q_EASY_SETTINGS) {
        DEFINES+=BUILD_WITH_Q_EASY_SETTINGS

        contains(TEMPLATE, app) {
            INCLUDEPATH += $$Q_EASY_SETTINGS_PROJECT_PATH
            SOURCES += $$Q_EASY_SETTINGS_PROJECT_PATH/qeasysettings.cpp
            HEADERS += $$Q_EASY_SETTINGS_PROJECT_PATH/qeasysettings.hpp
        }
    } else {
         message("Build without QEasySettings project.");
    }
} else {
    message("Directory for QEasySettings project not exist!")
}
