TEMPLATE = subdirs

SUBDIRS += \
    lib \
    console \
    gui

console.depends = lib
gui.depends     = lib

OTHER_FILES += \
    commons/build_rules.pri
    commons/side_projects.pri
    commons/use_lib.pri
