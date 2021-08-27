QT_SINGLE_APPLICATION_PROJECT_PATH=$$clean_path($$PWD/../../SideProjects/qtsingleapplication)

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
    message("Directory for project QtSingleApplication not exist!")
}
