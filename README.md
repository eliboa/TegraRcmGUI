# TegraRcmGUI
A C++ GUI for [TegraRcmSmash](https://github.com/rajkosto/TegraRcmSmash) by [rajkosto](https://github.com/rajkosto). (payload loader for Nintendo Switch)

## Features
- Inject payloads (such as CFW bootloader, Nand/Key Dumper, etc)
- Manage favorites
- Run Linux on your switch (ShofEL2)
- Mount device as USB mass storage (hold power button down for 10sec to exit)
- Option - "Auto inject" : automatically inject payload after selection or/and when the Switch is plugged in RCM mode (does not apply at startup)
- Option - Minimize app to tray & tray icon's context menu 
- Option - Run app at Windows startup 
- Install APX device driver (if needed)

![Png](http://splatoon.eu/tuto_switch/TegraRcmGUI_v2.1.png)

## Download
[Latest release](https://github.com/eliboa/TegraRcmGUI/releases) (Windows)

## Important notice
This UI is **Windows-only**. For non Windows systems, you can use :
- GNU/Linux : [Fusée Launcher](https://github.com/Cease-and-DeSwitch/fusee-launcher)
- OS X & iOS : [NXBoot](https://mologie.github.io/nxboot/)
- Android : [NXLoader](https://github.com/DavidBuchanan314/NXLoader/releases)

## Issues / Suggestions
Please open new [issue](https://github.com/eliboa/TegraRcmGUI/issues) to submit an issue or suggestion.

## Credits
- [Kate Temkin](https://github.com/ktemkin) / [Fusée Launcher](https://github.com/Cease-and-DeSwitch/fusee-launcher)
- [Rajkosto](https://github.com/rajkosto) / [TegraRcmSmash](https://github.com/rajkosto/TegraRcmSmash) (Fusée Launcher reimplementation for Windows), [memloader](https://github.com/rajkosto/memloader), SD tool
- [fail0verflow](https://github.com/fail0verflow) / [ShofEL2](https://github.com/fail0verflow/shofel2) (Boot stack for no-modification, universal code execution and Linux on the Nintendo Switch)
- [Rob Fisher](http://come.to/robfisher)  / GetRelativeFilename
