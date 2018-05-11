@Echo OFF
if exist shofel2 (
  cd shofel2
)
::
:: CONTROLS
::
set USE_GIT=0
git --help > NUL 2> NUL
if errorlevel 1 (
  if not exist C:\windows\System32\WindowsPowerShell\v1.0\powershell.exe (
    echo Neither Powershell nor Git are installed on this computer.
    echo Please download https://github.com/SoulCipher/shofel2_linux/archive/master.zip manually
    echo Waiting 10s before closing...
    ping 127.0.0.1 -n 11 > nul
    exit
  )
) else (
  set USE_GIT=1
)
if not exist conf\imx_usb.conf set MISSING=1
if not exist conf\switch.conf set MISSING=1
if not exist coreboot\cbfs.bin set MISSING=1
if not exist coreboot\coreboot.rom set MISSING=1
if not exist dtb\tegra210-nintendo-switch.dtb set MISSING=1
if not exist image\switch.scr.img set MISSING=1
if not exist kernel\Image.gz set MISSING=1
if not defined MISSING (
  echo All needed files already at the right place !
  echo You should be able to boot Linux from TegraRcmGUI
  echo If not, verify read access permission. file already open in another program ?
  echo Waiting 10s before closing...
  ping 127.0.0.1 -n 11 > nul
  exit
)
if exist conf\ RMDIR /S /Q conf
if exist coreboot\ RMDIR /S /Q coreboot
if exist dtb\ RMDIR /S /Q dtb
if exist image\ RMDIR /S /Q image
if exist kernel\ RMDIR /S /Q kernel

::
:: PROCEDURE
::
if %USE_GIT% equ 1 (
  echo Downloading linux kernel from SoulCipher repo
  git clone https://github.com/SoulCipher/shofel2_linux
  echo Moving needed files
  move shofel2_linux\conf .\
  move shofel2_linux\coreboot .\
  move shofel2_linux\dtb .\
  move shofel2_linux\image .\
  move shofel2_linux\kernel .\
  echo Removing unnecessary files
  RMDIR /S /Q shofel2_linux
  echo Completed. You should be able to boot Linux from TegraRcmGUI.
) else (
SetLocal EnableDelayedExpansion
  if exist shofel2.zip del shofel2.zip
  echo Downloading linux kernel from SoulCipher repo
  echo https://github.com/SoulCipher/shofel2_linux/archive/master.zip
  powershell -Command "[Net.ServicePointManager]::SecurityProtocol = 'tls12, tls11, tls'; Invoke-WebRequest -Uri https://github.com/SoulCipher/shofel2_linux/archive/master.zip -OutFile shofel2.zip"
  echo Unzipping package
  powershell -Command "Expand-Archive -Path shofel2.zip"
  echo Moving needed files
  move shofel2\shofel2_linux-master\conf .\
  move shofel2\shofel2_linux-master\coreboot .\
  move shofel2\shofel2_linux-master\dtb .\
  move shofel2\shofel2_linux-master\image .\
  move shofel2\shofel2_linux-master\kernel .\
  echo Removing unnecessary files
  del shofel2.zip
  RMDIR /S /Q shofel2
  echo Completed. You should be able to boot Linux from TegraRcmGUI.
)
echo Waiting 5s before closing...
ping 127.0.0.1 -n 6 > nul
exit
