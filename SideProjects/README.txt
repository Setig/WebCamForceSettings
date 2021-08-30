Directory "WebCamForceSettings/SideProjects" used for side projects in main project WebCamForceSettings.
Any side project is not required to download and build: the application WebCamForceSettings can work without any of the side projects.
The side project must be placed in the directory "WebCamForceSettings/SideProjects".
Any placed side project is considered usable and will be used when building the main project.
In order not to use a placed side project, when building the main project, see the section "Defines for side projects".


Support side projects:
  1. QtSingleApplication (sources: https://github.com/qtproject/qt-solutions/tree/master/qtsingleapplication)
  2. QEasySettings (sources: https://github.com/mguludag/QEasySettings)


Side projects placement directories:
  1. QtSingleApplication: "WebCamForceSettings/SideProjects/qtsingleapplication"
  2. QEasySettings: "WebCamForceSettings/SideProjects/QEasySettings"


Defines for side projects:
  1. QtSingleApplication:
    1.1. BUILD_WITHOUT_QT_SINGLE_APPLICATION
	1.2. BUILD_WITHOUT_Q_EASY_SETTINGS
