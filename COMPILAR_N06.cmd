@echo off
setlocal
title Compilar Mega Man X4 nativo para DSi
if not defined X4_WSL_DISTRO set "X4_WSL_DISTRO=Arch"
wsl -d "%X4_WSL_DISTRO%" --cd "%~dp0." --exec bash --noprofile --norc -c "python3 native/compile.py --n084"
set "X4_BUILD_RESULT=%ERRORLEVEL%"
if not "%X4_BUILD_RESULT%"=="0" echo Falha ao compilar. Confira o log indicado acima.
if /I not "%~1"=="--no-pause" pause
exit /b %X4_BUILD_RESULT%
