# TegraRcmGUI
C++ GUI for [TegraRcmSmash](https://github.com/rajkosto/TegraRcmSmash) by [rajkosto](https://github.com/rajkosto) (payload loader for Nintendo Switch)

## Features
- Inject payloads (such as CFW bootloader, Nand/Key Dumper, etc)
- Manage favorites
- Run Linux on your switch (ShofEL2)
- Mount device as USB mass storage (read/write from/to SD card only, hold power button down for 5sec to exit)
- Option - "Auto inject" : automatically inject payload after selection or/and when the Switch is plugged in RCM mode 
- Option - Minimize app to tray & tray icon's context menu 
- Option - Run app at Windows startup 
- Install APX device driver (if needed)

![Png](http://tegrarcmgui.gq/TegraRcmGUI_v2.2.png)

## Download
[Latest release](https://github.com/eliboa/TegraRcmGUI/releases/latest) (Windows)

## Important notice
This UI is **Windows-only**. 
For other platforms, you can use :
- [Fusée Launcher](https://github.com/Cease-and-DeSwitch/fusee-launcher) (GNU/Linux)
- [NXBoot](https://mologie.github.io/nxboot/) (OS X, iOS)
- [NXLoader](https://github.com/DavidBuchanan314/NXLoader) (Android)
- [Web Fusée Launcher](https://fusee-gelee.firebaseapp.com/) (Cross-platform, only works with Chrome)

## Issue / Suggestion
Please open new [issue](https://github.com/eliboa/TegraRcmGUI/issues) to report a bug or submit a suggestion.

## Credits
- [Kate Temkin](https://github.com/ktemkin) / [Fusée Launcher](https://github.com/Cease-and-DeSwitch/fusee-launcher)
- [Rajkosto](https://github.com/rajkosto) / [TegraRcmSmash](https://github.com/rajkosto/TegraRcmSmash) (Fusée Launcher reimplementation for Windows), [memloader](https://github.com/rajkosto/memloader), SD tool
- [fail0verflow](https://github.com/fail0verflow) / [ShofEL2](https://github.com/fail0verflow/shofel2) (Boot stack for no-modification, universal code execution and Linux on the Nintendo Switch)
- [Rob Fisher](http://come.to/robfisher)  / GetRelativeFilename
