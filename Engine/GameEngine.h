#ifndef _MARIMO_GAME_ENGIEN_H_
#define _MARIMO_GAME_ENGIEN_H_
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� GAMEENGINE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// GAMEENGINE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef GAMEENGINE_EXPORTS
#define GAMEENGINE_API __declspec(dllexport)
#define GAMEENGINE_DLL __declspec(dllexport)
#else
#define GAMEENGINE_API __declspec(dllimport)
#define GAMEENGINE_DLL __declspec(dllimport)
#endif

#endif // #ifndef _MARIMO_GAME_ENGIEN_H_