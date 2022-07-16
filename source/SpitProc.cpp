// -----------------------------------------------------
// BMP to MSX
// �X�|�C�g�_�C�A���O
//											(C)2000 HRA!
// -----------------------------------------------------

#include "SpitProc.h"
#include "winEvent.h"
#include "Resource.h"
#include "winuty.h"
#include "convert.h"

// -----------------------------------------------------
// �v���g�^�C�v�錾

// ��ʃC�x���g
EVENT( onSpInitDlg		);
EVENT( onSpClose		);
EVENT( onSpCommand		);
EVENT( onSpPaint		);

// -----------------------------------------------------
// �O���[�o���ϐ�

static SPOITDATA	*sd;
static int	r, g, b;			//	MSX�F

// =====================================================
// �C�x���g����

// -----------------------------------------------------
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
// -----------------------------------------------------
WINPROC( SpitProc )
{
	switch( msg ){
	ONEVENT( WM_INITDIALOG	,onSpInitDlg	);
	ONEVENT( WM_CLOSE		,onSpClose		);
	ONEVENT( WM_COMMAND		,onSpCommand	);
	ONEVENT( WM_PAINT		,onSpPaint		);
	}
	return FALSE;
}

// =====================================================
// ��ʃC�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		������
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	��������
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onSpInitDlg )
{
	char szBuf[ 256 ];
	// �\�����X�V����
	SetDlgItemInt( hWnd,IDC_RED			,sd->red		,FALSE );
	SetDlgItemInt( hWnd,IDC_GREEN		,sd->green		,FALSE );
	SetDlgItemInt( hWnd,IDC_BLUE		,sd->blue		,FALSE );
	r = (sd->red	+ 18 ) * 7 / 255;
	g = (sd->green	+ 18 ) * 7 / 255;
	b = (sd->blue	+ 18 ) * 7 / 255;
	SetDlgItemInt( hWnd,IDC_MSXRED		,r ,FALSE );
	SetDlgItemInt( hWnd,IDC_MSXGREEN	,g ,FALSE );
	SetDlgItemInt( hWnd,IDC_MSXBLUE		,b ,FALSE );
	wsprintf( szBuf,"%08XH",sd->colorcode );
	SetDlgItemText( hWnd,IDC_COLORCODE	,szBuf );
	wsprintf( szBuf,"(%d,%d)",sd->x,sd->y );	
	SetDlgItemText( hWnd,IDC_ZAHYO		,szBuf );
	return TRUE;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	��������
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onSpClose )
{
	EndDialog( hWnd,0 );
	return TRUE;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	��������
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onSpCommand )
{
	switch( LOWORD( wp ) ){
	ONEVENT( IDOK		, onSpClose		);	// ����
	}
	return TRUE;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		��ʂ̕`��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	��������
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onSpPaint )
{
	RECT	rc;
	HWND	hCol;
	HBRUSH	hBr;
	PAINTSTRUCT	ps;
	HDC	hDC;

	//	�T���v���F
	hCol = GetDlgItem( hWnd,IDC_COLOR );
	hDC = GetDC( hCol );
	GetClientRect( hCol,&rc );
	hBr = CreateSolidBrush( sd->colorcode );
	FillRect( hDC,&rc,hBr );
	DeleteBrush( hBr );
	ReleaseDC( hCol,hDC );

	//	�l�r�w�F
	hCol = GetDlgItem( hWnd,IDC_MSXCOLOR );
	hDC = GetDC( hCol );
	GetClientRect( hCol,&rc );
	hBr = CreateSolidBrush( RGB( convert7to255[r], convert7to255[g], convert7to255[b] ) );
	FillRect( hDC,&rc,hBr );
	DeleteBrush( hBr );
	ReleaseDC( hCol,hDC );

	hDC = BeginPaint( hWnd,&ps );
	EndPaint( hWnd,&ps );

	return TRUE;
}

// =====================================================
// ���[�e�B���e�B

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�_�C�A���O�̕\��
//	2.	����
//		hWnd	...	(I)	�e�E�B���h�E�n���h��
//		pdata	...	(I)	�X�|�C�g���f�[�^�̃A�h���X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void spShow( HWND hWnd,SPOITDATA *pdata )
{
	sd = pdata;
	DialogBox( wu_get_instance(), MAKEINTRESOURCE( IDD_DOTVIEW ), hWnd, (DLGPROC)SpitProc );
}
