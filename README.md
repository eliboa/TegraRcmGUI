# TegraRcmGUI
C++ GUI for [TegraRcmSmash](https://github.com/rajkosto/TegraRcmSmash) by [rajkosto](https://github.com/rajkosto) (Fusée Gelée exploit for Nintendo Switch)

## Features
- Inject payloads (such as CFW bootloader, Nand/Key Dumper, etc)
- Manage favorites
- Run Linux on your switch (ShofEL2)
- Mount device as USB mass storage (read/write from/to SD card & NAND partitions, hold power button down for 5sec to exit)
- Dump BIS keys for eMMC content decryption (biskeydump by rajkosto)
- Option - "Auto inject" : automatically inject payload after selection or/and when the Switch is plugged in RCM mode 
- Option - Minimize app to tray & tray icon's context menu 
- Option - Run app at Windows startup 
- Install APX device driver (if needed)

![Png](http://tegrarcmgui.gq/TegraRcmGUI_v2.5.png)

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

## How to backup/restore your Nintendo Switch's NAND ?

 1) Use [memloader](https://github.com/rajkosto/memloader) v3 to mount eMMC on your computer
 2) Download and open [NxNandManager](https://github.com/eliboa/NxNandManager). Select "File" then "Open drive".   
 3) Select the mounted drive. You can now perform backup/restore operations.   

![Png](http://splatoon.eu/switch/NxNandManager_v1.1_howto_open_drive.png)   

## Credits
- [Rajkosto](https://github.com/rajkosto) / [TegraRcmSmash](https://github.com/rajkosto/TegraRcmSmash) (Fusée Launcher reimplementation for Windows), [memloader](https://github.com/rajkosto/memloader), SD tool, [biskeydump](https://github.com/rajkosto/biskeydump)
- [Kate Temkin](https://github.com/ktemkin) / [Fusée Launcher](https://github.com/Cease-and-DeSwitch/fusee-launcher)
- [fail0verflow](https://github.com/fail0verflow) / [ShofEL2](https://github.com/fail0verflow/shofel2) (Boot stack for no-modification, universal code execution and Linux on the Nintendo Switch)
- [SciresM](https://github.com/SciresM) / [Atmosphere](https://github.com/Atmosphere-NX/Atmosphere)
- [CTCaer](https://github.com/CTCaer/hekate)  / [Hekate](https://github.com/CTCaer/hekate)
- [Reisyukaku](https://github.com/Reisyukaku/) / [ReiNX](https://github.com/Reisyukaku/ReiNX)
