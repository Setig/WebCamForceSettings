# WebCamForceSettings
Is designed to work with video proc amp and camera control settings (properties) of the webcam.

## Build dependencies
* Qt 5.15.0 or newer (need C++17)
* Only on Windows 7, Windows 10

# How to use?
WebCamForceSettings - system tray program<br/>
wcfs&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- console program

## GUI argument options
>Usage: WebCamForceSettings [OPTION]<br/>
The application is designed to work with webcam video proc amp and camera control settings (properties).<br/><br/>
OPTIONS:<br/>
-s, --show-camera-settings-dialog="devicePath"<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;show dialog web camera settings by device path<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(devicePath for you webcam can be found using cameras info)<br/><br/>
-l	create log file in directory:<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"C:\Users\%USERNAME%\AppData\Local\Temp\WebCamFS\logs"<br/>

## Console argument options
>Usage: wcfs OPTION<br/>
The console application is designed to work with webcam info (camera name, device path, video proc amp and camera control settings (properties)).<br/><br/>
OPTIONS:<br/>
-c, --cameras-info&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;print info for all available web cameras(video input devices) in the OS<br/>
-h, --help&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;display this help and exit<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;--gui_help&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;display help for gui application and exit<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;--version&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;output version information and exit<br/>

# How to build it?
`qmake WebCamForceSettings.pro`<br/>
`make`

For 'WebCamForceSettings' need transation files, it looks for them in the directory 'translations'. Qt translations are also needed, which are searched for in the 'translations / qt' directory.<br/>
Approximate file types location scheme (without Qt libraries):
> libWebCamForceSettings.dll<br/>
> WebCamForceSettings.exe<br/>
> wcfs.exe<br/>
> translations/wcfs_\*.qm<br/>
> translations/qt/qt_\*.qm<br/>
> translations/qt/qtbase_\*.qm<br/>
