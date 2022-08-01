// -------------------------------------------------------------
// BMP to MSX
// �X�N���v�g�E�B���h�E
//											(C)2000 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ScriptProc.h"
#include "Resource.h"
#include "utils.h"
#include "config.h"
#include "Setting.h"
#include "calc.h"
#include "ProgProc.h"
#include "winuty.h"
#include "PreView.h"
#include "fileuty.h"

void Converter( SETTING *Mode,int nSrc );			// WndProc.cpp : �ϊ������֐�
#define OFFSET( t,m ) (int)( &( ((t*)NULL)->m ) )	// �I�t�Z�b�g�A�h���X��Ԃ�

// -------------------------------------------------------------
// �ݒ�

#define cnFontSize	16								// �X�N���v�g�̃t�H���g�T�C�Y
#define TXT_SIZE	30000							// �X�N���v�g�t�@�C���̍ő�T�C�Y
#define WORD_MAX	32								// �P��ő咷
#define MSG_MAX		1024							// ���b�Z�[�W�{�b�N�X������

#ifdef _ENGLISH
#define cszFontName	"MS Gothic"						// font of script text.
#else
#define cszFontName	"�l�r �S�V�b�N"					// �X�N���v�g�̃t�H���g��
#endif

typedef struct {
	char	*word;
	int		id;
} SCRWORD;

typedef bool (*SCRFUNC)( const char *szText,int *ptr );

enum {
	SW_NONE=-1,			// �G���[
	SW_LOADSETTING=0,	// �ݒ�Ǎ�
	SW_CONVERT,			// �ϊ�
	SW_SETFILENAME,		// �t�@�C�����ݒ�
	SW_COMMENT,			// �R�����g
	SW_SCREEN,			// �o�͉�ʃ��[�h
	SW_MESSAGE,			// ���b�Z�[�W�o��
	SW_SETFLAG,			// �e��t���O�ݒ�
	SW_SETTING,			// �ݒ�_�C�A���O
	SW_PALETTE,			// �p���b�g�̐ݒ�
	SW_PALMODE,			// �p���b�g�̃��[�h��ݒ�
	SW_CUTERR,			// �؂�̂Č덷
	SW_GOSAVAL,			// �덷�g�U�W��
	SW_SELCOL,			// �I�F���[�h
	SW_DTHADD,			// �f�B�U���Z���@
	SW_ALGOMODE,		// ���R�������A���S���Y��
};

// -------------------------------------------------------------
// �v���g�^�C�v

EVENT( onScriptClose		);
EVENT( onScriptCommand		);
EVENT( onScriptInitDlg		);
EVENT( onScriptPaint		);
EVENT( onScriptWinChg		);
EVENT( onScriptNew			);
EVENT( onScriptOpen			);
EVENT( onScriptSave			);
EVENT( onScriptDropFiles	);
EVENT( onScriptRun			);

static void scrGetWord( const char *szText,int *ptr,char *word,int WordMax );
static int scrSearch( const char *word,SCRWORD *sword,int n );
static void scrSkipSpace( const char *szText,int *ptr );
static bool scrGetString( const char *szText,int *ptr,char *word,int WordMax );
static bool scrGetNumber( const char *szText,int *ptr,int *num );
static inline bool scrIsSpace( char c );

// �X�N���v�g�R�}���h�֐�
bool scrLoadSetting( const char *szText,int *ptr );
bool scrConvert( const char *szText,int *ptr );
bool scrSetFileName( const char *szText,int *ptr );
bool scrComment( const char *szText,int *ptr );
bool scrScreen( const char *szText,int *ptr );
bool scrMessage( const char *szText,int *ptr );
bool scrSetFlag( const char *szText,int *ptr );
bool scrSetting( const char *szText,int *ptr );
bool scrPalette( const char *szText,int *ptr );
bool scrPalMode( const char *szText,int *ptr );
bool scrCutErr( const char *szText,int *ptr );
bool scrGosaVal( const char *szText,int *ptr );
bool scrPalSelMode( const char *szText,int *ptr );
bool scrDthAdd( const char *szText,int *ptr );
bool scrAlgoMode( const char *szText,int *ptr );

// �֐��z��
static SCRFUNC scrfunc[]={
	scrLoadSetting,				// SW_LOADSETTING
	scrConvert,					// SW_CONVERT
	scrSetFileName,				// SW_SETFILENAME
	scrComment,					// SW_COMMENT
	scrScreen,					// SW_SCREEN
	scrMessage,					// SW_MESSAGE
	scrSetFlag,					// SW_SETFLAG
	scrSetting,					// SW_SETTING
	scrPalette,					// SW_PALETTE
	scrPalMode,					// SW_PALMODE
	scrCutErr,					// SW_CUTERR
	scrGosaVal,					// SW_GOSAVAL
	scrPalSelMode,				// SW_SELCOL
	scrDthAdd,					// SW_DTHADD
	scrAlgoMode,				// SW_ALGOMODE
};

// WndProc
int bdAddFile( const char *cszFileName );
void bdDelete( int Index );

// -------------------------------------------------------------
// �O���[�o���ϐ�

extern HINSTANCE hIns;					// �C���X�^���X�n���h��
extern SETTING	NowMode;				// ���݂̐ݒ�

static HICON	hIcon = NULL;			// �_�C�A���O�̃A�C�R��
static HFONT	hFont = NULL;			// �����t�H���g
static HWND		hWnd  = NULL;			// �_�C�A���O
static HWND		hText = NULL;			// �e�L�X�g�{�b�N�X
static SETTING	scrset;					// �ϊ��ݒ�
static char szTarget[ MAX_PATH ];		// �ϊ��Ώۂ̃t�@�C����

// ���ߌ�
static SCRWORD scrword[]={
	{"LOADSETTING"	, SW_LOADSETTING		},
	{"�ݒ�Ǎ�"		, SW_LOADSETTING		},
	{"CONVERT"		, SW_CONVERT			},
	{"�ϊ�����"		, SW_CONVERT			},
	{"SETFILENAME"	, SW_SETFILENAME		},
	{"�t�@�C����"	, SW_SETFILENAME		},
	{";"			, SW_COMMENT			},
	{"SCREEN"		, SW_SCREEN			},
	{"��ʃ��[�h"	, SW_SCREEN			},
	{"MSG"			, SW_MESSAGE			},
	{"���b�Z�[�W"	, SW_MESSAGE			},
	{"SETFLAG"		, SW_SETFLAG			},
	{"�t���O"		, SW_SETFLAG			},
	{"SETTING"		, SW_SETTING			},
	{"�ݒ��ʕ\��"	, SW_SETTING			},
	{"PALETTE"		, SW_PALETTE			},
	{"�p���b�g"		, SW_PALETTE			},
	{"PALMODE"		, SW_PALMODE			},
	{"�p���b�g����"	, SW_PALMODE			},
	{"CUTERR"		, SW_CUTERR			},
	{"�؂�̂Č덷"	, SW_CUTERR			},
	{"GOSAVAL"		, SW_GOSAVAL			},
	{"�덷�g�U�W��"	, SW_GOSAVAL			},
	{"SELCOLMODE"	, SW_SELCOL			},
	{"�I�F���[�h"	, SW_SELCOL			},
	{"DITHERADD"	, SW_DTHADD			},
	{"�f�B�U���Z"	, SW_DTHADD			},
	{"ALGOMODE"		, SW_ALGOMODE		},
	{"���R�搶��"	, SW_ALGOMODE		},
};

// �t���O��
static SCRWORD flgword[]={
	{"�덷�g�U"			,OFFSET( SETTING, diffusion_error_enable	)},		// �덷�g�U ����:true / ���Ȃ�:false
	{"GOSA"				,OFFSET( SETTING, diffusion_error_enable	)},		// �덷�g�U ����:true / ���Ȃ�:false
	{"�C���^�[���[�X"	,OFFSET( SETTING, interlace					)},		// �C���^�[���[�X ����:true / ���Ȃ�:false
	{"INTERACE"			,OFFSET( SETTING, interlace					)},		// �C���^�[���[�X ����:true / ���Ȃ�:false
	{"�Œ�p���b�g"		,OFFSET( SETTING, fixed_palette				)},		// �Œ�p���b�g ����:true / ���Ȃ�:false
	{"PALNOCHG"			,OFFSET( SETTING, fixed_palette				)},		// �Œ�p���b�g ����:true / ���Ȃ�:false
	{"�T�C�Y����"		,OFFSET( SETTING, resize_enable					)},		// �T�C�Y����
	{"RESIZE"			,OFFSET( SETTING, resize_enable					)},		// �T�C�Y����
	{"�����t�@�C����"	,OFFSET( SETTING, AutoName					)},		// �����t�@�C��������
	{"AUTONAME"			,OFFSET( SETTING, AutoName					)},		// �����t�@�C��������
	{"�F���Čv�Z"		,OFFSET( SETTING, JKrc						)},		// �F�����̍Čv�Z
	{"COLORRECALC"		,OFFSET( SETTING, JKrc						)},		// �F�����̍Čv�Z
	{"�[���g�p�s��"		,OFFSET( SETTING, NonZero					)},		// �O�Ԃ̐F���g��Ȃ�
	{"ZERONOUSE"		,OFFSET( SETTING, NonZero					)},		// �O�Ԃ̐F���g��Ȃ�
	{"�^�C��"			,OFFSET( SETTING, Tile						)},		// �Ԃ����^�C�����g��
	{"TILE"				,OFFSET( SETTING, Tile						)},		// �Ԃ����^�C�����g��
};

// �t���O�l
static SCRWORD flgbool[]={
	{"TRUE"			,1	},
	{"FALSE"		,0	},
	{"ON"			,1	},
	{"OFF"			,0	},
};

// �p���b�g���[�h
static SCRWORD flgpmode[]={
	{"NOUSE"		,PALEN_NONE },
	{"AUTO"			,PALEN_AUTO },
	{"CUSTOM"		,PALEN_USE  },
	{"���g�p"		,PALEN_NONE },
	{"����"			,PALEN_AUTO },
	{"�w��"			,PALEN_USE  },
};

// �I�F���[�h
static SCRWORD flgselmode[]={
	{"SCATTER"		,SELCOL_BUNSAN	},	// ���U�I��
	{"MAX"			,SELCOL_TABUNPU },	// �����z�I��
	{"���U"			,SELCOL_BUNSAN	},	// ���U�I��
	{"�����z"		,SELCOL_TABUNPU },	// �����z�I��
};

// �f�B�U���Z���[�h
static SCRWORD flgdthadd[]={
	{"ROTATE"		,EADD_ROTATE	},	// ��]
	{"NONE"			,EADD_NONE		},	// ���ʑ�
	{"X"			,EADD_X			},	// �w���炵
	{"Y"			,EADD_Y			},	// �x���炵
	{"��]"			,EADD_ROTATE	},	// ��]
	{"���ʑ�"		,EADD_NONE		},	// ���ʑ�
	{"�w���炵"		,EADD_X			},	// �w���炵
	{"�x���炵"		,EADD_Y			},	// �x���炵
};

// ���R�搶���A���S���Y��
static SCRWORD flgalgo[]={
	{"����"			,ALGO_NONE		},	// ����
	{"�Čv�Z"		,ALGO_RECALC	},	// �t�Z�ɂ��덷����
	{"�p���t��"		,ALGO_POWERFUL	},	// �p���t�����p�@�̃��C�g���[�V���O�@�Ɏg���Ă�����
	{"�m�s�r�b��"	,ALGO_NTSC2		},	// NTSC�������ǔ�
	{"�F����"		,ALGO_RECALC2	},	//	�t�Z�ɂ��덷�����i�F�����d���j
	{"NONE"			,ALGO_NONE		},	// ����
	{"RECALC"		,ALGO_RECALC	},	// �t�Z�ɂ��덷����
	{"POWERFUL"		,ALGO_POWERFUL	},	// �p���t�����p�@�̃��C�g���[�V���O�@�Ɏg���Ă�����
	{"NTSC2"		,ALGO_NTSC2		},	// NTSC�������ǔ�
	{"COLOR"		,ALGO_RECALC2	},	//	�t�Z�ɂ��덷�����i�F�����d���j
};

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
WINPROC( ScriptProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE				, onScriptClose		);	// ����
		ONEVENT( WM_COMMAND				, onScriptCommand	);	// �R�}���h����
		ONEVENT( WM_INITDIALOG			, onScriptInitDlg	);	// �_�C�A���O������
		ONEVENT( WM_PAINT				, onScriptPaint		);	// �ĕ`��
		ONEVENT( WM_WINDOWPOSCHANGING	, onScriptWinChg	);	// �E�B���h�E�T�C�Y�ύX���O
		ONEVENT( WM_DROPFILES			, onScriptDropFiles	);	// �t�@�C���h���b�v
	}
	return FALSE;
}

// =====================================================
// �C�x���g

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptClose )
{
	if( hFont != NULL ) DeleteObject( hFont );
	if( hIcon != NULL ) DeleteObject( hIcon );
	hIcon = NULL;
	EndDialog( hWnd,0 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE	...	��������
//		FALSE	...	������
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptCommand )
{
	switch( LOWORD( wp ) )
	{
	// ���j���[�C�x���g
	ONEVENT( IDM_EXIT		, onScriptClose		);		// ����
	ONEVENT( IDM_NEW		, onScriptNew		);		// �V�K�쐬
	ONEVENT( IDM_OPEN		, onScriptOpen		);		// �J��
	ONEVENT( IDM_SAVE		, onScriptSave		);		// �ۑ�
	ONEVENT( IDM_RUN		, onScriptRun		);		// ���s
	}
	return FALSE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�_�C�A���O�̏�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptInitDlg )
{
	::hWnd = hWnd;
	// �A�C�R�����Z�b�g����
	hIcon = LoadIcon( hIns,MAKEINTRESOURCE( IDI_ICONSCRIPT ) );
	SendMessage( hWnd,WM_SETICON,ICON_BIG  ,(LPARAM)hIcon );
	SendMessage( hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon );
	// �t�H���g���Z�b�g����
	hFont = CreateFont( cnFontSize,0,0,0,400,FALSE,FALSE,FALSE,SHIFTJIS_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
						FF_DONTCARE | FIXED_PITCH,cszFontName );
	hText = GetDlgItem( hWnd,IDC_SCRIPT );
	SendMessage( hText,WM_SETFONT,(WPARAM)hFont,TRUE );
	Edit_LimitText( hText,TXT_SIZE-1 );							// �e�L�X�g�ő咷
	return TRUE;	// �����ōŏ��̃t�H�[�J�X�𓖂ĂĂ��炤
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
EVENT( onScriptPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd,&ps );
	EndPaint( hWnd,&ps );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��ʃT�C�Y�ύX
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptWinChg )
{
	RECT r;
	// �E�B���h�E�T�C�Y��ύX
	GetClientRect( hWnd,&r );
	SetWindowPos( hText,0,0,0,r.right,r.bottom,SWP_NOZORDER );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�V�K�쐬
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptNew )
{
	int i;
	i = Edit_GetTextLength( hText );
	// �m�F���b�Z�[�W
	if( i ){
		if( MessageBox( hWnd,cszWrnClear,MsgCapWrn,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )!=IDYES )
			return TRUE;
	}
	// �N���A
	SetWindowText( hText,"" );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C�����J��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptOpen )
{
	char			szFileName[ MAX_PATH ];
	char			szBuf[ TXT_SIZE ],c;
	FU_FILE			*hFile;
	int				i = 0;
	int				tmp;
	OPENFILENAME	ofn;

	// ̧�ٖ������
	szFileName[0]='\0';
	memset( &ofn, 0, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefScrpt;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// �J�����g
	ofn.lpstrTitle		= ScrptLoad;
	if( !GetOpenFileName( &ofn ) ) return TRUE;
	// ̧�ق�ǂݍ���
	hFile = fu_open( szFileName, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		MessageBox( hWnd,cszOpenErr,MsgCap,MB_OK | MB_ICONWARNING );
		return TRUE;
	}
	tmp = fu_read( hFile, szBuf, TXT_SIZE );
	if( tmp > TXT_SIZE ){
		szBuf[ TXT_SIZE-1 ]=0;
	}else if( tmp ){
		szBuf[ tmp ]=0;
	}else{
		szBuf[ 0 ]=0;
	}
	// �o�C�i���f�[�^�������ɓǂݏo�����ꍇ�ɂ̓G���[�ł͂���
	while( (c=szBuf[i++])!=0 ){
		if( c=='\t' || c=='\n' || c=='\r' ) continue;
		if( c<32 && c>0 ){
			MessageBox( hWnd,cszNoText,MsgCap,MB_OK | MB_ICONWARNING );
			CloseHandle( hFile );
			return TRUE;
		}
	}
	fu_close( hFile );
	// �ǂݍ��񂾃e�L�X�g��ݒ肷��
	SetWindowText( hText,szBuf );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ۑ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptSave ) {
	char	szFileName[ MAX_PATH ];
	char	szBuf[ TXT_SIZE ];
	FU_FILE	*hFile;

	// ���g��������Ή������Ȃ�
	if( Edit_GetTextLength( hText )==0 ) return TRUE;
	// �t�@�C�������擾����
	if( !GetName( hWnd, szFileName, MAX_PATH, NULL, cszDefScrpt, NULL ) ) return TRUE;
	// �ۑ�����
	hFile = fu_open( szFileName, "wb" );
	if( hFile == FU_INVALID_HANDLE ) {
		MessageBox( hWnd,cszOpenErr, MsgCap, MB_OK | MB_ICONWARNING );
		return TRUE;
	}
	fu_write( hFile, szBuf, TXT_SIZE );
	fu_close( hFile );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���h���b�v
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptDropFiles )
{
	HDROP	hDrop;
	char	szBuf[ MAX_PATH ];
	char	szBuf2[ MAX_PATH ];
	int		i,j;
	hDrop = (HDROP)wp;
	char szText[ TXT_SIZE ];

	// �e�L�X�g���擾����
	GetWindowText( hText,szText,TXT_SIZE );
	// �t�@�C�������擾
	j=DragQueryFile( hDrop,0xFFFFFFFFL,szBuf,MAX_PATH );
	// �o�߂�\��
	GetFileTitle( szBuf, szBuf2, sizeof( szBuf2 ) );
	prShow( szBuf2 );
	// �S�t�@�C������������
	for( i=0;i<j;i++ ){
		// �h���b�v���ꂽ�t�@�C�������擾����
		DragQueryFile( hDrop,i,szBuf,MAX_PATH );
		// �o�߃_�C�A���O�̕\���X�V
		prSetText( szBuf );
		// �e�L�X�g����������
		scrPlayScript( szText,szBuf );
	}
	// �o�߂��폜
	prDestroy();
	// �I��
	DragFinish( hDrop );
	MessageBox( NULL,cszExit,MsgCap,MB_OK | MB_ICONWARNING );
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���s
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onScriptRun )
{
	char szText[ TXT_SIZE ];
	// �e�L�X�g���擾����
	GetWindowText( hText,szText,TXT_SIZE );
	// �e�L�X�g����������
	scrPlayScript( szText,"" );
	MessageBox( NULL,cszExit,MsgCap,MB_OK | MB_ICONWARNING );
	return TRUE;
}

// =====================================================
// �X�N���v�g����

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�X�N���v�g����
//	2.	����
//		szText		...	(O)	�X�N���v�g�i�[�惁����
//		szFileName	...	(I)	�摜�t�@�C����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void scrPlayScript( char *szText,char *szFileName )
{
	char word[ WORD_MAX ];
	int ptr=0;
	int cmd;
	// �ݒ������������
	scrset = NowMode;
	lstrcpy( szTarget,szFileName );
	// �������[�v
	while( szText[ptr]!=0 ){
		// ���s/�z���C�g�X�y�[�X��ǂݔ�΂�
		scrSkipSpace( szText,&ptr );
		// �P����擾
		scrGetWord( szText,&ptr,word,WORD_MAX );
		if( word[0]==0 ) break;								// ���ׂĎ��s
		// �P��R�[�h������
		cmd = scrSearch( word,scrword,ELMCNT( scrword ) );
		if( cmd != SW_NONE ){
			if( !(scrfunc[cmd])( szText,&ptr ) ) break;		// �R�}���h�֐����ĂуG���[�������͏I��
		}else{
			MessageBox( NULL,cszSyntax,MsgCap,MB_OK | MB_ICONWARNING );
			break;											// �s���ȕ��@
		}
		if( szText[ptr]==0 ) break;
	}
	// ���s���I�����ʒu�ɃJ�[�\�����ړ�����
	Edit_SetSel( hText,ptr,ptr );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�z���C�g�X�y�[�X�Ȃ� true,����ȊO�Ȃ� false ��Ԃ�
//	2.	����
//		c	...	(I)	���肵���������i�P�o�C�g�j
//	3.	�Ԓl
//		true	...	�X�y�[�X
//		false	...	��X�y�[�X
//	4.	���l
//		VC++6.0SP3 �� isspace() �� -21(0xEB)��^����� 8 ���Ԃ��Ă���
//		���ߓ��{�ꍬ�݂��Ǝg���Ȃ�
// -------------------------------------------------------------
static inline bool scrIsSpace( char c )
{
	return( c==' ' || c=='\t' || c=='\xA' || c=='\xD' );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�z���C�g�X�y�[�X��ǂݔ�΂�
//	2.	����
//		szText	...	(I)		�e�L�X�g
//		ptr		...	(I/O)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void scrSkipSpace( const char *szText,int *ptr )
{
	while( scrIsSpace( szText[ *ptr ] ) ) ++(*ptr);
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�P��̎擾
//	2.	����
//		szText	...	(I)		�e�L�X�g
//		ptr		...	(I/O)	�e�L�X�g�̎Q�ƈʒu
//		word	...	(O)		�P��̏o�͐�
//		WordMax	...	(I)		word �̃T�C�Y
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void scrGetWord( const char *szText,int *ptr,char *word,int WordMax )
{
	int i=0;
	--WordMax;
	word[0]=0;
	scrSkipSpace( szText,ptr );
	while( szText[ *ptr ]!='=' && szText[ *ptr ]!=',' && 
		   szText[ *ptr ]!= 0  && !scrIsSpace( szText[ *ptr ] ) && i<WordMax ){
		word[i++]=szText[ (*ptr)++ ];
	}
	word[i]=0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�X�N���v�g�\���̌���
//	2.	����
//		word	...	(I)	�P��
//		sword	...	(I)	�\���z��
//		n		...	(I)	sword �̗v�f��
//	3.	�Ԓl
//		��v����΁Asword �̃C���f�b�N�X
//		��v���Ȃ���� SW_NONE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static int scrSearch( const char *word,SCRWORD *sword,int n )
{
	int i;
	// ����
	for( i=0; i<n; ++i ){
		if( !lstrcmpi( word,sword[i].word ) ) return sword[i].id;
	}
	return SW_NONE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		������̎擾
//	2.	����
//		szText	...	(I)		�e�L�X�g
//		ptr		...	(I/O)	�e�L�X�g�̎Q�ƈʒu
//		word	...	(O)		������̏o�͐�
//		WordMax	...	(I)		word �̃T�C�Y
//	3.	�Ԓl
//		true	...	����
//		false	...	�ُ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static bool scrGetString( const char *szText,int *ptr,char *word,int WordMax )
{
	int i=0;
	--WordMax;
	word[0]=0;
	scrSkipSpace( szText,ptr );
	if( szText[ *ptr ] != '\"' ){
		MessageBox( NULL,cszNoString,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	while( (unsigned)szText[ *ptr ]>=' ' && szText[ *ptr ]!='\"' && i<WordMax ){
		word[i++]=szText[ (*ptr)++ ];
	}
	word[i]=0;
	if( szText[ *ptr ]!='\"' ){
		MessageBox( NULL,cszNoDelim,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���l�̎擾
//	2.	����
//		szText	...	(I)		�e�L�X�g
//		ptr		...	(I/O)	�e�L�X�g�̎Q�ƈʒu
//		num		...	(I)		���l�̊i�[��
//	3.	�Ԓl
//		true	...	����
//		false	...	�ُ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static bool scrGetNumber( const char *szText,int *ptr,int *num )
{
	int p;
	scrSkipSpace( szText,ptr );
	if( !calc_exp( num,&szText[*ptr],&p ) ) return false;
	*ptr += p;
	return true;
}

// =====================================================
// �X�N���v�g�R�}���h

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ��ǂݍ���
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrLoadSetting( const char *szText,int *ptr )
{
	char filename[ WORD_MAX ];
	// �ݒ�t�@�C�������擾����
	if( !scrGetString( szText,ptr,filename,WORD_MAX ) ) return false;
	// �ݒ��ǂݍ���
	if( !GetCfgFile( &scrset,filename ) ){
		MessageBox( NULL,cszNoCfg,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ϊ����������s����
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrConvert( const char *szText,int *ptr )
{
	int n;
	// �t�@�C�������ݒ肳��Ă��Ȃ���ΏI������
	if( szTarget[0]==0 ){
		MessageBox( NULL,cszNoFile,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �_�C�A���O�{�b�N�X��\��
	prShow( szTarget );
	// �ϊ�
	if( (n=bdAddFile( szTarget ))==-1 ){
		MessageBox( NULL,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
		prDestroy();
		return 0;
	}
	Converter( &scrset,n );
	// �p��
	if( scrset.PreView == 1 ){
		bdDelete(n);
	}
	prDestroy();
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C������ݒ肷��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrSetFileName( const char *szText,int *ptr )
{
	// �ݒ�t�@�C�������擾����
	if( !scrGetString( szText,ptr,szTarget,WORD_MAX ) ) return false;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�R�����g����
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrComment( const char *szText,int *ptr )
{
	// �R�����g���I���i�s���������j�܂Ń|�C���^���ړ�����
	while( szText[ *ptr ]!='\xA' && szText[ *ptr ]!='\xD' && szText[ *ptr ]!=0 ) ++(*ptr);
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�o�̓��[�h��ύX����
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrScreen( const char *szText,int *ptr )
{
	int m;
	// ���l�����
	if( !calc_exp( &m,szText,ptr ) ){
		MessageBox( NULL,cszScrMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// ����
	switch( m ){
	case 2:		m=MD_SC2;	break;
	case 3:		m=MD_SC3;	break;
	case 4:		m=MD_SC2;	break;
	case 5:		m=MD_SC5;	break;
	case 6:		m=MD_SC6;	break;
	case 7:		m=MD_SC7;	break;
	case 8:		m=MD_SC8;	break;
	case 10:	m=MD_SC10;	break;
	case 11:	m=MD_SC10;	break;
	case 12:	m=MD_SC12;	break;
	default:
		MessageBox( NULL,cszScrMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	scrset.mode = m;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���b�Z�[�W�{�b�N�X��\������
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrMessage( const char *szText,int *ptr )
{
	char word[ MSG_MAX ];
	// �ݒ�t�@�C�������擾����
	if( !scrGetString( szText,ptr,word,MSG_MAX ) ) return false;
#ifdef _ENGLISH
	MessageBox( NULL,word,"Message",MB_OK );
#else
	MessageBox( NULL,word,"ү����",MB_OK );
#endif
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�e���׸ނ̐ݒ�
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrSetFlag( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int p,b;
	// �P��̎擾
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszNoFlag,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �P��̌���
	p = scrSearch( word,flgword,ELMCNT( flgword ) );
	if( p==SW_NONE || szText[ *ptr ]==0 ){
		MessageBox(  NULL,cszNoFlag,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// '=' ���̋L����ǂݔ�΂�
	scrSkipSpace( szText,ptr );
	++(*ptr);
	scrSkipSpace( szText,ptr );
	// �t���O�̐ݒ�l��ǂݍ���
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszNoBool,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �t���O�l�̌���
	b = scrSearch( word,flgbool,ELMCNT( flgbool ) );
	if( b==SW_NONE ){
		MessageBox(  NULL,cszNoBool,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �l�̑��
	*((bool*)(((char*)&scrset)+p)) = (b!=0);
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ϊ��ݒ�ύX�_�C�A���O�̕\������
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrSetting( const char *szText,int *ptr )
{
	// ���[�h�ݒ��ҏW�G���A�փR�s�[����
	EdtMode = scrset;
	lstrcpy( szCaption,cszScrSetting );
	if( DialogBox( hIns,MAKEINTRESOURCE( IDD_SETTING ),hWnd,(DLGPROC)SetProc )==0 ){
		scrset = EdtMode;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g�̐ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrPalette( const char *szText,int *ptr )
{
	int p,r,g,b;
	// �p���b�g�ԍ����擾����
	if( !calc_exp( &p,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// �Ԃ̐ݒ�l���擾����
	if( !calc_exp( &r,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// �΂̐ݒ�l���擾����
	if( !calc_exp( &g,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// �Ԃ̐ݒ�l���擾����
	if( !calc_exp( &b,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �͈͊O
	if( p<0 || p>15 || r<0 || r>7 || g<0 || g>7 || b<0 || b>7 ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �p���b�g��ݒ肷��
	scrset.Col[p].red   = r;
	scrset.Col[p].green = g;
	scrset.Col[p].blue  = b;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g�̗L��������Ԃ̐ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrPalMode( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int p,b;
	// �p���b�g�ԍ����擾����
	if( !calc_exp( &p,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// �t���O�̐ݒ�l��ǂݍ���
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszPMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �t���O�l�̌���
	b = scrSearch( word,flgpmode,ELMCNT( flgpmode ) );
	if( b==SW_NONE ){
		MessageBox(  NULL,cszPMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �l�̑��
	scrset.PalEn[p] = b;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�؂�̂Č덷�l�̐ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrCutErr( const char *szText,int *ptr )
{
	int r;
	// �؂�̂Č덷�̒l���擾
	if( !calc_exp( &r,szText,ptr ) ){
		MessageBox( NULL,cszCutErr,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �͈͊O�`�F�b�N
	if( r<0 || r>255 ){
		MessageBox( NULL,cszCutErr,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	scrset.err = r;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�덷�g�U�W���ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrGosaVal( const char *szText,int *ptr )
{
	int r;
	// �؂�̂Č덷�̒l���擾
	if( !calc_exp( &r,szText,ptr ) ){
		MessageBox( NULL,cszGosaVal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �͈͊O�`�F�b�N
	if( r<0 || r>500 ){
		MessageBox( NULL,cszGosaVal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	scrset.diffusion_error_coef = (float)r/1000.0f;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�I�F���[�h�ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrPalSelMode( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int b;
	// �t���O�̐ݒ�l��ǂݍ���
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �t���O�l�̌���
	b = scrSearch( word,flgselmode,ELMCNT( flgselmode ) );
	if( b==SW_NONE ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �l�̑��
	scrset.SelCol = b;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�f�B�U���Z���@�ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrDthAdd( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int a;
	// �t���O�̐ݒ�l��ǂݍ���
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �t���O�l�̌���
	a = scrSearch( word,flgdthadd,ELMCNT( flgdthadd ) );
	if( a==SW_NONE ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �l�̑��
	scrset.ErrAdd = a;
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���R�搶���A���S���Y���ݒ薽��
//	2.	����
//		szText	...	(I)	�e�L�X�g
//		ptr		...	(I)	�e�L�X�g�̎Q�ƈʒu
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool scrAlgoMode( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int a;
	// �t���O�̐ݒ�l��ǂݍ���
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �t���O�l�̌���
	a = scrSearch( word,flgalgo,ELMCNT( flgalgo ) );
	if( a==SW_NONE ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// �l�̑��
	scrset.AlgoMode = a;
	return true;
}
