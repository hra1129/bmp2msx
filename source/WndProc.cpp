// -----------------------------------------------------
// BMP to MSX
// ���C���E�B���h�E
//									(C)2000/5/6 HRA!
// -----------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <htmlhelp.h>		//	HTMLhelp workshop �ɕt��
#include "Config.h"
#include "winEvent.h"
#include "Resource.h"
#include "Setting.h"
#include "Ver.h"
#include "Convert.h"
#include "ProgProc.h"
#include "string.h"
#include "bsave.h"
#include "utils.h"
#include "SubProc.h"
#include "ScriptProc.h"
#include "winuty.h"
#include "splug.h"
#include "PreView.h"
#include "ProgProc.h"
#include "globals.h"
#include "PlugProc.h"
#include "CalibrationProc.h"

// -----------------------------------------------------
// �ݒ�

static const int mode[] = { 2, 3, 5, 6, 7, 8, 10, 12, 5, 6, 7, 8, 10, 12 };	// ��ʃ��[�h�ԍ�

// ��ʃT�C�Y
static const SSIZE ScrSize[]={
	{ 256, 192 },	// screen 2/4
	{  64,  48 },	// screen 3
	{ 256, 212 },	// screen 5
	{ 512, 212 },	// screen 6
	{ 512, 212 },	// screen 7
	{ 256, 212 },	// screen 8
	{ 256, 212 },	// screen 10/11
	{ 256, 212 },	// screen 12
	{ 256, 256 },	// screen 5 (256 lines)
	{ 512, 256 },	// screen 6 (256 lines)
	{ 512, 256 },	// screen 7 (256 lines)
	{ 256, 256 },	// screen 8 (256 lines)
	{ 256, 256 },	// screen 10/11 (256 lines)
	{ 256, 256 },	// screen 12 (256 lines)
};

// �t���O
bool bSave	= false;	// �I�����ɐݒ��ۑ����邩�ۂ�
bool bSrc	= false;	// �v���r���[�\���̓\�[�X���ہi�f�B�X�e�B�l�[�V�����j��
bool bData	= true;		// �ݒ����\�����邩�ۂ�

#ifdef _ENGLISH

#define cszFontName	"MS Gothic"						// font
const char *cSetting  = "Setting";
const char *cOriginal = "Original";
const char *cResult	  = "Result";
const char *cClipboard= "Clipboard";
const char *cUse	  = "Use";
const char *cNoUse	  = "No use";
const char *cNonZero  = "No use 'color 0'";
const char *cUseZero  = "use 'color 0'";

const char *cShowSet	  = "[%s]\n"
						"Screen Mode = %s\n"
						"Output = %s\n"
						"Error litter = %s : multiply = 0.%03d : ignore error(max)=%d\n"
						"All custom palette = %s : color 0 = %s\n"
						"Recalc J and K = %s\n"
						"Interace = %s\n"
						"Dither = %s : Dither synthesize = %s\n"
						"Bit = %s\n"
						"SC10-12 algolithm = %s\n"
						"Auto output file name = %s\n"
						"Palette data = %s\n"
						"50%%:50%% net tile = %s\n";

#else

#define cszFontName	"�l�r �S�V�b�N"					// �t�H���g��
const char *cSetting  = "�ݒ�";
const char *cOriginal = "�ϊ���";
const char *cResult	  = "�ϊ�����";
const char *cClipboard= "�د���ް�މ摜";
const char *cUse	  = "�g�p";
const char *cNoUse	  = "���g�p";
const char *cNonZero  = "0�Ԏg�p�֎~";
const char *cUseZero  = "0�Ԃ��g�p����";

const char *cShowSet	  = "[%s]\n"
						"���Ӱ�� = %s\n"
						"�o�͐� = %s\n"
						"�덷�g�U = %s : �W�� = 0.%03d : �̌덷=%d\n"
						"�S�Œ���گ� = %s : 0�Ԃ̐F = %s\n"
						"JK�Čv�Z = %s\n"
						"����ڰ� = %s\n"
						"�ި�� = %s : �ި�ލ������@ = %s\n"
						"�F�Č����x = %s\n"
						"���R��ٺ�ؽ�� = %s\n"
						"����̧�ٖ� = %s\n"
						"��گďo�͕��@ = %s\n"
						"50%%:50%%����� = %s\n";

#endif

// -----------------------------------------------------
// �v���g�^�C�v

// ��ʃC�x���g
EVENT( onCreate				);
EVENT( onClose				);
EVENT( onCommand			);
EVENT( onDestroy			);
EVENT( onDropFiles			);
EVENT( onLButton			);
EVENT( onRButton			);
EVENT( onPaint				);
EVENT( onInitMenu			);
// �t�@�C�����j���[�C�x���g
EVENT( onOpen				);
EVENT( onScript				);
// �ݒ胁�j���[�C�x���g
EVENT( onDefault			);
EVENT( onSetting			);
EVENT( onPlugIn				);
EVENT( onColorCalibration	);

// �w���v���j���[�C�x���g
EVENT( onHelp				);
EVENT( onVer				);
// �ҏW���j���[�C�x���g
EVENT( onPaste				);
// �v���r���[���j���[�C�x���g
EVENT( onMenuDelete			);
EVENT( onMenuAllDelete		);
EVENT( onMenuSave			);
EVENT( onMenuSaveAs			);
EVENT( onMenuBmpSave		);
EVENT( onMenuRedo			);
EVENT( onMenuData			);
EVENT( onMenuZoomUp			);
EVENT( onMenuZoomDown		);
// �v���r���[ - �\�� ���j���[�C�x���g
EVENT( onMenuNext			);
EVENT( onMenuPrev			);
EVENT( onMenuChange			);
EVENT( onMenuOriginal		);

void Converter( SETTING *Mode,int nSrc );
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y );
bool DoEvents( void );	// main.cpp
static void MakeSettingStr( char *szBuf,const char *szTitle,SETTING *Mode );
static void UpdateVScreen( void );
static bool CreatePal( int &n,SETTING *Mode,COLORREF *in,int w,int h,COLORREF *Pal );

// -----------------------------------------------------
// �O���[�o���ϐ�

static HDC		hMemDC = NULL;			// ���z���
static HBITMAP	hMemBM = NULL;			// ���z���
static HDC		hBGDC  = NULL;			// ���S���
static HBITMAP	hBGBM  = NULL;			// ���S���
static HWND		hWnd   = NULL;			// �E�B���h�E�n���h��
SETTING NowMode;						// ���݂̐ݒ�
static SETTING	LastMode;				// �ŏI�ϊ��ݒ�
extern int	nZoom;						// �\���{��

extern bool	bRedo;						// �ĕϊ�����̫�Đݒ肩

// =====================================================

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		���b�Z�[�W�����֐��Ɉˑ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
WINPROC( WndProc )
{
	switch( msg )
	{
		ONEVENT( WM_CREATE		, onCreate		);
		ONEVENT( WM_CLOSE		, onClose		);
		ONEVENT( WM_COMMAND		, onCommand		);
		ONEVENT( WM_DESTROY		, onDestroy		);
		ONEVENT( WM_DROPFILES	, onDropFiles	);
		ONEVENT( WM_LBUTTONDOWN	, onLButton		);
		ONEVENT( WM_RBUTTONDOWN , onRButton		);
		ONEVENT( WM_PAINT		, onPaint		);
		ONEVENT( WM_INITMENU		, onInitMenu	);
	}
	return DEFPROC();
}

// =====================================================
// ��ʃC�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		�R�}���h�����֐��Ɉˑ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onCommand )
{
	switch( LOWORD( wp ) )
	{
	// ���j���[
	ONEVENT( IDM_EXIT				,onClose			);	// �I��
	ONEVENT( IDM_OPEN				,onOpen				);	// �t�@�C�����J��
	ONEVENT( IDM_SCRIPT				,onScript			);	// �X�N���v�g���[�h
	// �ҏW���j���[
	ONEVENT( IDM_PASTE				,onPaste			);	// �y�[�X�g
	// �v���r���[���j���[
	ONEVENT( IDM_NEXT				,onMenuNext			);	// ��
	ONEVENT( IDM_PREV				,onMenuPrev			);	// �O
	ONEVENT( IDM_CLEAR				,onMenuDelete		);	// �폜
	ONEVENT( IDM_ALLCLEAR			,onMenuAllDelete	);	// �S�폜
	ONEVENT( IDM_SAVE				,onMenuSave			);	// �ۑ�
	ONEVENT( IDM_SAVEAS				,onMenuSaveAs		);	// ���O��t���ĕۑ�
	ONEVENT( IDM_BMPSAVE			,onMenuBmpSave		);	// �a�l�o�ŕۑ�
	ONEVENT( IDM_REDO				,onMenuRedo			);	// �ĕϊ�
	ONEVENT( IDM_CHANGE				,onMenuChange		);	// �ϊ���/�ϊ����ʐ؂�ւ�
	ONEVENT( IDM_DATA				,onMenuData			);	// �ݒ�̕\���؂�ւ�
	ONEVENT( IDM_ORIGNAL			,onMenuOriginal		);	// �I���W�i���摜�C���[�W
	ONEVENT( IDM_ZOOMUP				,onMenuZoomUp		);	// �g��
	ONEVENT( IDM_ZOOMDOWN			,onMenuZoomDown		);	// �k��
	// �ݒ胁�j���[
	ONEVENT( IDM_DEFSET				,onDefault			);	// �ݒ�̏�����
	ONEVENT( IDM_SETTING			,onSetting			);	// �ݒ�
	ONEVENT( IDM_PLUGIN				,onPlugIn			);	// �v���O�C���ݒ�
	ONEVENT( IDM_COLOR_CALIBRATION, onColorCalibration	);
	// �w���v���j���[
	ONEVENT( IDM_HELP				,onHelp				);	// �w���v�̕\��
	ONEVENT( IDM_VER				,onVer				);	// �o�[�W�������
	}
	return DEFPROC();
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E������
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		-1	...	�E�B���h�E�������s
//		0	...	�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onCreate )
{
	HDC		hDC;
	char	szFileName[ MAX_PATH ] = "";
	char	szBuf[ MAX_PATH ];
	int		i,j,n;
	// �v���r���[�̃N���A
	pvInitialize();
	// �E�B���h�E�n���h��
	::hWnd = hWnd;
	// ���[�h�ݒ���N���A����
	bSave = !GetCfgFile( &NowMode, CfgFile );
	bSave = !GetPathFile( PathFile, g_sPlug, sizeof( g_sPlug ) ) || bSave;
	bSave = !GetCustomFile( CustomFile ) || bSave;
	spi_initialize( g_sPlug );
	LastMode.Mode = -1;
	// ���z��ʂ��쐬����
	hDC = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hDC );
	hMemBM = CreateCompatibleBitmap( hDC, cnWidth, cnHeight );
	hBGDC  = CreateCompatibleDC( hDC );
	hBGBM  = LoadBitmap( hIns,MAKEINTRESOURCE( IDB_LOGO ) );
	ReleaseDC( hWnd,hDC );
	if( hMemDC==NULL || hMemBM==NULL || hBGDC ==NULL || hBGBM ==NULL ) return -1;
	SelectBitmap( hMemDC,hMemBM );
	SelectBitmap( hBGDC ,hBGBM  );
	// ��ʂ̍X�V
	UpdateVScreen();
	// �R�}���h���C��
	if( lstrlen( cmd ) ){
		// �����񂩂� " ����菜��
		i=j=0;
		while( cmd[i]!='\0' && j<MAX_PATH-1 ){
			if( cmd[i]!='"' ){
				szFileName[j]=cmd[i];
				j++;
			}
			i++;
		}
		szFileName[j]='\0';
		// �_�C�A���O�{�b�N�X��\��
		GetFileTitle( szFileName, szBuf, sizeof( szBuf ) );
		prShow( szFileName );
		// �ϊ�
		if( (n=bdAddFile( szFileName ))==-1 ){
			MessageBox( hWnd,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
			prDestroy();
			return 0;
		}
		Converter( &NowMode,n );
		// �p��
		if( NowMode.PreView == 1 ){
			bdDelete(n);
		}
		prDestroy();
	}
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onClose )
{
	DestroyWindow( hWnd );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�p����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onDestroy )
{
	if( hBGDC !=NULL ) DeleteDC( hBGDC );
	if( hBGBM !=NULL ) DeleteBitmap( hBGBM );
	if( hMemDC!=NULL ) DeleteDC( hMemDC );
	if( hMemBM!=NULL ) DeleteBitmap( hMemBM );
	pvAllDelete();
	hBGDC =NULL;
	hBGBM =NULL;
	hMemDC=NULL;
	hMemBM=NULL;
	DestroyWindow( hOrgWnd );
	if( bSave ) {
		SaveCfgFile( &NowMode, CfgFile );
		SavePathFile( PathFile, g_sPlug, sizeof( g_sPlug ) );
		SaveCustomFile( CustomFile );
	}
	bSave = false;
	PostQuitMessage(0);
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���h���b�v
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onDropFiles )
{
	HDROP	hDrop;
	char	szBuf[ MAX_PATH ];
	int		i,j,n;
	hDrop = (HDROP)wp;
	SETTING	tmp;

	// �t�@�C�������擾
	j = DragQueryFile( hDrop, 0xFFFFFFFFL, szBuf, MAX_PATH );
	//	�o�߂�\��
	prShow( "" );
	// �S�t�@�C������������
	for( i = 0; i < j; i++ ){
		// �h���b�v���ꂽ�t�@�C�������擾����
		DragQueryFile( hDrop, i, szBuf, MAX_PATH );
		// �o�߂Ƀt�@�C������\��
		prSetText( szBuf );
		// �t�@�C����ϊ�����
		if( (n=bdAddFile( szBuf ))==-1 ){
			MessageBox( hWnd, cszBDErr, MsgCap, MB_OK | MB_ICONWARNING );
			break;
		}
		switch( NowMode.CnvMode ) {
		case CM_DEFAULT:
			tmp = NowMode;
			break;
		case CM_LAST:
			if( LastMode.Mode == -1 ) {
				tmp = NowMode;
			} else {
				tmp = LastMode;
			}
			break;
		case CM_IMAGE:
			if( pvIndex < 0 ) {
				tmp = NowMode;
			} else {
				tmp = tPreview[ pvIndex ].tMode;
			}
			break;
		}
		Converter( &tmp, n );
		if( tmp.PreView == 1 ){
			bdDelete(n);
		}
	}
	// �I��
	prDestroy();
	DragFinish( hDrop );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�}�E�X�N���b�N
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onLButton )
{
	if( pvIndex==-1 ){
		MessageBox( hWnd, cszMainClk, MsgCapWrn, MB_OK | MB_ICONWARNING );
		return 0;
	}else{
		return CALLEVENT( onMenuRedo );	// �ĕϊ�
	}
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�}�E�X�E�N���b�N
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onRButton )
{
	HMENU	hMenu, hSubMenu;
	POINT	p;

	hMenu	= GetMenu( hWnd );
	hSubMenu= GetSubMenu( hMenu, 2 );
	GetCursorPos( &p );
	TrackPopupMenu( hSubMenu, TPM_CENTERALIGN | TPM_VCENTERALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hWnd, NULL );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		��ʂ̕`��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	int			z;
	// �X�V�v���̂��镔���̂ݍX�V����
	hDC = BeginPaint( hWnd,&ps );
	z = nZoom + 1;
	StretchBlt( hDC, 0, 0, cnWidth * z, cnHeight * z,
			 hMemDC, 0, 0, cnWidth    , cnHeight    , SRCCOPY );
	EndPaint( hWnd,&ps );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�̏�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onInitMenu ) {
	HANDLE			hMem;
	MENUITEMINFO	mi;
	const int pvitem[]={
		IDM_NEXT,IDM_PREV,IDM_CHANGE,IDM_ORIGNAL,IDM_CLEAR,
		IDM_ALLCLEAR,IDM_SAVE,IDM_SAVEAS,IDM_REDO,IDM_BMPSAVE,
	};
	int i;

	// ���j���[�A�C�e���̏�����
	memset( &mi,0,sizeof( mi ) );
	mi.cbSize	= sizeof( mi );
	mi.fMask	= MIIM_STATE;
	// �N���b�v�{�[�h�ɉ摜�����邩�m�F����
	OpenClipboard(hWnd);
	hMem=GetClipboardData( CF_DIB );
	CloseClipboard();
	// �ҏW�̃y�[�X�g
	mi.fState	= hMem==NULL ? MFS_GRAYED : MFS_ENABLED;
	SetMenuItemInfo( hMenu,IDM_PASTE,FALSE,&mi );
	// �v���r���[
	mi.fState	= pvIndex==-1 ? MFS_GRAYED : MFS_ENABLED;
	for( i=0; i<ELMCNT( pvitem ); ++i ){
		SetMenuItemInfo( hMenu,pvitem[i],FALSE,&mi );
	}
	return 0;
}

// =====================================================
// [�t�@�C��]���j���[�C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C�����J��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onOpen )
{
	char	szFileName[ MAX_PATH ] = "";
	char	szBuf[ MAX_PATH ];
#ifdef _ENGLISH
	char	szFilter[ 64 + SPI_MAXLEN ]	= "All Files(*.*)\0*.*\0BMP file(*.BMP)\0*.BMP\0";
	const	int	nextidx = sizeof( "All Files(*.*)\0*.*\0BMP file(*.BMP)\0*.BMP\0" );
#else
	char	szFilter[ 64 + SPI_MAXLEN ]	= "���ׂĂ�̧��(*.*)\0*.*\0BMP ̧��(*.BMP)\0*.BMP\0";
	const	int	nextidx = sizeof( "���ׂĂ�̧��(*.*)\0*.*\0BMP ̧��(*.BMP)\0*.BMP\0" );
#endif
	OPENFILENAME	ofn;
	int n;
	SETTING	tmp;

	//	�t�B���^�̍쐬
	memcpy( &szFilter[ nextidx - 1 ], spi_get_filter(), SPI_MAXLEN );
	
	// �t�@�C���������
	szFileName[0]='\0';
	ZeroMemory( &ofn,sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= szFilter;
#ifdef _ENGLISH
	ofn.lpstrTitle		= "Open File";
#else
	ofn.lpstrTitle		= "�t�@�C�����J��";
#endif
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// �J�����g
	if( !GetOpenFileName( &ofn ) ) return 0;
	// �t�@�C���ǂݍ���
	if( (n=bdAddFile( szFileName ))==-1 ){
		MessageBox( hWnd,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
		return 0;
	}
	// �_�C�A���O�{�b�N�X��\��
	GetFileTitle( szFileName, szBuf, sizeof( szBuf ) );
	prShow( szBuf );
	// �ϊ�
	switch( NowMode.CnvMode ) {
	case CM_DEFAULT:
		tmp = NowMode;
		break;
	case CM_LAST:
		if( LastMode.Mode == -1 ) {
			tmp = NowMode;
		} else {
			tmp = LastMode;
		}
		break;
	case CM_IMAGE:
		if( pvIndex < 0 ) {
			tmp = NowMode;
		} else {
			tmp = tPreview[ pvIndex ].tMode;
		}
		break;
	}
	Converter( &tmp,n );
	//
	if( tmp.PreView == 1 ){
		bdDelete(n);
	}
	prDestroy();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�X�N���v�g���[�h�E�B���h�E��\������
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onScript )
{
	DialogBox( hIns,MAKEINTRESOURCE( IDD_SCRIPT ),hWnd,(DLGPROC)ScriptProc );
	return 0;
}

// =====================================================
// [�v���r���[]���j���[�C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ�f�[�^�̕\����؂�ւ���
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuData )
{
	bData = !bData;
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�ϊ���/�ϊ����ʂ̕\����؂�ւ���
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuChange )
{
	bSrc = !bSrc;
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���̃v���r���[
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuNext )
{
	pvIndex = pvNextPreview( pvIndex );
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�O�̃v���r���[
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuPrev )
{
	pvIndex = pvPrevPreview( pvIndex );
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�̍폜
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuDelete )
{
	if( pvIndex < 0 ) return 0;
	if( MessageBox( hWnd,cszDel,MsgCapWrn,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )==IDNO ) return 0;
	pvDelete( pvIndex );
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�̑S�폜
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuAllDelete )
{
	if( pvIndex < 0 ) return 0;
	if( MessageBox( hWnd,cszAllDel,MsgCapWrn,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )==IDNO ) return 0;
	pvAllDelete();
	pvIndex = -1;
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�摜�̕ۑ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuSave ) {
	int w,h;
	int	ecode;

	if( pvIndex < 0 ) return 0;
	w=ScrSize[ tPreview[pvIndex].tMode.Mode ].w;
	if( (tPreview[pvIndex].tMode.Mode >= MD_SC5) && !tPreview[pvIndex].tMode.b192 ) {
		h=ScrSize[ tPreview[pvIndex].tMode.Mode ].h;
	} else {
		h=192;
	}
	if( tPreview[pvIndex].tMode.Inter ) h=h*2;
	switch( tPreview[pvIndex].tMode.Mode ) {
	case MD_SC2:
	case MD_SC3:
	case MD_SC5:
	case MD_SC5_256L:
	case MD_SC7:
	case MD_SC7_256L:
		w = w / 2;
		break;
	case MD_SC6:
	case MD_SC6_256L:
		w = w / 4;
		break;
	}
	ecode = bsvSaveBmp( hWnd,tBmpview[tPreview[pvIndex].nSrc].sFileName,
				tPreview[pvIndex].bmp,w,h,&(tPreview[pvIndex].tMode) );
	switch( ecode ) {
	case BSV_ERR_CREATE:
	case BSV_ERR_WRITE:
		MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
		break;
	}
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�摜�ɖ��O��t���ĕۑ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuSaveAs ) {
	int w,h;
	int	ecode;
	SETTING tMode;
	if( pvIndex < 0 ) return 0;
	tMode = tPreview[pvIndex].tMode;
	w=ScrSize[ tMode.Mode ].w; 
	if( (tMode.Mode >= MD_SC5) && !tMode.b192 ) {
		h=ScrSize[ tMode.Mode ].h;
	} else {
		h=192;
	}
	if( tMode.Inter ) h = h * 2;
	tMode.AutoName = false;
	ecode = bsvSaveBmp( hWnd,tBmpview[tPreview[pvIndex].nSrc].sFileName,
			 tPreview[pvIndex].bmp,w,h,&tMode );
	switch( ecode ) {
	case BSV_ERR_CREATE:
	case BSV_ERR_WRITE:
		MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
		break;
	}
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�a�l�o�ۑ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuBmpSave )
{
	SETTING	tMode;
	char	sFileName[ MAX_PATH ];
	
	if( pvIndex < 0 ) return 0;
	tMode = tPreview[pvIndex].tMode;

	// �t�@�C����
	if( !GetName( hWnd,sFileName, sizeof( sFileName ), cszBmpSave, cszBmpFilter, CfgBmp ) ) return 0;

	//	�ۑ�
	if( !cnvSaveBmpFile( sFileName, tPreview[ pvIndex ].bmp, cnWidth, cnHeight, &tMode ) ){
		MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
		return 0;
	}

	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�g��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuZoomUp )
{
	RECT	r = { 0 };
	int		z;

	//	�g��
	nZoom = ( nZoom + 1 ) % cnZoomMax;
	z = nZoom + 1;
	//	�E�B���h�E�̒���
	r.left		= 0;
	r.top		= 0;
	r.right		= cnWidth * z;
	r.bottom	= cnHeight * z;
	AdjustWindowRect( &r,CNWS,TRUE );
	SetWindowPos( hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
	InvalidateRect( hWnd, NULL, FALSE );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�k��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuZoomDown )
{
	RECT	r = { 0 };
	int		z;

	//	�g��
	nZoom = ( nZoom + cnZoomMax - 1 ) % cnZoomMax;
	z = nZoom + 1;
	//	�E�B���h�E�̒���
	r.left		= 0;
	r.top		= 0;
	r.right		= cnWidth * z;
	r.bottom	= cnHeight * z;
	AdjustWindowRect( &r,CNWS,TRUE );
	SetWindowPos( hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
	InvalidateRect( hWnd, NULL, FALSE );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�ĕϊ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuRedo )
{
	SETTING Mode;
	char	szBuf[ MAX_PATH ];
	int nSrc;
	if( pvIndex < 0 ) return 0;
	// �ݒ���s��
	EdtMode = tPreview[ pvIndex ].tMode;
	LastMode = EdtMode;
	lstrcpy( szCaption,cszRedo );
	bRedo = true;
	if( DialogBox( hIns,MAKEINTRESOURCE( IDD_SETTING ),hWnd,(DLGPROC)SetProc )==0 ){
		// �������s��
		Mode = EdtMode;
		nSrc = tPreview[ pvIndex ].nSrc;
		// �_�C�A���O�{�b�N�X��\��
		if( strcmp( cszClipName, tBmpview[ tPreview[ pvIndex ].nSrc ].sFileName ) ) {
			GetFileTitle( tBmpview[ tPreview[ pvIndex ].nSrc ].sFileName, szBuf, sizeof( szBuf ) );
			prShow( szBuf );
		} else {
			prShow( cszClipName );
		}
		// �ϊ�
		if( Mode.PreView & 2 ) ++tBmpview[nSrc].n;
		Converter( &Mode,nSrc );
		// �_�C�A���O�����
		prDestroy();
	}
	bRedo = false;
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�I���W�i���摜�̕\��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onMenuOriginal )
{
	if( pvIndex == -1 ) return 0;	// �v���r���[�������ꍇ�̓I���W�i����\���ł��Ȃ�
	swShow( tBmpview[ tPreview[ pvIndex ].nSrc ].image );
	return 0;
}

// =====================================================
// [�ҏW]���j���[�C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�د���ް�ނ��璣��t��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPaste	)
{
	HGLOBAL	hMem;
	DWORD	size;
	char *bmp,*mem;
	SETTING	tmp;
	int n;
	// �د���ް�ނ��J��
	OpenClipboard(hWnd);
	// �r�b�g�}�b�v���擾����
	hMem=GetClipboardData( CF_DIB );
	if( hMem==NULL ){
		CloseClipboard();
		return 0;
	}
	size = GlobalSize( hMem );
	// ���������m��
	bmp = (char*)LocalAlloc( LMEM_FIXED,size );
	if( bmp==NULL ){
		CloseClipboard();
		return 0;
	}
	mem = (char*)GlobalLock( hMem );
	if( mem != NULL ){
		CopyMemory( bmp, mem, size );
	}
	GlobalUnlock( hMem );
	// �د���ް�ނ����
	CloseClipboard();
	// �摜�t�@�C���C���[�W���L������
	if( (n=bdAddImage( bmp )) == -1 ){
		MessageBox( hWnd,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
		LocalFree( bmp );
		return 0;
	}
	// �_�C�A���O�{�b�N�X��\��
	prShow( cszClipName );
	// �ϊ�����
	switch( NowMode.CnvMode ) {
	case CM_DEFAULT:
		tmp = NowMode;
		break;
	case CM_LAST:
		if( LastMode.Mode == -1 ) {
			tmp = NowMode;
		} else {
			tmp = LastMode;
		}
		break;
	case CM_IMAGE:
		if( pvIndex < 0 ) {
			tmp = NowMode;
		} else {
			tmp = tPreview[ pvIndex ].tMode;
		}
		break;
	}
	tmp.AutoName = false;	// �t�@�C�������������͋����I�ɉ���
	Converter( &tmp,n );
	// ��n��
	if( tmp.PreView == 1 ){
		bdDelete(n);
	}
	prDestroy();
	return 0;
}

// =====================================================
// [�ݒ�]���j���[�C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
// 		�f�t�H���g�̐ݒ�ɖ߂�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onDefault ) {
#ifdef _ENGLISH
	if( MessageBox( hWnd,"Load default setting?","Warning",
					MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )==IDNO ) return 0;
#else
	if( MessageBox( hWnd,"�f�t�H���g�̐ݒ�ɖ߂��܂����H","�m�F",
					MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )==IDNO ) return 0;
#endif
	GetDefCfg(&NowMode);
	bSave = true;
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�F�Z�b�e�B���O
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onSetting ) {
	// ���[�h�ݒ��ҏW�G���A�փR�s�[����
	EdtMode = NowMode;
	lstrcpy( szCaption,cszSetting );
	if( DialogBox( hIns,MAKEINTRESOURCE( IDD_SETTING ),hWnd,(DLGPROC)SetProc )==0 ){
		NowMode = EdtMode;
		bSave = true;
		UpdateVScreen();
	}
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�F�v���O�C���ݒ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onPlugIn ) {
	if( piShow( hWnd, g_sPlug, sizeof( g_sPlug ) ) == 0 ) {
		//	�v���O�C�����X�g���X�V����
		spi_initialize( g_sPlug );
		bSave = true;
	}
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�F�J���[�L�����u���[�V����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onColorCalibration )	{

	if( DialogBox( hIns, MAKEINTRESOURCE( IDD_CALIBRATION ), hWnd, (DLGPROC)CalibrationProc ) == 0 ){
		bSave = true;
	}
	return 0;
}

// =====================================================
// [�w���v]���j���[�C�x���g

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�F�w���v
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onHelp ) {
	char szHelpName[ MAX_PATH ], *ptr;

	strcpy( szHelpName, __argv[0] );
	ptr = strrchr( szHelpName, '\\' );
	if( ptr != NULL ) {
		strcpy( ptr+1, cszHelpName );
	}

	HtmlHelp( hWnd, szHelpName, HH_DISPLAY_TOPIC, 0 );
	return 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�F�o�[�W�������
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
EVENT( onVer )
{
	DialogBox( hIns,MAKEINTRESOURCE( IDD_VER ),hWnd,(DLGPROC)VerProc );
	return 0;
}

// =====================================================
// ���[�e�B���e�B

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C����ϊ�����
//	2.	����
//		Mode	...	(I)	�ݒ�
//		nSrc	...	(I)	�摜�t�@�C���C���[�W�̃C���f�b�N�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void Converter( SETTING *Mode, int nSrc ) {
	static COLORREF Pal[ 256 ];
	static TAILPAT	tail[ TAILMAX ];
	COLORREF *in, *out;
	LPBYTE		bmp;
	SETTING		tMode;
	int			tailcnt = 0;
	int			width, height, owidth, oheight, wwidth, wheight, n, prv=-1;
	char		*ptr = tBmpview[nSrc].image;
	int			i;
	int			ecode;

	// �v���r���[���쐬����
	if( Mode->PreView & 2 ){
		prv=pvBlankIndex();
		if( !pvAddPreview( prv ) ){
			MessageBox( hWnd, cszCannotAddPV, MsgCapWrn, MB_OK | MB_ICONWARNING );
			return;
		}
	}
	// ��������ǂݍ���
	if( !GetBmp( &in,&width,&height,&ptr ) ){
		MessageBox( hWnd, cszNoSupp, MsgCap, MB_OK | MB_ICONWARNING );
		return;
	}
	// ����������ꍇ�̓G���[
	if( (width < 8 || height < 8) && Mode->Resize ){
		LocalFree( in );
		MessageBox( hWnd, cszSmall, MsgCap, MB_OK | MB_ICONWARNING );
		return;
	}
	// �����[�����̐F���擾
	if( Mode->FourceZero == FZ_MATRIX ){
		Mode->FZColor = GetPix( in, width, height, Mode->FZX,Mode->FZY );
	}
	// �ϊ���̃T�C�Y�����߁A���������m�ۂ���
	owidth  = ScrSize[ Mode->Mode ].w;
	if( (Mode->Mode >= MD_SC5) && Mode->b192 ) {
		oheight = 192;
	} else {
		oheight = ScrSize[ Mode->Mode ].h;
	}
	oheight = oheight * ( Mode->Inter ? 2 : 1 );
	out = (COLORREF*)LocalAlloc( LMEM_FIXED,sizeof(COLORREF) * owidth * oheight );
	if( out==NULL ){
		LocalFree( in );
		MessageBox( hWnd, cszNotEnoughMem, MsgCap, MB_OK | MB_ICONWARNING );
		return;
	}
	// �w�i�F�ŃN���A����
	for( i = 0; i < owidth * oheight; ++i ) {
		out[ i ] = Mode->FCColor;
	}
	// �T�C�Y�𒲐߂���
	if( Mode->Resize ) {
		//	�A�X�y�N�g��
		switch( Mode->SizeMode ) {
		case RM_MSX:
			if( (Mode->Mode == MD_SC6) || (Mode->Mode == MD_SC7) || (Mode->Mode == MD_SC6_256L) || (Mode->Mode == MD_SC7_256L) ) {
				wwidth  = width  * 5 / 3;	//	���� 1.66�{
				wheight = height;
			} else {
				wwidth  = width;
				wheight = height * 6 / 5;	//	�c�� 1.2�{
			}
			if( Mode->Inter ) {
				wheight = wheight * 2;
			}
			//	���𒲐�
			if( wwidth > owidth ) {
				wheight = wheight * owidth / wwidth;
				wwidth  = owidth;
			}
			//	�����𒲐�
			if( wheight > oheight ) {
				wwidth  = wwidth * oheight / wheight;
				wheight = oheight;
			}
			break;
		default:
			wwidth  = owidth;
			wheight = oheight;
			break;
		}
		//	���T���v��
		switch( Mode->Resample ) {
		case RS_NORMAL:		//	�m�[�}���g��k��
			cnvNormResize( in, width, height, out, owidth, oheight, wwidth, wheight, prProg,Mode->Seido );
			break;
		case RS_ANTIALIAS:	//	�A���`�G�C���A�X�t���g��k��
			cnvAntiResize( in, width, height, out, owidth, oheight, wwidth, wheight, prProg,Mode->Seido );
			break;
		default:			//	�T�C�Y���ߖ���
			if( Mode->Inter && Mode->Mode != MD_SC6 && Mode->Mode != MD_SC7 && Mode->Mode != MD_SC6_256L && Mode->Mode != MD_SC7_256L ) {
				cnvCopyInter( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
			}else{
				cnvCopy( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
			}
		}
	} else {
		//	�T�C�Y��������
		if( Mode->Inter && Mode->Mode != MD_SC6 && Mode->Mode != MD_SC7 && Mode->Mode != MD_SC6_256L && Mode->Mode != MD_SC7_256L ) {
			cnvCopyInter( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
		}else{
			cnvCopy( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
		}
	}

	LocalFree(in);
	in=NULL;
	// �v���r���[������Ό��摜��ۑ�����
	if( Mode->PreView & 2 ) pvDrawBmp( hMemDC, prv, out, owidth, oheight );
	// �p���b�g�����߂�
	if( !CreatePal( n, Mode, out, owidth, oheight, Pal ) ){
		LocalFree( out );
		return;
	}
	//	��گĂ�Ă���
	if( Mode->Mode < MD_SC8 || (Mode->Mode >= MD_SC5_256L && Mode->Mode < MD_SC8_256L) ) {
		if( !( Mode->Pal && (Mode->SortMode == SM_INCAUTO) ) ) {
			cnvSortPalette( Mode, Pal );
		}
	}
	// �^�C���p�^�[�������߂�
	if( Mode->Tile ){
		switch( Mode->Mode ){
		case MD_SC2:
		case MD_SC3:
		case MD_SC5:
		case MD_SC6:
		case MD_SC7:
		case MD_SC5_256L:
		case MD_SC6_256L:
		case MD_SC7_256L:
			tailcnt = cnvCreateTail4( Mode->Col, Mode->PalEn, !Mode->NonZero, tail, Mode->Mode );
			break;
		}
	}
	// �ϊ����ʂ��i�[���郁�������m�ۂ���
	bmp = (BYTE*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,0x20000 );	//	128KB�m�ہiVRAM�C���[�W���j
	if( bmp==NULL ){
		MessageBox( NULL,cszNotEnoughMem,MsgCap,MB_OK | MB_ICONWARNING );
		return;
	}
	// �ϊ�����(COLORREF�z�񁨂l�r�w�摜)
	if( Mode->Mode < MD_SC10 || (Mode->Mode >= MD_SC5_256L && Mode->Mode <= MD_SC10_256L) ) {	// Screen10/12 �ȊO
		cnvRecolor( out, owidth, oheight, bmp, Mode, prProg, Pal, tail, tailcnt );
	}else{							// Screen10/12
		cnvNtcolor( out, owidth, oheight, bmp, Mode, prProg );
	}
	LocalFree( out );
	out=NULL;
	// �t�H�[�J�X���ڂ�
	SetFocus( hWnd );
	// �ϊ����ʂ�ۑ�����
	if( n == 16) owidth /= 2; else if( n == 4 ) owidth /= 4;
	if( Mode->PreView & 1 ){
		tMode = *Mode;
		if( tBmpview[nSrc].sFileName[0]=='\0' ){			// �N���b�v�{�[�h����̏ꍇ�̓t�@�C������������͖���
			tMode.AutoName = false;
		}
		ecode = bsvSaveBmp( hWnd,tBmpview[nSrc].sFileName,bmp,owidth,oheight,Mode );
		switch( ecode ) {
		case BSV_ERR_CREATE:
		case BSV_ERR_WRITE:
			MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
			break;
		}
	}
	if( Mode->PreView & 2 ){
		tPreview[prv].bmp=bmp;								// �ϊ���� VRAM �C���[�W���L��
		tPreview[prv].tMode=*Mode;							// ���݂̐ݒ���L��
		tPreview[prv].nSrc = nSrc;							// �v���r���[�̌��t�@�C��
		SelectBitmap( hMemDC,tPreview[prv].hBmp );			// �v���r���[���쐬����
		draw_screen( bmp,hMemDC,Mode );
		SelectBitmap( hMemDC,hMemBM );						// ���ɖ߂�
		pvIndex = prv;										// ���̃v���r���[��\��������
		bSrc=false;											// �ϊ����ʂ̉摜��\������
		UpdateVScreen();									// ��ʂ̍X�V
		return;
	}
	// �������̉��
	LocalFree( bmp );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�s�N�Z���l�擾
//	2.	����
//		in		...	(I)	�摜������
//		width	...	(I)	��
//		height	...	(I)	����
//		x		...	(I)	�w���W
//		y		...	(I)	�x���W
//	3.	�Ԓl
//		�F
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y )
{
	// �͈͊O�C��
	if( x<0 ) x=0;
	if( y<0 ) y=0;
	if( x>=width  ) x=width -1;
	if( y>=height ) y=height-1;
	// �w��̈ʒu�̐F��Ԃ�
	return( *( in + x + (height-y)*width ) );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g�̍쐬
//	2.	����
//		n		...	(O)		�p���b�g��
//		Mode	...	(I/O)	�ݒ�
//		in		...	(I)		���͉摜
//		w		...	(I)		���͉摜�̕�
//		h		...	(I)		���͉摜�̍���
//		Pal		...	(O)		�p���b�g�̐F�i�t���J���[�j
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
static bool CreatePal( int &n, SETTING *Mode, COLORREF *in, int w, int h, COLORREF *Pal )
{
	COLORREF	*t;
	int			i, j, pp, pnum;

	n = 0;
	if( Mode->Mode<MD_SC8 || (Mode->Mode >= MD_SC5_256L && Mode->Mode < MD_SC8_256L) ){	// Screen 5, 6 or 7
		j = 0;
		pp = 0;
		if( Mode->Mode == MD_SC6 || Mode->Mode == MD_SC6_256L ) pnum = 4; else pnum = 16;
		for( i = 0 ; i < pnum; ++i ){
			if( Mode->NonZero && i == 0 ) continue;
			if( Mode->PalEn[ i ] == PALEN_NONE ) continue;
			++n;
			if( Mode->PalEn[ i ] == PALEN_USE ){
				Pal[pp]=RGB( convert7to255_r[ Mode->Col[i].red   ],
							 convert7to255_g[ Mode->Col[i].green ],
							 convert7to255_b[ Mode->Col[i].blue	 ] );
				++pp;
			}
		}
		if( n<2 ){
			MessageBox( hWnd, cszPalCnt, MsgCap, MB_OK | MB_ICONWARNING );
			return false;
		}
		if( !Mode->Pal ){		// �œK�p���b�g
			t = (COLORREF*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,sizeof( COLORREF)*w*h );	// ��Ɨ̈�̊m��
			if( t==NULL ){
				MessageBox( hWnd, cszNotEnoughMem, MsgCap, MB_OK | MB_ICONWARNING );
				return false;
			}
			CopyMemory( t, in, w * h * sizeof( COLORREF ) );
			cnvGetPalette( t, w, h, Pal, Mode->SelCol, n, pp, Mode->FourceZero != FZ_NONE,Mode->FZColor );
			LocalFree( t );
			// �œK�p���b�g���Œ�p���b�g�֔��f
			j = pp;
			for( i = 0; i < pnum; ++i ){
				if( Mode->NonZero && i==0 ) continue;
				if( Mode->PalEn[i]==PALEN_AUTO ){
					Mode->Col[i].red	= convert_rgb_to_palette( convert7to255_r, 8, GetRValue( Pal[j] ) );
					Mode->Col[i].green	= convert_rgb_to_palette( convert7to255_g, 8, GetGValue( Pal[j] ) );
					Mode->Col[i].blue	= convert_rgb_to_palette( convert7to255_b, 8, GetBValue( Pal[j] ) );
					++j;
				}
			}
		}
		n = pnum;
		// �Œ�p���b�g
		for( i = 0; i < pnum; ++i ){
			n = 256;
			Pal[i] = RGB(	convert7to255_r[ Mode->Col[i].red   ],
							convert7to255_g[ Mode->Col[i].green ],
							convert7to255_b[ Mode->Col[i].blue  ] );
		}
	}else if( Mode->Mode==MD_SC8 || Mode->Mode==MD_SC8_256L ){	// Screen8
		cnvGetPaletteS8( Pal );
	}
	return true;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		��ʂ��X�V����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
static void UpdateVScreen( void )
{
	HDC		hMDC;
	HDC		hDC;
	char	szBuf[ 1000+MAX_PATH ];
	char	sFileName[ 50+MAX_PATH ];
	char	sPrev[ 50 ];
	HFONT	hFont,hOldFont;
	RECT	rtext	= { 20, 240, 494, 404 };
	RECT	r		= { 0, 0, cnWidth, cnHeight }; 
	int		z;
	// DC�̊m��
	hDC=GetDC( hWnd );
	// ���z��ʂɉ摜��`��
	if( pvIndex == -1 ){
		StretchBlt( hMemDC, 0, 0, cnWidth, cnHeight, hBGDC, 0, 0, cnBGWidth, cnBGHeight, SRCCOPY );
	}else{
		hMDC = CreateCompatibleDC( hDC );
		if( bSrc ){	// �\�[�X�̕\��
			SelectBitmap( hMDC, tPreview[pvIndex].hBmpSrc );
		}else{		// �f�B�X�e�B�l�[�V�����̕\��
			SelectBitmap( hMDC, tPreview[pvIndex].hBmp );
		}
		FillRect( hMemDC, &r, (HBRUSH) GetStockObject( WHITE_BRUSH ) );
		BitBlt( hMemDC, 0, 0, cnWidth, cnHeight, hMDC,0,0, SRCCOPY );
		if( hMDC!=NULL ) DeleteDC( hMDC );
	}
	// �ݒ��`��
	SetBkColor( hMemDC,cnDataBk1 );
	if( pvIndex == -1 ){
		MakeSettingStr( szBuf,cSetting,&NowMode );
	}else{
		wsprintf( sPrev,"(%d/%d) [%s]",pvGetNum( pvIndex ),pvGetCnt(),( bSrc ? cOriginal : cResult ) );
		if( bSrc ) SetBkColor( hMemDC,cnDataBk2 );
		if( tBmpview[tPreview[pvIndex].nSrc].sFileName[0]=='\0' ){
			lstrcpy( sFileName,cClipboard );
		}else{
			GetFileTitle( tBmpview[tPreview[pvIndex].nSrc].sFileName,sFileName,MAX_PATH );
		}
		lstrcat( sFileName,sPrev );
		MakeSettingStr( szBuf,sFileName,&(tPreview[pvIndex].tMode) );
	}
	if( bData ){
		hFont = CreateFont( 12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,SHIFTJIS_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,FIXED_PITCH | FF_DONTCARE,cszFontName );
		hOldFont = SelectFont( hMemDC,hFont );
		SetTextColor( hMemDC,cnDataText );
		DrawText( hMemDC,szBuf,lstrlen(szBuf),&rtext,DT_LEFT );
		SelectFont( hMemDC,hOldFont );
		DeleteFont( hFont );
	}
	// ��ʂɕ`��
	z = nZoom + 1;
	StretchBlt( hDC, 0, 0, cnWidth * z, cnHeight * z,
			 hMemDC, 0, 0, cnWidth    , cnHeight    , SRCCOPY );
	ReleaseDC( hWnd,hDC );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ�\���p������̍쐬
//	2.	����
//		szBuf	...	(I)	�ݒ蕶����̏o�͐�
//		szTitle	...	(I)	�^�C�g��������
//		Mode	...	(I)	�ݒ���e
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void MakeSettingStr( char *szBuf,const char *szTitle,SETTING *Mode )
{
	wsprintf( szBuf,cShowSet,
					szTitle,
					ScrModeName[Mode->Mode],
					PreViewName[Mode->PreView-1],
					(Mode->diffusion_error_enable  ? cUse : cNoUse),(int)( Mode->diffusion_error_coef * 1000 ),Mode->err,
					(Mode->Pal   ? cUse : cNoUse),
					(Mode->NonZero ? cNonZero : cUseZero),
					(Mode->JKrc  ? cUse : cNoUse),
					(Mode->Inter ? cUse : cNoUse),
					ErrAlgoName[Mode->ErrAlgo],ErrAlgo2Name[Mode->ErrAdd],
					ColorSeido[Mode->Seido],
					AlgoName[Mode->AlgoMode],
					(Mode->AutoName ? cUse : cNoUse),
					PltMode[Mode->PltMode],
					(Mode->Tile ? cUse : cNoUse) );
}
