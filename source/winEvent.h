// -------------------------------------------------------------
// ウィンドウ関数支援マクロ
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _WINEVENT_H_
#define _WINEVENT_H_

#include <windows.h>

#define	WINPROC(name)		LRESULT WINAPI name( HWND hWnd,UINT msg,WPARAM wp,LPARAM lp )
#define	DEFPROC()			DefWindowProc( hWnd,msg,wp,lp )
#define EVENT(name)			LRESULT name( HWND hWnd,UINT msg,WPARAM wp,LPARAM lp )
#define ONEVENT(id,name)	case (id):	return (name)(hWnd,msg,wp,lp)
#define CALLEVENT(name)		(name)(hWnd,msg,wp,lp)

typedef LRESULT (*LPEVENT)(HWND,UINT,WPARAM,LPARAM);

#endif
