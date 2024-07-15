@echo off

 setlocal enabledelayedexpansion
 where /Q cl.exe || (
   set __VSCMD_ARG_NO_LOGO=1
   for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set "VS=%%i"
   if "!VS!" equ "" (
     echo ERROR: Visual Studio installation not found
     exit /b 1
   )  
   call "!VS!\VC\Auxiliary\Build\vcvarsall.bat" amd64 || exit /b 1
 )
 
 if "%VSCMD_ARG_TGT_ARCH%" neq "x64" (
   echo ERROR: please run this from MSVC x64 native tools command prompt, 32-bit target is not supported!
   exit /b 1
 )

set MainFile=win32_main.cpp
set CommonCompilerFlags=-O2 -MTd -fp:fast -Oi -Zi
set CommonLinkerFlags=-incremental:no user32.lib gdi32.lib opengl32.lib

if not exist ..\build mkdir ..\build
pushd ..\build

cl "..\\src\\%MainFile%" %CommonCompilerFlags% /link %CommonLinkerFlags%
popd
