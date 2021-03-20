// -----------------------------------------------------
// BMP to MSX
// �o�[�W�����\���_�C�A���O
//										(C)2000/5/6 HRA!
// -----------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include "Ver.h"
#include "Resource.h"
#include "config.h"

// -----------------------------------------------------
// �v���g�^�C�v

EVENT( onVerClose		);
EVENT( onVerCommand		);
EVENT( onVerInitDlg		);
EVENT( onVerPaint		);

// -----------------------------------------------------
// �O���[�o���ϐ�

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
WINPROC( VerProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onVerClose	);
		ONEVENT( WM_COMMAND		, onVerCommand	);
		ONEVENT( WM_INITDIALOG	, onVerInitDlg	);
		ONEVENT( WM_PAINT		, onVerPaint	);
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
EVENT( onVerClose )
{
	EndDialog( hWnd,0 );
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
EVENT( onVerCommand )
{
	switch( LOWORD( wp ) )
	{
	ONEVENT( IDOK		, onVerClose		);		// OK
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
EVENT( onVerInitDlg )
{
	//	�o�[�W�����ԍ����Z�b�g����
	SetDlgItemText( hWnd, IDC_VERSION, CFG_VERSION );

	return TRUE;	// �����ōŏ��̃t�H�[�J�X�𓖂ĂĂ��炤
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
EVENT( onVerPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd,&ps );
	EndPaint( hWnd,&ps );
	return TRUE;
}
