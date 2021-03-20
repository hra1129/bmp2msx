// -------------------------------------------------------------
// BMP to MSX
// �N���֐�
//										(C)2000/5/6 HRA!
// -------------------------------------------------------------

#define	_INSTANCE_
#include "globals.h"
#include <windows.h>
#include <windowsx.h>
#include "winEvent.h"
#include "Resource.h"
#include "Config.h"
#include "splug.h"
#include "winuty.h"

// -------------------------------------------------------------
// �ݒ�

// -------------------------------------------------------------
// �v���g�^�C�v

bool InitApp( HINSTANCE hIns,int nShow );
void DestroyApp( void );
WINPROC( WndProc );							// WndProc.cpp
WINPROC( SubProc );							// SubProc.cpp

// -------------------------------------------------------------
// �O���[�o���ϐ�

HACCEL hKey			= NULL;
static HWND hWnd	= NULL;
HMENU hMenuSub		= NULL;

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�N���֐�
//	2.	����
//		hIns	...	(I)	�A�v���̃C���X�^���X�n���h��
//		hpIns	...	(I)	���g�p
//		lpCmd	...	(I)	�R�}���h���C������
//		nShow	,,,	(I)	�E�B���h�E�����\�����
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int WINAPI WinMain( HINSTANCE hIns,HINSTANCE hpIns,LPSTR lpCmd,int nShow )
{
	::cmd=lpCmd;
	::hIns = hIns;
	if( InitApp( hIns,nShow ) ) while( wu_do_events( hWnd, hKey ) );
	DestroyApp();
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�������֐�
//	2.	����
//		hIns	...	(I)	�C���X�^���X�n���h��
//		nShow	...	(I)	�E�B���h�E�̏����\�����
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		���s���Ă� DestroyApp() �͌Ăяo���Ȃ���΂Ȃ�Ȃ�
// -----------------------------------------------------
bool InitApp( HINSTANCE hIns,int nShow )
{
	WNDCLASSEX	wc;
	RECT		r={0,0,cnWidth,cnHeight};

	wu_set_instance( hIns );
	
	wc.cbClsExtra		= 0;
	wc.cbSize			= sizeof( wc );
	wc.cbWndExtra		= 0;
	wc.hbrBackground	= NULL;
	wc.hCursor			= LoadCursor( NULL,IDC_ARROW );
	wc.hIcon			= LoadIcon( hIns,MAKEINTRESOURCE( IDI_ICON ) );
	wc.hIconSm			= (HICON)LoadImage( hIns,MAKEINTRESOURCE( IDI_ICON ),
						  IMAGE_ICON,16,16,LR_DEFAULTCOLOR );
	wc.hInstance		= hIns;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= CSZCLASSNAME;
	wc.lpszMenuName		= NULL;
	wc.style			= NULL;
	if( !RegisterClassEx( &wc ) ) return false;

	wc.hCursor			= NULL;
	wc.lpszClassName	= CSZCLASSNAMESUB;
	wc.lpfnWndProc		= SubProc;
	if( !RegisterClassEx( &wc ) ) return false;
	
	AdjustWindowRect( &r,CNWS,TRUE );
	hMenu	= LoadMenu( hIns,MAKEINTRESOURCE( IDR_MENU    ) );
	hMenuSub= LoadMenu( hIns,MAKEINTRESOURCE( IDR_MENUSUB ) );
	hWnd	= CreateWindowEx(	WS_EX_ACCEPTFILES,CSZCLASSNAME,CSZTITLE,CNWS,
								CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,NULL,hMenu   ,hIns,NULL );
	hOrgWnd	= CreateWindowEx(	0,CSZCLASSNAMESUB,CSZTITLESUB,CNWSS,
								CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,hWnd,hMenuSub,hIns,NULL );
	hKey	= LoadAccelerators( hIns,MAKEINTRESOURCE( IDR_KEY ) );
	if( hWnd==NULL || hOrgWnd==NULL ) return false;
	ShowWindow( hWnd   ,nShow   );
	ShowWindow( hOrgWnd,SW_HIDE );
	UpdateWindow( hWnd );

	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��n���֐�
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		InitApp() �Ŋm�ۂ������\�[�X���������
// -----------------------------------------------------
void DestroyApp( void )
{
}
