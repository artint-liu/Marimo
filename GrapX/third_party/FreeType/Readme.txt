FreeType 2.4.11 Ŀǰ��֧��DLL������ɣ���GrapX��ʹ�þ�̬�⣬���е�CRT realloc����
��clstd��zlib ��alloc��ͻ�������ڰ�ȫ���ǣ�zlibĿǰ��ʹ�þ�̬���ӷ�ʽ��Ȩ�����׾�
����FT2411����Ϊ��̬���ӿ⡣

vs�����ļ��������¸Ķ���
1. �������͸�Ϊ����̬��(.DLL)��
2. ftconfig.h�ļ��ж�"FT_EXPORT"��"FT_EXPORT_DEF"�����������¸Ķ���
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
3. 64λ���win32�������ø���,�������Ŀ¼�޸�Ϊ".\..\..\..\objs\win64\vc2010\"

* ִ��������ʱ����ͬʱѡ��"win32"��"win64"��Ŀ,��Ϊ����ƽ̨�м��ļ���Ŀ¼����ͬ��,�ụ��Ӱ��,
  Ҫ���������������������(win32��win64)ƽ̨�����п�,ע�����л�ƽ̨ʱ���ִ����Ŀ�������.
  �����޸�win64ƽ̨���м��ļ�Ŀ¼,ע��ƽ̨���������ö�Ҫ�޸Ĳ��Ҳ�����ͬ.