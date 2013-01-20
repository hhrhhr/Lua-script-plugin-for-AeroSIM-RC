@echo off

set _MINGW=d:\devel\MinGW440\bin
set GCC=g++
set SIM=f:\sim\AeroSIM-RC\Plugin\LuaScripts

set path=%windir%\system32
set path=%path%;%_MINGW%

if .%1 == .clean goto clean

:all

echo make Plugin.cpp...
g++ -Wall -c -o plugin.o Plugin.cpp -I../luajit-2.0/src
if ERRORLEVEL 1 goto error

echo make plugin_AeroSIMRC.dll...
g++ -Wall -shared -static -s -o plugin_AeroSIMRC.dll plugin.o -L../luajit-2.0/src -lluajit
if ERRORLEVEL 1 goto error

echo copy files...
copy /y plugin_AeroSIMRC.dll %SIM%\
if not exist %SIM%\lua\ mkdir %SIM%\lua
del /q /f %SIM%\lua\*.*
copy /y lua\*.lua %SIM%\lua\
copy /y plugin.txt %SIM%\

goto eof

:clean
del /q /f *.o *.a *.exe *.dll

goto eof

:error
echo.
echo "ERROR!"

:eof
pause
