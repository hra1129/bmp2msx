// -------------------------------------------------------------
//	BMP to MSX グローバル変数
//												(C)2001	HRA!
// -------------------------------------------------------------

#ifndef	_GLOBALS_H_
#define	_GLOBALS_H_

#include <windows.h>

// -------------------------------------------------------------
//	インスタンス生成判定

#ifdef	_INSTANCE_
#define	EXTERN
#else
#define	EXTERN	extern
#endif

// -------------------------------------------------------------
//	変数

EXTERN	char		g_sPlug[ 260 ];								//	プラグインフォルダ
EXTERN	char		*cmd;										//	コマンドライン
EXTERN	HWND		hOrgWnd;									//	オリジナル画像ウィンドウ
EXTERN	HINSTANCE	hIns;										// インスタンスハンドル:main.cpp
EXTERN	HMENU		hMenu;										// メニューハンドル:main.cpp

// -------------------------------------------------------------
//	後始末

#ifdef	_INSTANCE_
#undef	_INSTANCE_
#endif

#endif	//	ndef _GLOBALS_H_
