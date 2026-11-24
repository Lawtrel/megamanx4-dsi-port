@echo off
setlocal
title Megaman X4 Ds
powershell.exe -NoLogo -NoProfile -STA -ExecutionPolicy Bypass -File "%~dp0Abrir-No-Windows.ps1"
set "MMX4_RESULT=%ERRORLEVEL%"
if not "%MMX4_RESULT%"=="0" pause
exit /b %MMX4_RESULT%
