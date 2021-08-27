TEMPLATE = subdirs

include(MainProject/commons/side_projects.pri)

OTHER_FILES += \
    SideProjects/README.txt

contains(DEFINES, BUILD_WITH_QT_SINGLE_APPLICATION) {
    SUBDIRS += $$QT_SINGLE_APPLICATION_PROJECT_PATH/buildlib
}

SUBDIRS += MainProject
