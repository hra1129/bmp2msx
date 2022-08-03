// -------------------------------------------------------------
// BMP to MSX
// �I���W�i���摜�\���E�B���h�E
//											(C)2000 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include "Config.h"
#include "winEvent.h"
#include "Resource.h"
#include "SpitProc.h"

// -------------------------------------------------------------
// �v���g�^�C�v�錾

// ��ʃC�x���g
EVENT( onSubClose		);	// ����
EVENT( onSubCreate		);	// ����
EVENT( onSubDestroy		);	// �p��
EVENT( onSubPaint		);	// �`��
EVENT( onSubCommand		);	// �R�}���h����
EVENT( onSubMouseMove	);	// �}�E�X�ړ�
EVENT( onSubLButton		);	// ���N���b�N
EVENT( onSubSize		);	// �T�C�Y�ύX

// ���j���[�C�x���g
EVENT( onSubSingle		);	// ���{�\��
EVENT( onSubDouble		);	// �Q�{�\��

// -------------------------------------------------------------
// �O���[�o���ϐ�

static HBITMAP	hMemBM	= NULL;
static HDC		hMemDC	= NULL;
static int w;						// �摜��
static int h;						// �摜����
static int ww;						// ���{�̎��̃E�B���h�E��
static int wh;						// ���{�̎��̃E�B���h�E����
extern HWND			hOrgWnd;		// main.cpp
extern HINSTANCE	hIns;			// main.cpp

// =====================================================
// ���[�e�B���e�B

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�I���W�i���摜�_�C�A���O�̕\��
//	2.	����
//		bmp	...	(I)	�摜�C���[�W
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void swShow( const char *bmp )
{
	BITMAPINFOHEADER *bih;
	const char *bits;
	RECT r;

	bih = (BITMAPINFOHEADER*)bmp;
	bits = bmp + bih->biSize;
	if( bih->biClrUsed ){
		bits+=bih->biClrUsed * sizeof(RGBQUAD);
	}else if( bih->biBitCount <= 8 ){
		bits+=(1 << bih->biBitCount) * sizeof(RGBQUAD);
	}
	// �T�C�Y�̎擾
	r.top = r.left = 0;
	r.right = w = bih->biWidth;
	r.bottom= h = bih->biHeight;
	AdjustWindowRect( &r,CNWSS,TRUE );
	ww = r.right-r.left;
	wh = r.bottom-r.top;
	SetWindowPos( hOrgWnd,NULL,0,0,ww,wh,SWP_NOMOVE | SWP_NOZORDER );	
	// �r�b�g�}�b�v���쐬����
	HDC	hDC;
	HBITMAP hBM;
	hDC = GetDC( hOrgWnd );
	hBM = CreateCompatibleBitmap( hDC,w,h );
	ReleaseDC( hOrgWnd,hDC );
	if( hBM==NULL ) return;
	SelectBitmap( hMemDC,hBM );
	if( hMemBM ) DeleteBitmap( hMemBM );
	hMemBM = hBM;
	// �r�b�g�}�b�v�ɉ摜��`��
	StretchDIBits( hMemDC,0,0,w,h,0,0,w,h,bits,(BITMAPINFO*)bih,DIB_RGB_COLORS,SRCCOPY );
	ShowWindow( hOrgWnd,SW_SHOW );
}

// =====================================================

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	��������
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( SubProc )
{
	switch( msg ){
	ONEVENT( WM_CLOSE		,onSubClose		);
	ONEVENT( WM_CREATE		,onSubCreate	);
	ONEVENT( WM_DESTROY		,onSubDestroy	);
	ONEVENT( WM_PAINT		,onSubPaint		);
	ONEVENT( WM_COMMAND		,onSubCommand	);
	ONEVENT( WM_MOUSEMOVE	,onSubMouseMove	);
	ONEVENT( WM_LBUTTONDOWN	,onSubLButton	);
	ONEVENT( WM_SIZE		,onSubSize		);	
	}
	return DEFPROC();
}

// =====================================================
// ��ʃC�x���g

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubClose )
{
	ShowWindow( hWnd,SW_HIDE );
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubCreate )
{
	HDC	hDC;
	hDC = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hDC );
	ReleaseDC( hWnd,hDC );
	if( hMemDC == NULL ) return -1;
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubDestroy )
{
	if( hMemDC!=NULL ) DeleteDC( hMemDC );
	if( hMemBM!=NULL ) DeleteBitmap( hMemBM );
	hMemDC = NULL;
	hMemBM = NULL;
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��ʂ̕`��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		r;

	hDC = BeginPaint( hWnd,&ps );
	GetClientRect( hWnd,&r );
	SetStretchBltMode( hDC,COLORONCOLOR );
	StretchBlt( hDC,0,0,r.right,r.bottom,hMemDC,0,0,w,h,SRCCOPY );
	EndPaint( hWnd,&ps );
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubCommand )
{
	switch( LOWORD( wp ) ){
	// �@�\���j���[
	ONEVENT( IDM_EXIT		, onSubClose		);	// ����
	ONEVENT( IDM_SINGLE		, onSubSingle		);	// ���{�\��
	ONEVENT( IDM_DOUBLE		, onSubDouble		);	// �Q�{�\��
	}
	return DEFPROC();
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�}�E�X�ړ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubMouseMove )
{
	// �J�[�\���`��
	SetCursor( LoadCursor( hIns,MAKEINTRESOURCE( IDC_SPOIT ) ) );
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���N���b�N
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubLButton )
{
	SPOITDATA sd;
	POINT pos;
	HDC	hDC;
	// �X�|�C�g
	GetCursorPos( &pos );
	ScreenToClient( hWnd,&pos );
	hDC = GetDC( hWnd );
	sd.x			= pos.x;
	sd.y			= pos.y;
	sd.colorcode	= GetPixel( hDC,pos.x,pos.y );
	sd.red			= GET_RED( sd.colorcode );
	sd.green		= GET_GREEN( sd.colorcode );
	sd.blue			= GET_BLUE( sd.colorcode );
	ReleaseDC( hWnd,hDC );
	spShow( hWnd,&sd );
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�T�C�Y�ύX
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubSize )
{
	InvalidateRect( hWnd,NULL,FALSE );
	return 0;
}

// =====================================================
// ���j���[�C�x���g

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���{�\��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubSingle		)
{
	SetWindowPos( hOrgWnd,NULL,0,0,ww,wh,SWP_NOMOVE | SWP_NOZORDER );	
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�Q�{�\��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSubDouble		)
{
	SetWindowPos( hOrgWnd,NULL,0,0,ww+w,wh+h,SWP_NOMOVE | SWP_NOZORDER );	
	return 0;
}
