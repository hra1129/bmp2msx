// -------------------------------------------------------------
// BMP to MSX
// スクリプトウィンドウ
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

void Converter( SETTING *Mode,int nSrc );			// WndProc.cpp : 変換処理関数
#define OFFSET( t,m ) (int)( &( ((t*)NULL)->m ) )	// オフセットアドレスを返す

// -------------------------------------------------------------
// 設定

#define cnFontSize	16								// スクリプトのフォントサイズ
#define TXT_SIZE	30000							// スクリプトファイルの最大サイズ
#define WORD_MAX	32								// 単語最大長
#define MSG_MAX		1024							// メッセージボックス文字列長

#ifdef _ENGLISH
#define cszFontName	"MS Gothic"						// font of script text.
#else
#define cszFontName	"ＭＳ ゴシック"					// スクリプトのフォント名
#endif

typedef struct {
	char	*word;
	int		id;
} SCRWORD;

typedef bool (*SCRFUNC)( const char *szText,int *ptr );

enum {
	SW_NONE=-1,			// エラー
	SW_LOADSETTING=0,	// 設定読込
	SW_CONVERT,			// 変換
	SW_SETFILENAME,		// ファイル名設定
	SW_COMMENT,			// コメント
	SW_SCREEN,			// 出力画面モード
	SW_MESSAGE,			// メッセージ出力
	SW_SETFLAG,			// 各種フラグ設定
	SW_SETTING,			// 設定ダイアログ
	SW_PALETTE,			// パレットの設定
	SW_PALMODE,			// パレットのモードを設定
	SW_CUTERR,			// 切り捨て誤差
	SW_GOSAVAL,			// 誤差拡散係数
	SW_SELCOL,			// 選色モード
	SW_DTHADD,			// ディザ加算方法
	SW_ALGOMODE,		// 自然が生成アルゴリズム
};

// -------------------------------------------------------------
// プロトタイプ

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

// スクリプトコマンド関数
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

// 関数配列
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
// グローバル変数

extern HINSTANCE hIns;					// インスタンスハンドル
extern SETTING	NowMode;				// 現在の設定

static HICON	hIcon = NULL;			// ダイアログのアイコン
static HFONT	hFont = NULL;			// 等幅フォント
static HWND		hWnd  = NULL;			// ダイアログ
static HWND		hText = NULL;			// テキストボックス
static SETTING	scrset;					// 変換設定
static char szTarget[ MAX_PATH ];		// 変換対象のファイル名

// 命令語
static SCRWORD scrword[]={
	{"LOADSETTING"	, SW_LOADSETTING		},
	{"設定読込"		, SW_LOADSETTING		},
	{"CONVERT"		, SW_CONVERT			},
	{"変換処理"		, SW_CONVERT			},
	{"SETFILENAME"	, SW_SETFILENAME		},
	{"ファイル名"	, SW_SETFILENAME		},
	{";"			, SW_COMMENT			},
	{"SCREEN"		, SW_SCREEN			},
	{"画面モード"	, SW_SCREEN			},
	{"MSG"			, SW_MESSAGE			},
	{"メッセージ"	, SW_MESSAGE			},
	{"SETFLAG"		, SW_SETFLAG			},
	{"フラグ"		, SW_SETFLAG			},
	{"SETTING"		, SW_SETTING			},
	{"設定画面表示"	, SW_SETTING			},
	{"PALETTE"		, SW_PALETTE			},
	{"パレット"		, SW_PALETTE			},
	{"PALMODE"		, SW_PALMODE			},
	{"パレット動作"	, SW_PALMODE			},
	{"CUTERR"		, SW_CUTERR			},
	{"切り捨て誤差"	, SW_CUTERR			},
	{"GOSAVAL"		, SW_GOSAVAL			},
	{"誤差拡散係数"	, SW_GOSAVAL			},
	{"SELCOLMODE"	, SW_SELCOL			},
	{"選色モード"	, SW_SELCOL			},
	{"DITHERADD"	, SW_DTHADD			},
	{"ディザ加算"	, SW_DTHADD			},
	{"ALGOMODE"		, SW_ALGOMODE		},
	{"自然画生成"	, SW_ALGOMODE		},
};

// フラグ名
static SCRWORD flgword[]={
	{"誤差拡散"			,OFFSET( SETTING, diffusion_error_enable	)},		// 誤差拡散 する:true / しない:false
	{"GOSA"				,OFFSET( SETTING, diffusion_error_enable	)},		// 誤差拡散 する:true / しない:false
	{"インターレース"	,OFFSET( SETTING, interlace					)},		// インターレース する:true / しない:false
	{"INTERACE"			,OFFSET( SETTING, interlace					)},		// インターレース する:true / しない:false
	{"固定パレット"		,OFFSET( SETTING, fixed_palette				)},		// 固定パレット する:true / しない:false
	{"PALNOCHG"			,OFFSET( SETTING, fixed_palette				)},		// 固定パレット する:true / しない:false
	{"サイズ調節"		,OFFSET( SETTING, resize_enable					)},		// サイズ調節
	{"RESIZE"			,OFFSET( SETTING, resize_enable					)},		// サイズ調節
	{"自動ファイル名"	,OFFSET( SETTING, AutoName					)},		// 自動ファイル名決定
	{"AUTONAME"			,OFFSET( SETTING, AutoName					)},		// 自動ファイル名決定
	{"色差再計算"		,OFFSET( SETTING, JKrc						)},		// 色差情報の再計算
	{"COLORRECALC"		,OFFSET( SETTING, JKrc						)},		// 色差情報の再計算
	{"ゼロ使用不可"		,OFFSET( SETTING, NonZero					)},		// ０番の色を使わない
	{"ZERONOUSE"		,OFFSET( SETTING, NonZero					)},		// ０番の色を使わない
	{"タイル"			,OFFSET( SETTING, Tile						)},		// 網がけタイルを使う
	{"TILE"				,OFFSET( SETTING, Tile						)},		// 網がけタイルを使う
};

// フラグ値
static SCRWORD flgbool[]={
	{"TRUE"			,1	},
	{"FALSE"		,0	},
	{"ON"			,1	},
	{"OFF"			,0	},
};

// パレットモード
static SCRWORD flgpmode[]={
	{"NOUSE"		,PALEN_NONE },
	{"AUTO"			,PALEN_AUTO },
	{"CUSTOM"		,PALEN_USE  },
	{"未使用"		,PALEN_NONE },
	{"自動"			,PALEN_AUTO },
	{"指定"			,PALEN_USE  },
};

// 選色モード
static SCRWORD flgselmode[]={
	{"SCATTER"		,SELCOL_BUNSAN	},	// 分散選択
	{"MAX"			,SELCOL_TABUNPU },	// 多分布選択
	{"分散"			,SELCOL_BUNSAN	},	// 分散選択
	{"多分布"		,SELCOL_TABUNPU },	// 多分布選択
};

// ディザ加算モード
static SCRWORD flgdthadd[]={
	{"ROTATE"		,EADD_ROTATE	},	// 回転
	{"NONE"			,EADD_NONE		},	// 同位相
	{"X"			,EADD_X			},	// Ｘずらし
	{"Y"			,EADD_Y			},	// Ｙずらし
	{"回転"			,EADD_ROTATE	},	// 回転
	{"同位相"		,EADD_NONE		},	// 同位相
	{"Ｘずらし"		,EADD_X			},	// Ｘずらし
	{"Ｙずらし"		,EADD_Y			},	// Ｙずらし
};

// 自然画生成アルゴリズム
static SCRWORD flgalgo[]={
	{"無し"			,ALGO_NONE		},	// 無し
	{"再計算"		,ALGO_RECALC	},	// 逆算による誤差訂正
	{"パワフル"		,ALGO_POWERFUL	},	// パワフル活用法のレイトレーシング法に使われていた式
	{"ＮＴＳＣ改"	,ALGO_NTSC2		},	// NTSC方式改良版
	{"色合い"		,ALGO_RECALC2	},	//	逆算による誤差訂正（色合い重視）
	{"NONE"			,ALGO_NONE		},	// 無し
	{"RECALC"		,ALGO_RECALC	},	// 逆算による誤差訂正
	{"POWERFUL"		,ALGO_POWERFUL	},	// パワフル活用法のレイトレーシング法に使われていた式
	{"NTSC2"		,ALGO_NTSC2		},	// NTSC方式改良版
	{"COLOR"		,ALGO_RECALC2	},	//	逆算による誤差訂正（色合い重視）
};

// -------------------------------------------------------------
//	1.	日本語名
//		ウィンドウ関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理した
//		FALSE	...	未処理
//	4.	備考
//		なし
// -------------------------------------------------------------
WINPROC( ScriptProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE				, onScriptClose		);	// 閉じる
		ONEVENT( WM_COMMAND				, onScriptCommand	);	// コマンド分岐
		ONEVENT( WM_INITDIALOG			, onScriptInitDlg	);	// ダイアログ初期化
		ONEVENT( WM_PAINT				, onScriptPaint		);	// 再描画
		ONEVENT( WM_WINDOWPOSCHANGING	, onScriptWinChg	);	// ウィンドウサイズ変更直前
		ONEVENT( WM_DROPFILES			, onScriptDropFiles	);	// ファイルドロップ
	}
	return FALSE;
}

// =====================================================
// イベント

// -------------------------------------------------------------
//	1.	日本語名
//		ウィンドウ閉じる
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
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
//	1.	日本語名
//		コマンド
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理した
//		FALSE	...	未処理
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptCommand )
{
	switch( LOWORD( wp ) )
	{
	// メニューイベント
	ONEVENT( IDM_EXIT		, onScriptClose		);		// 閉じる
	ONEVENT( IDM_NEW		, onScriptNew		);		// 新規作成
	ONEVENT( IDM_OPEN		, onScriptOpen		);		// 開く
	ONEVENT( IDM_SAVE		, onScriptSave		);		// 保存
	ONEVENT( IDM_RUN		, onScriptRun		);		// 実行
	}
	return FALSE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ダイアログの初期化
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptInitDlg )
{
	::hWnd = hWnd;
	// アイコンをセットする
	hIcon = LoadIcon( hIns,MAKEINTRESOURCE( IDI_ICONSCRIPT ) );
	SendMessage( hWnd,WM_SETICON,ICON_BIG  ,(LPARAM)hIcon );
	SendMessage( hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon );
	// フォントをセットする
	hFont = CreateFont( cnFontSize,0,0,0,400,FALSE,FALSE,FALSE,SHIFTJIS_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
						FF_DONTCARE | FIXED_PITCH,cszFontName );
	hText = GetDlgItem( hWnd,IDC_SCRIPT );
	SendMessage( hText,WM_SETFONT,(WPARAM)hFont,TRUE );
	Edit_LimitText( hText,TXT_SIZE-1 );							// テキスト最大長
	return TRUE;	// 自動で最初のフォーカスを当ててもらう
}

// -------------------------------------------------------------
//	1.	日本語名
//		画面の描画
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
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
//	1.	日本語名
//		画面サイズ変更
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptWinChg )
{
	RECT r;
	// ウィンドウサイズを変更
	GetClientRect( hWnd,&r );
	SetWindowPos( hText,0,0,0,r.right,r.bottom,SWP_NOZORDER );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		新規作成
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptNew )
{
	int i;
	i = Edit_GetTextLength( hText );
	// 確認メッセージ
	if( i ){
		if( MessageBox( hWnd,cszWrnClear,MsgCapWrn,MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )!=IDYES )
			return TRUE;
	}
	// クリア
	SetWindowText( hText,"" );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイルを開く
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptOpen )
{
	char			szFileName[ MAX_PATH ];
	char			szBuf[ TXT_SIZE ],c;
	FU_FILE			*hFile;
	int				i = 0;
	int				tmp;
	OPENFILENAME	ofn;

	// ﾌｧｲﾙ名を入力
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
	ofn.lpstrInitialDir	= NULL;	// カレント
	ofn.lpstrTitle		= ScrptLoad;
	if( !GetOpenFileName( &ofn ) ) return TRUE;
	// ﾌｧｲﾙを読み込む
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
	// バイナリデータを強引に読み出した場合にはエラーではじく
	while( (c=szBuf[i++])!=0 ){
		if( c=='\t' || c=='\n' || c=='\r' ) continue;
		if( c<32 && c>0 ){
			MessageBox( hWnd,cszNoText,MsgCap,MB_OK | MB_ICONWARNING );
			CloseHandle( hFile );
			return TRUE;
		}
	}
	fu_close( hFile );
	// 読み込んだテキストを設定する
	SetWindowText( hText,szBuf );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		保存
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptSave ) {
	char	szFileName[ MAX_PATH ];
	char	szBuf[ TXT_SIZE ];
	FU_FILE	*hFile;

	// 中身が無ければ何もしない
	if( Edit_GetTextLength( hText )==0 ) return TRUE;
	// ファイル名を取得する
	if( !GetName( hWnd, szFileName, MAX_PATH, NULL, cszDefScrpt, NULL ) ) return TRUE;
	// 保存する
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
//	1.	日本語名
//		ファイルドロップ
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptDropFiles )
{
	HDROP	hDrop;
	char	szBuf[ MAX_PATH ];
	char	szBuf2[ MAX_PATH ];
	int		i,j;
	hDrop = (HDROP)wp;
	char szText[ TXT_SIZE ];

	// テキストを取得する
	GetWindowText( hText,szText,TXT_SIZE );
	// ファイル数を取得
	j=DragQueryFile( hDrop,0xFFFFFFFFL,szBuf,MAX_PATH );
	// 経過を表示
	GetFileTitle( szBuf, szBuf2, sizeof( szBuf2 ) );
	prShow( szBuf2 );
	// 全ファイルを処理する
	for( i=0;i<j;i++ ){
		// ドロップされたファイル名を取得する
		DragQueryFile( hDrop,i,szBuf,MAX_PATH );
		// 経過ダイアログの表示更新
		prSetText( szBuf );
		// テキストを処理する
		scrPlayScript( szText,szBuf );
	}
	// 経過を削除
	prDestroy();
	// 終了
	DragFinish( hDrop );
	MessageBox( NULL,cszExit,MsgCap,MB_OK | MB_ICONWARNING );
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		実行
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onScriptRun )
{
	char szText[ TXT_SIZE ];
	// テキストを取得する
	GetWindowText( hText,szText,TXT_SIZE );
	// テキストを処理する
	scrPlayScript( szText,"" );
	MessageBox( NULL,cszExit,MsgCap,MB_OK | MB_ICONWARNING );
	return TRUE;
}

// =====================================================
// スクリプト処理

// -------------------------------------------------------------
//	1.	日本語名
//		スクリプト処理
//	2.	引数
//		szText		...	(O)	スクリプト格納先メモリ
//		szFileName	...	(I)	画像ファイル名
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void scrPlayScript( char *szText,char *szFileName )
{
	char word[ WORD_MAX ];
	int ptr=0;
	int cmd;
	// 設定を初期化する
	scrset = NowMode;
	lstrcpy( szTarget,szFileName );
	// 処理ループ
	while( szText[ptr]!=0 ){
		// 改行/ホワイトスペースを読み飛ばす
		scrSkipSpace( szText,&ptr );
		// 単語を取得
		scrGetWord( szText,&ptr,word,WORD_MAX );
		if( word[0]==0 ) break;								// すべて実行
		// 単語コードを検索
		cmd = scrSearch( word,scrword,ELMCNT( scrword ) );
		if( cmd != SW_NONE ){
			if( !(scrfunc[cmd])( szText,&ptr ) ) break;		// コマンド関数を呼びエラー発生時は終了
		}else{
			MessageBox( NULL,cszSyntax,MsgCap,MB_OK | MB_ICONWARNING );
			break;											// 不正な文法
		}
		if( szText[ptr]==0 ) break;
	}
	// 実行を終えた位置にカーソルを移動する
	Edit_SetSel( hText,ptr,ptr );
}

// -------------------------------------------------------------
//	1.	日本語名
//		ホワイトスペースなら true,それ以外なら false を返す
//	2.	引数
//		c	...	(I)	判定したい文字（１バイト）
//	3.	返値
//		true	...	スペース
//		false	...	非スペース
//	4.	備考
//		VC++6.0SP3 の isspace() は -21(0xEB)を与えると 8 が返ってくる
//		ため日本語混在だと使えない
// -------------------------------------------------------------
static inline bool scrIsSpace( char c )
{
	return( c==' ' || c=='\t' || c=='\xA' || c=='\xD' );
}

// -------------------------------------------------------------
//	1.	日本語名
//		ホワイトスペースを読み飛ばす
//	2.	引数
//		szText	...	(I)		テキスト
//		ptr		...	(I/O)	テキストの参照位置
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void scrSkipSpace( const char *szText,int *ptr )
{
	while( scrIsSpace( szText[ *ptr ] ) ) ++(*ptr);
}

// -------------------------------------------------------------
//	1.	日本語名
//		単語の取得
//	2.	引数
//		szText	...	(I)		テキスト
//		ptr		...	(I/O)	テキストの参照位置
//		word	...	(O)		単語の出力先
//		WordMax	...	(I)		word のサイズ
//	3.	返値
//		なし
//	4.	備考
//		なし
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
//	1.	日本語名
//		スクリプト予約語の検索
//	2.	引数
//		word	...	(I)	単語
//		sword	...	(I)	予約語配列
//		n		...	(I)	sword の要素数
//	3.	返値
//		一致すれば、sword のインデックス
//		一致しなければ SW_NONE
//	4.	備考
//		なし
// -------------------------------------------------------------
static int scrSearch( const char *word,SCRWORD *sword,int n )
{
	int i;
	// 検索
	for( i=0; i<n; ++i ){
		if( !lstrcmpi( word,sword[i].word ) ) return sword[i].id;
	}
	return SW_NONE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		文字列の取得
//	2.	引数
//		szText	...	(I)		テキスト
//		ptr		...	(I/O)	テキストの参照位置
//		word	...	(O)		文字列の出力先
//		WordMax	...	(I)		word のサイズ
//	3.	返値
//		true	...	正常
//		false	...	異常
//	4.	備考
//		なし
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
//	1.	日本語名
//		数値の取得
//	2.	引数
//		szText	...	(I)		テキスト
//		ptr		...	(I/O)	テキストの参照位置
//		num		...	(I)		数値の格納先
//	3.	返値
//		true	...	正常
//		false	...	異常
//	4.	備考
//		なし
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
// スクリプトコマンド

// -------------------------------------------------------------
//	1.	日本語名
//		設定を読み込む
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrLoadSetting( const char *szText,int *ptr )
{
	char filename[ WORD_MAX ];
	// 設定ファイル名を取得する
	if( !scrGetString( szText,ptr,filename,WORD_MAX ) ) return false;
	// 設定を読み込む
	if( !GetCfgFile( &scrset,filename ) ){
		MessageBox( NULL,cszNoCfg,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		変換処理を実行する
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrConvert( const char *szText,int *ptr )
{
	int n;
	// ファイル名が設定されていなければ終了する
	if( szTarget[0]==0 ){
		MessageBox( NULL,cszNoFile,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// ダイアログボックスを表示
	prShow( szTarget );
	// 変換
	if( (n=bdAddFile( szTarget ))==-1 ){
		MessageBox( NULL,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
		prDestroy();
		return 0;
	}
	Converter( &scrset,n );
	// 廃棄
	if( scrset.PreView == 1 ){
		bdDelete(n);
	}
	prDestroy();
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイル名を設定する
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrSetFileName( const char *szText,int *ptr )
{
	// 設定ファイル名を取得する
	if( !scrGetString( szText,ptr,szTarget,WORD_MAX ) ) return false;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		コメント処理
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrComment( const char *szText,int *ptr )
{
	// コメントが終わる（行末か文末）までポインタを移動する
	while( szText[ *ptr ]!='\xA' && szText[ *ptr ]!='\xD' && szText[ *ptr ]!=0 ) ++(*ptr);
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		出力モードを変更する
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrScreen( const char *szText,int *ptr )
{
	int m;
	// 数値を入力
	if( !calc_exp( &m,szText,ptr ) ){
		MessageBox( NULL,cszScrMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 判定
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
//	1.	日本語名
//		メッセージボックスを表示する
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrMessage( const char *szText,int *ptr )
{
	char word[ MSG_MAX ];
	// 設定ファイル名を取得する
	if( !scrGetString( szText,ptr,word,MSG_MAX ) ) return false;
#ifdef _ENGLISH
	MessageBox( NULL,word,"Message",MB_OK );
#else
	MessageBox( NULL,word,"ﾒｯｾｰｼﾞ",MB_OK );
#endif
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		各種ﾌﾗｸﾞの設定
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrSetFlag( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int p,b;
	// 単語の取得
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszNoFlag,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 単語の検索
	p = scrSearch( word,flgword,ELMCNT( flgword ) );
	if( p==SW_NONE || szText[ *ptr ]==0 ){
		MessageBox(  NULL,cszNoFlag,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// '=' 等の記号を読み飛ばす
	scrSkipSpace( szText,ptr );
	++(*ptr);
	scrSkipSpace( szText,ptr );
	// フラグの設定値を読み込む
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszNoBool,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// フラグ値の検索
	b = scrSearch( word,flgbool,ELMCNT( flgbool ) );
	if( b==SW_NONE ){
		MessageBox(  NULL,cszNoBool,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 値の代入
	*((bool*)(((char*)&scrset)+p)) = (b!=0);
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		変換設定変更ダイアログの表示命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrSetting( const char *szText,int *ptr )
{
	// モード設定を編集エリアへコピーする
	EdtMode = scrset;
	lstrcpy( szCaption,cszScrSetting );
	if( DialogBox( hIns,MAKEINTRESOURCE( IDD_SETTING ),hWnd,(DLGPROC)SetProc )==0 ){
		scrset = EdtMode;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットの設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrPalette( const char *szText,int *ptr )
{
	int p,r,g,b;
	// パレット番号を取得する
	if( !calc_exp( &p,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// 赤の設定値を取得する
	if( !calc_exp( &r,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// 緑の設定値を取得する
	if( !calc_exp( &g,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// 赤の設定値を取得する
	if( !calc_exp( &b,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 範囲外
	if( p<0 || p>15 || r<0 || r>7 || g<0 || g>7 || b<0 || b>7 ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// パレットを設定する
	scrset.Col[p].red   = r;
	scrset.Col[p].green = g;
	scrset.Col[p].blue  = b;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットの有効無効状態の設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrPalMode( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int p,b;
	// パレット番号を取得する
	if( !calc_exp( &p,szText,ptr ) ){
		MessageBox( NULL,cszPal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	++(*ptr);
	// フラグの設定値を読み込む
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszPMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// フラグ値の検索
	b = scrSearch( word,flgpmode,ELMCNT( flgpmode ) );
	if( b==SW_NONE ){
		MessageBox(  NULL,cszPMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 値の代入
	scrset.PalEn[p] = b;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		切り捨て誤差値の設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrCutErr( const char *szText,int *ptr )
{
	int r;
	// 切り捨て誤差の値を取得
	if( !calc_exp( &r,szText,ptr ) ){
		MessageBox( NULL,cszCutErr,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 範囲外チェック
	if( r<0 || r>255 ){
		MessageBox( NULL,cszCutErr,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	scrset.err = r;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		誤差拡散係数設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrGosaVal( const char *szText,int *ptr )
{
	int r;
	// 切り捨て誤差の値を取得
	if( !calc_exp( &r,szText,ptr ) ){
		MessageBox( NULL,cszGosaVal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 範囲外チェック
	if( r<0 || r>500 ){
		MessageBox( NULL,cszGosaVal,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	scrset.diffusion_error_coef = (float)r/1000.0f;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		選色モード設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrPalSelMode( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int b;
	// フラグの設定値を読み込む
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// フラグ値の検索
	b = scrSearch( word,flgselmode,ELMCNT( flgselmode ) );
	if( b==SW_NONE ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 値の代入
	scrset.SelCol = b;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ディザ加算方法設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrDthAdd( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int a;
	// フラグの設定値を読み込む
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// フラグ値の検索
	a = scrSearch( word,flgdthadd,ELMCNT( flgdthadd ) );
	if( a==SW_NONE ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 値の代入
	scrset.ErrAdd = a;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		自然画生成アルゴリズム設定命令
//	2.	引数
//		szText	...	(I)	テキスト
//		ptr		...	(I)	テキストの参照位置
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool scrAlgoMode( const char *szText,int *ptr )
{
	char word[ WORD_MAX ];
	int a;
	// フラグの設定値を読み込む
	scrGetWord( szText,ptr,word,WORD_MAX );
	if( word[0]==0 ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// フラグ値の検索
	a = scrSearch( word,flgalgo,ELMCNT( flgalgo ) );
	if( a==SW_NONE ){
		MessageBox(  NULL,cszSelMode,MsgCap,MB_OK | MB_ICONWARNING );
		return false;
	}
	// 値の代入
	scrset.AlgoMode = a;
	return true;
}
