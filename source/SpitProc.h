// -------------------------------------------------------------
// BMP to MSX
// �X�|�C�g�_�C�A���O
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _SPITPROC_H_
#define _SPITPROC_H_

#include <windows.h>
#include <windowsx.h>

// �^
typedef struct {
	COLORREF	colorcode;
	int			red;
	int			green;
	int			blue;
	int			x;
	int			y;
} SPOITDATA;

// �\���֐�
void spShow( HWND hWnd,SPOITDATA *pdata );

#endif	// _SPITPROC_H_
