@echo off

set _MINGW=d:\devel\MinGW440\bin
set path=%windir%\system32
set path=%path%;%_MINGW%

set GCC=g++

set SIM=f:\sim\AeroSIM-RC\Plugin\PluginDemo2

if .%1 == .clean goto clean

:all
rem %GCC% -c -o MAX7456.o MAX7456.cpp
rem %GCC% -c -o OSD.o OSD.cpp
rem %GCC% -c -o Plugin.o Plugin.cpp
rem %GCC% -shared -static -s -o plugin_AeroSIMRC.dll MAX7456.o OSD.o Plugin.o
rem %GCC% -shared -static -s -o plugin_AeroSIMRC.dll Plugin.o
rem ar rcu libplugin_AeroSIMRC.a MAX7456.o OSD.o Plugin.o

echo make Plugin.cpp...
g++ -Wall -c -o plugin.o Plugin.cpp -I../luajit-2.0/src
echo make plugin_AeroSIMRC.dll...
g++ -Wall -shared -static -s -o plugin_AeroSIMRC.dll plugin.o -L../luajit-2.0/src -lluajit

echo copy files...
copy /y plugin_AeroSIMRC.dll %SIM%\
if not exist %SIM%\lua\ mkdir %SIM%\lua
del /q /f %SIM%\lua\*.*
copy /y lua\*.lua %SIM%\lua\

goto eof

:clean
del /q /f *.o *.a *.exe *.dll

goto eof

:eof
