FreeType 2.4.11 目前不支持DLL库的生成，在GrapX中使用静态库，其中的CRT realloc函数
与clstd中zlib 的alloc冲突，而出于安全考虑，zlib目前想使用静态链接方式。权衡利弊决
定把FT2411改造为动态链接库。

vs工程文件做出如下改动：
1. 配置类型改为“动态库(.DLL)”
2. ftconfig.h文件中对"FT_EXPORT"和"FT_EXPORT_DEF"定义做出如下改动：
-------------------------------------------------------------------------------
#ifndef FT_EXPORT

#ifdef __cplusplus
#define FT_EXPORT( x )  extern "C" __declspec(dllexport) x
#else
#define FT_EXPORT( x )  extern __declspec(dllexport) x
#endif

#else

#ifdef __cplusplus
#define FT_EXPORT( x )  extern "C" __declspec(dllimport) x
#else
#define FT_EXPORT( x )  extern __declspec(dllimport) x
#endif

#endif /* !FT_EXPORT */


#ifndef FT_EXPORT_DEF

#ifdef __cplusplus
#define FT_EXPORT_DEF( x )  extern "C" __declspec(dllexport)  x
#else
#define FT_EXPORT_DEF( x )  extern __declspec(dllexport) x
#endif

#else

#ifdef __cplusplus
#define FT_EXPORT_DEF( x )  extern "C" __declspec(dllimport)  x
#else
#define FT_EXPORT_DEF( x )  extern __declspec(dllimport) x
#endif

#endif /* !FT_EXPORT_DEF */
-------------------------------------------------------------------------------
3. 64位版从win32工程配置复制,并将输出目录修改为".\..\..\..\objs\win64\vc2010\"

* 执行批生成时不能同时选择"win32"和"win64"项目,因为两个平台中间文件的目录是相同的,会互相影响,
  要想解决此问题可以依次生成(win32或win64)平台的所有库,注意在切换平台时最好执行项目清理操作.
  或者修改win64平台的中间文件目录,注意平台下所有配置都要修改并且不能相同.