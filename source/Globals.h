// -------------------------------------------------------------
//	BMP to MSX �O���[�o���ϐ�
//												(C)2001	HRA!
// -------------------------------------------------------------

#ifndef	_GLOBALS_H_
#define	_GLOBALS_H_

#include <windows.h>

// -------------------------------------------------------------
//	�C���X�^���X��������

#ifdef	_INSTANCE_
#define	EXTERN
#else
#define	EXTERN	extern
#endif

// -------------------------------------------------------------
//	�ϐ�

EXTERN	char		g_sPlug[ 260 ];								//	�v���O�C���t�H���_
EXTERN	char		*cmd;										//	�R�}���h���C��
EXTERN	HWND		hOrgWnd;									//	�I���W�i���摜�E�B���h�E
EXTERN	HINSTANCE	hIns;										// �C���X�^���X�n���h��:main.cpp
EXTERN	HMENU		hMenu;										// ���j���[�n���h��:main.cpp

// -------------------------------------------------------------
//	��n��

#ifdef	_INSTANCE_
#undef	_INSTANCE_
#endif

#endif	//	ndef _GLOBALS_H_
