编译 Marimo 库
==============

下载并安装VLD
http://vld.codeplex.com/downloads/get/824280
增加环境变量"VLDDIR"指向vld刚刚安装的目录

下载并安装DXSDK Jun10
http://www.microsoft.com/en-us/download/details.aspx?id=6812

获得其它第三方库
用git取下https://github.com/artint-liu/Marimo_3rd.git,将Marimo3rdParty.7z解压到Marimo工程目录.


Visual Studio 2017
使用clstd库的工程配置
C/C++ 附加包含目录：$(MARIMO_DIR)clstd\
链接器附加库目录  ：$(MARIMO_DIR)clstd\bin\$(PlatformToolset)\
链接器附加依赖项  ：clstd_win32.$(PlatformTarget).$(Configuration).lib
C/C++ 附加选项    ：/Zc:__cplusplus

使用GrapX库的工程配置
链接器附加库目录  ：$(MARIMO_DIR)bin\$(PlatformTarget)
链接器附加依赖项  ：GrapX_win32.$(PlatformTarget).$(Configuration).lib
C/C++ 附加选项    ：/Zc:__cplusplus

VLD 目录：$(VLDDIR)\lib\Win$(PlatformArchitecture)
