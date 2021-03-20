// -----------------------------------------------------
// BMP to MSX
// �v���O�C���_�C�A���O
//										(C)2001/5/11 HRA!
// -----------------------------------------------------

#include <windowsx.h>

#include "PlugProc.h"
#include "Resource.h"
#include "config.h"
#include "winEvent.h"
#include "winuty.h"

// -----------------------------------------------------
// �v���g�^�C�v

WINPROC( PlugProc );

EVENT( onPlugClose		);
EVENT( onPlugCommand	);
EVENT( onPlugInitDlg	);
EVENT( onPlugOk			);
EVENT( onPlugPaint		);
EVENT( onPlugFolder		);

// -----------------------------------------------------
// �O���[�o���ϐ�

static int		g_nMax	= 0;
static char*	g_pText	= NULL;

// -----------------------------------------------------
//	�֐���`

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�_�C�A���O�̕\��
//	2.	����
//		hWnd	...	(I)	�e�E�B���h�E
//		pText	...	(O)	�p�X���i�[����A�h���X
//		nMax	...	(I)	pText �̃T�C�Y
//	3.	�Ԓl
//		-1	...	�X�V����
//		0	...	�X�V����
//	4.	���l
//		�L�����Z������������ pText �̓��e�͕ω����Ȃ�
// -----------------------------------------------------
int piShow( HWND hWnd, char *pText, int nMax )
{
	int i;

	g_nMax	= nMax;
	g_pText	= pText;
	i = DialogBox( wu_get_instance(), MAKEINTRESOURCE( IDD_PLUGIN ), hWnd, (DLGPROC)PlugProc );
	g_nMax	= 0;
	return i;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	�����ς�
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
WINPROC( PlugProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onPlugClose	);
		ONEVENT( WM_COMMAND		, onPlugCommand	);
		ONEVENT( WM_INITDIALOG	, onPlugInitDlg	);
		ONEVENT( WM_PAINT		, onPlugPaint	);
	}
	return FALSE;
}

// =====================================================
// �C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	�����ς�
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugClose )
{
	EndDialog( hWnd, -1 );
	return TRUE;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	�����ς�
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugCommand )
{
	switch( LOWORD( wp ) )
	{
	ONEVENT( IDOK		, onPlugOk			);		// [OK]
	ONEVENT( IDCANCEL	, onPlugClose		);		// [Cancel]
	ONEVENT( IDC_FOLDER	, onPlugFolder		);		// [...]
	}
	return TRUE;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�_�C�A���O�̏�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	�����ς�
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugInitDlg )
{
	//	�p�X���X�V����
	SetDlgItemText( hWnd, IDC_PATH, g_pText );
	return TRUE;	// �����ōŏ��̃t�H�[�J�X�𓖂ĂĂ��炤
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�n�j�{�^��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugOk )
{
	//	�p�X���X�V����
	GetDlgItemText( hWnd, IDC_PATH, g_pText, g_nMax );
	EndDialog( hWnd, 0 );
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
//		TRUE	...	�����ς�
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd,&ps );
	EndPaint( hWnd,&ps );
	return TRUE;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�t�H���_�̑I��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	�����ς�
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugFolder )
{
	char	szFileName[ MAX_PATH ];
	char	*ptr;
	OPENFILENAME	ofn;

	// �t�@�C���������
	szFileName[0]='\0';
	ZeroMemory( &ofn,sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= wu_get_instance();
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefPlug;
#ifdef _ENGLISH
	ofn.lpstrTitle		= "PlugIn Folder";
#else
	ofn.lpstrTitle		= "��׸޲�̫���";
#endif
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= g_pText;
	if( !GetOpenFileName( &ofn ) ) return 0;

	//	�p�X�̐ݒ�
	ptr = strrchr( szFileName, '\\' );
	if( ptr != NULL ) {
		ptr[1] = '\0';
	}
	SetDlgItemText( hWnd, IDC_PATH, szFileName );

	return 0;
}
