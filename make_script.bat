@echo off

:: setup paths
set _MINGW=d:\devel\MinGW440\bin

set path=%windir%\system32
set path=%path%;%_MINGW%

g++ -Wall -static -o script.exe script.cpp -I../luajit-2.0/src -L../luajit-2.0/src -lluajit

pause
