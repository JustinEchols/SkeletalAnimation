# SkeletalAnimation
Skeletal animation example.

## Dependencies
stb_image.h for loading textures

freetype for font rendering

OpenGL for graphics

C runtime library

## Build
There are two builds. Release and debug. The build.bat file compiles the release build which does not include freetype and font rendering. If
you want to compile the debug build you will need to add freetype appropriately. The directory structure after adding freetype would look like:

dependencies/freetype/include

dependencies/freetype/freetype.lib

To compile in release mode, call the build.bat file.

SkeletalAnimation\src>build.bat

## Run
SkeletalAnimation\src>..\build\win32_main.exe
