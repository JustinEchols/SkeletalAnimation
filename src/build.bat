@echo off

REM NOTE(Justin): Remove the comment below to enable the developer environment during the build. This makes the compile time alot slower,
REM so if you want to enable it once call shell.bat from the command line.

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
set GameFile=game.cpp
set CommonCompilerFlags= -MTd -nologo -fp:except- -fp:fast -Gm- -GR- -EHa -Zo -Oi -Z7 -WX -W4 -wd4201 -wd4100 -wd4505 -wd4189 -D_CRT_SECURE_NO_WARNINGS=1 -DDEVELOPER=1
set CommonLinkerFlags=-incremental:no user32.lib gdi32.lib opengl32.lib kernel32.lib winmm.lib xinput.lib
set IncludeDirectories= /I "../dependencies/freetype/include"
set LibDirectories= /LIBPATH:"../dependencies/freetype"

if not exist ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL

REM DEBUG 
REM echo WAITING FOR PDB > lock.tmp
REM cl -Od %CommonCompilerFlags% ..\src\%GameFile% -Fmgame.map %IncludeDirectories% /LD /link %LibDirectories% freetype.lib -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:GameUpdateAndRender
REM del lock.tmp
REM cl %CommonCompilerFlags% ..\src\%MainFile% -Fmwin32_main.map  /link %CommonLinkerFlags%
REM popd

REM RELEASE 
echo WAITING FOR PDB > lock.tmp
cl -O2 %CommonCompilerFlags% -DRELEASE=1 ..\src\%GameFile% -Fmgame.map /LD /link -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:GameUpdateAndRender 
del lock.tmp
cl %CommonCompilerFlags% ..\src\%MainFile% -Fmwin32_main.map  /link %CommonLinkerFlags%
popd
