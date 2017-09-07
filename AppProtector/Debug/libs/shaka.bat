@echo off
if "%PATH_BASE%" == "" set PATH_BASE=%PATH%
set PATH=%CD%;%PATH_BASE%;
java -jar "%~dp0\shakaapktool\ShakaApktool_3.0.0.jar" %1 %2 %3 %4 %5 %6 %7 %8 %9