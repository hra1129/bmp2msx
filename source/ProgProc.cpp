// -------------------------------------------------------------
// BMP to MSX
// �o�ߕ\���v���O���X�o�[�_�C�A���O
//											(C)2000 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include "ProgProc.h"
#include "winuty.h"
#include "resource.h"

// -------------------------------------------------------------
//	�ϐ�

static int	nCount = 0;		//	�\���J�E���^
static HWND	hProg  = NULL;	//	�_�C�A���O�n���h��

// -------------------------------------------------------------
// �v���g�^�C�v

WINPROC( ProgProc );
EVENT( onPrgInitDlg		);

// -------------------------------------------------------------
//	�֐���`

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���O���X�o�[�_�C�A���O�̕\��
//	2.	����
//		cszText	...	(I)	�\�����镶����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void prShow( const char* cszText )
{
	if( hProg == NULL ) {
		hProg = CreateDialog( wu_get_instance(), MAKEINTRESOURCE( IDD_PROGRESS ), NULL, (DLGPROC)ProgProc );
		ShowWindow( hProg,SW_SHOW );
	}
	SendMessage( GetDlgItem( hProg, IDC_PROGRESS ), PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
	wu_move_center( hProg, NULL );
	prSetText( cszText );
	++nCount;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�e�L�X�g�̕ύX
//	2.	����
//		cszText	...	(I)	�\�����镶����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void prSetText( const char* cszText )
{
	if( hProg != NULL ) {
		SetDlgItemText( hProg, IDC_FILENAME, cszText );
		SendMessage( GetDlgItem( hProg, IDC_PROGRESS), PBM_SETPOS, 0, 0 );
		SetFocus( hProg );
		InvalidateRect( GetDlgItem( hProg, IDC_FILENAME), NULL, TRUE );
	}
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���O���X�o�[�\���̔p��
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void prDestroy( void )
{
	--nCount;
	if( nCount < 1 ) {
		if( hProg != NULL ) DestroyWindow( hProg );
		hProg = NULL;
		nCount = 0;
	}
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�o�ߕ\���i�R�[���o�b�N�p�֐��j
//	2.	����
//		p		...	(I)	0�`100 �ɂ��o�߂̋
//	3.	�Ԓl
//		true	...	���s
//		false	...	���~
//	4.	���l
//		�ϊ������̃R�[���o�b�N�Ƃ��Ďw�肷��B
// -----------------------------------------------------
bool prProg( int p )
{
	SendMessage( GetDlgItem(hProg,IDC_PROGRESS),PBM_SETPOS,p,0 );
	return true;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�C�x���g����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�֐�
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		�C�x���g�����֐��Ɉˑ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
WINPROC( ProgProc )
{
	switch( msg )
	{
	ONEVENT( WM_INITDIALOG	, onPrgInitDlg		);
	}
	return FALSE;
}

// =====================================================
// �C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�������C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�������Ȃ�
// -----------------------------------------------------
EVENT( onPrgInitDlg )
{
	return TRUE;
}
