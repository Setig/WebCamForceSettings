function Component()
{
	installer.installationFinished.connect(this, Component.prototype.installationFinishedPageIsShown);
}

Component.prototype.createOperations = function()
{
	try
	{
		component.createOperations();
	}
	catch (e)
	{
		print(e);
	}
	
	if (installer.value("os") === "win")
	{
		// Desktop
		component.addOperation("CreateShortcut", "@TargetDir@/WebCamForceSettings.exe", "@DesktopDir@/WebCam Force Settings.lnk");
		
		// StartMenu
		component.addOperation("CreateShortcut", "@TargetDir@/WebCamForceSettings.exe", "@StartMenuDir@/WebCam Force Settings.lnk");
		component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe",     "@StartMenuDir@/Modify WebCamFS.lnk");
	}
}

Component.prototype.installationFinishedPageIsShown = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            installer.addWizardPageItem( component, "InstallFinishForm", QInstaller.InstallationFinished );
        }
    } catch(e) {
        console.log(e);
    }
}