// -------------------------------------------------------------
// BMP to MSX
// スポイトダイアログ
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _SPITPROC_H_
#define _SPITPROC_H_

#include <windows.h>
#include <windowsx.h>

// 型
typedef struct {
	COLORREF	colorcode;
	int			red;
	int			green;
	int			blue;
	int			x;
	int			y;
} SPOITDATA;

// 表示関数
void spShow( HWND hWnd,SPOITDATA *pdata );

#endif	// _SPITPROC_H_
