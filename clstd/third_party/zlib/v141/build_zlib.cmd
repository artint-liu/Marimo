@echo off
set ZLIB_LIB=zlib-1.2.11

echo 1.需要在 vs 2017 命令行提示中执行
echo 2.需要7z.exe
pause

if not exist %ZLIB_LIB%.tar.gz curl "https://www.zlib.net/%ZLIB_LIB%.tar.gz" --output %ZLIB_LIB%.tar.gz
7z x %ZLIB_LIB%.tar.gz
7z x %ZLIB_LIB%.tar

rem cd %ZLIB_LIB%
REM nmake -f win32/Makefile.msc
REM copy zlib.lib ..
REM copy zconf.h ..
REM copy zlib.h ..

rem cd zlib-1.2.11\contrib\masmx64
rem call bld_ml64.bat
rem cd %~dp0
rem 
rem cd zlib-1.2.11\contrib\masmx86
rem call bld_ml32.bat
rem cd %~dp0
echo ===============================================================
echo 这里需要手动去除zlibstat.vcxproj工程中“ZLIB_WINAPI”预处理定义，没有找到好的方法
echo ===============================================================
pause

cd %ZLIB_LIB%\contrib\vstudio\vc14
msbuild zlibstat.vcxproj /tv:15.0 /p:Configuration=Debug;Platform=Win32
msbuild zlibstat.vcxproj /tv:15.0 /p:Configuration=Debug;Platform=x64
msbuild zlibstat.vcxproj /tv:15.0 /p:Configuration=ReleaseWithoutAsm;Platform=Win32
msbuild zlibstat.vcxproj /tv:15.0 /p:Configuration=ReleaseWithoutAsm;Platform=x64
cd %~dp0

md include
md lib
md lib\x86
md lib\x64

copy %ZLIB_LIB%\zlib.h include
copy %ZLIB_LIB%\zconf.h include

copy %ZLIB_LIB%\contrib\vstudio\vc14\x64\ZlibStatDebug\zlibstat.lib lib\x64\zlibstat_d.lib
copy %ZLIB_LIB%\contrib\vstudio\vc14\x86\ZlibStatDebug\zlibstat.lib lib\x86\zlibstat_d.lib

copy %ZLIB_LIB%\contrib\vstudio\vc14\x64\ZlibStatReleaseWithoutAsm\zlibstat.lib lib\x64\zlibstat.lib
copy %ZLIB_LIB%\contrib\vstudio\vc14\x86\ZlibStatReleaseWithoutAsm\zlibstat.lib lib\x86\zlibstat.lib


pause
echo on

