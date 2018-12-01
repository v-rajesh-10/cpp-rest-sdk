@echo off
if exist bin.v2 rd /Q /S bin.v2
if exist libs\config\checks\architecture\bin rd /Q /S libs\config\checks\architecture\bin
if exist stage32 rd /Q /S stage32
if exist stage64 rd /Q /S stage64
if exist tools\build\src\engine\bin.ntx86 rd /Q /S tools\build\src\engine\bin.ntx86
if exist tools\build\src\engine\bootstrap rd /Q /S tools\build\src\engine\bootstrap
if exist b2.exe del b2.exe
if exist bjam.exe del bjam.exe
if exist bootstrap.log del bootstrap.log
if exist build.log del build.log
if exist project-config.jam del project-config.jam
