@echo off

REM setlocal enabledelayedexpansion
REM where /Q cl.exe || (
REM   set __VSCMD_ARG_NO_LOGO=1
REM   for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set "VS=%%i"
REM   if "!VS!" equ "" (
REM     echo ERROR: Visual Studio installation not found
REM     exit /b 1
REM   )  
REM   call "!VS!\VC\Auxiliary\Build\vcvarsall.bat" amd64 || exit /b 1
REM )

REM if "%VSCMD_ARG_TGT_ARCH%" neq "x64" (
REM   echo ERROR: please run this from MSVC x64 native tools command prompt, 32-bit target is not supported!
REM   exit /b 1
REM )

set MainFile=win32_main.cpp
set CommonCompilerFlags=-Od -fp:fast -W4 -wd4201 -wd4100 -wd4505 -wd4189 -Oi -Z7
set CommonLinkerFlags=-incremental:no user32.lib gdi32.lib opengl32.lib kernel32.lib


if not exist ..\build mkdir ..\build
pushd ..\build

cl ..\src\%MainFile% %CommonCompilerFlags% /link %CommonLinkerFlags% 
popd
