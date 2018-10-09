@echo off
if not exist FreeImage.dll curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage.dll" --output FreeImage.dll
if not exist FreeImage.h curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage.h" --output FreeImage.h
if not exist FreeImage.lib curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImage.lib" --output FreeImage.lib
if not exist FreeImaged.dll curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImaged.dll" --output FreeImaged.dll
if not exist FreeImaged.lib curl "https://github.com/artint-liu/Marimo_3rd/blob/master/GrapX/third_party/FreeImage/FreeImaged.lib" --output FreeImaged.lib
pause
echo on