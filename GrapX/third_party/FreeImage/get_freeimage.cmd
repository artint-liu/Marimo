@echo off
REM if not exist FreeImage.dll curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage.dll" --output FreeImage.dll
REM if not exist FreeImage.h curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage.h" --output FreeImage.h
REM if not exist FreeImage.lib curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage.lib" --output FreeImage.lib
REM if not exist FreeImaged.dll curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImaged.dll" --output FreeImaged.dll
REM if not exist FreeImaged.lib curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImaged.lib" --output FreeImaged.lib

REM if not exist FreeImage3180Win32Win64.zip curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage3180Win32Win64.zip" --output FreeImage3180Win32Win64.zip

7z x FreeImage3180Win32Win64.zip
xcopy FreeImage\Dist\x32\* x32\*
xcopy FreeImage\Dist\x64\* x64\*
copy x32\*.dll ..\..\..\bin\x86
copy x64\*.dll ..\..\..\bin\x64

pause
echo on