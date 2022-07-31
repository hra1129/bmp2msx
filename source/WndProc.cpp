// -----------------------------------------------------
// BMP to MSX
// メインウィンドウ
//									(C)2000/5/6 HRA!
// -----------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <htmlhelp.h>		//	HTMLhelp workshop に付属
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
// 設定

static const int mode[] = { 2, 3, 5, 6, 7, 8, 10, 12, 5, 6, 7, 8, 10, 12 };	// 画面モード番号

// 画面サイズ
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

// フラグ
bool bSave	= false;	// 終了時に設定を保存するか否か
bool bSrc	= false;	// プレビュー表示はソースか否（ディスティネーション）か
bool bData	= true;		// 設定情報を表示するか否か

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

#define cszFontName	"ＭＳ ゴシック"					// フォント名
const char *cSetting  = "設定";
const char *cOriginal = "変換元";
const char *cResult	  = "変換結果";
const char *cClipboard= "ｸﾘｯﾌﾟﾎﾞｰﾄﾞ画像";
const char *cUse	  = "使用";
const char *cNoUse	  = "未使用";
const char *cNonZero  = "0番使用禁止";
const char *cUseZero  = "0番も使用する";

const char *cShowSet	  = "[%s]\n"
						"画面ﾓｰﾄﾞ = %s\n"
						"出力先 = %s\n"
						"誤差拡散 = %s : 係数 = 0.%03d : 捨誤差=%d\n"
						"全固定ﾊﾟﾚｯﾄ = %s : 0番の色 = %s\n"
						"JK再計算 = %s\n"
						"ｲﾝﾀｰﾚｰｽ = %s\n"
						"ﾃﾞｨｻﾞ = %s : ﾃﾞｨｻﾞ合成方法 = %s\n"
						"色再現精度 = %s\n"
						"自然画ｱﾙｺﾞﾘｽﾞﾑ = %s\n"
						"自動ﾌｧｲﾙ名 = %s\n"
						"ﾊﾟﾚｯﾄ出力方法 = %s\n"
						"50%%:50%%網ﾀｲﾙ = %s\n";

#endif

// -----------------------------------------------------
// プロトタイプ

// 一般イベント
EVENT( onCreate				);
EVENT( onClose				);
EVENT( onCommand			);
EVENT( onDestroy			);
EVENT( onDropFiles			);
EVENT( onLButton			);
EVENT( onRButton			);
EVENT( onPaint				);
EVENT( onInitMenu			);
// ファイルメニューイベント
EVENT( onOpen				);
EVENT( onScript				);
// 設定メニューイベント
EVENT( onDefault			);
EVENT( onSetting			);
EVENT( onPlugIn				);
EVENT( onColorCalibration	);

// ヘルプメニューイベント
EVENT( onHelp				);
EVENT( onVer				);
// 編集メニューイベント
EVENT( onPaste				);
// プレビューメニューイベント
EVENT( onMenuDelete			);
EVENT( onMenuAllDelete		);
EVENT( onMenuSave			);
EVENT( onMenuSaveAs			);
EVENT( onMenuBmpSave		);
EVENT( onMenuRedo			);
EVENT( onMenuData			);
EVENT( onMenuZoomUp			);
EVENT( onMenuZoomDown		);
// プレビュー - 表示 メニューイベント
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
// グローバル変数

static HDC		hMemDC = NULL;			// 仮想画面
static HBITMAP	hMemBM = NULL;			// 仮想画面
static HDC		hBGDC  = NULL;			// ロゴ画面
static HBITMAP	hBGBM  = NULL;			// ロゴ画面
static HWND		hWnd   = NULL;			// ウィンドウハンドル
SETTING NowMode;						// 現在の設定
static SETTING	LastMode;				// 最終変換設定
extern int	nZoom;						// 表示倍率

extern bool	bRedo;						// 再変換かﾃﾞﾌｫﾙﾄ設定か

// =====================================================

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		メッセージ処理関数に依存
//	4.	備考
//		なし
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
// 一般イベント

// -----------------------------------------------------
//	1.	日本語名
//		コマンド分岐
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		コマンド処理関数に依存
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onCommand )
{
	switch( LOWORD( wp ) )
	{
	// メニュー
	ONEVENT( IDM_EXIT				,onClose			);	// 終了
	ONEVENT( IDM_OPEN				,onOpen				);	// ファイルを開く
	ONEVENT( IDM_SCRIPT				,onScript			);	// スクリプトモード
	// 編集メニュー
	ONEVENT( IDM_PASTE				,onPaste			);	// ペースト
	// プレビューメニュー
	ONEVENT( IDM_NEXT				,onMenuNext			);	// 次
	ONEVENT( IDM_PREV				,onMenuPrev			);	// 前
	ONEVENT( IDM_CLEAR				,onMenuDelete		);	// 削除
	ONEVENT( IDM_ALLCLEAR			,onMenuAllDelete	);	// 全削除
	ONEVENT( IDM_SAVE				,onMenuSave			);	// 保存
	ONEVENT( IDM_SAVEAS				,onMenuSaveAs		);	// 名前を付けて保存
	ONEVENT( IDM_BMPSAVE			,onMenuBmpSave		);	// ＢＭＰで保存
	ONEVENT( IDM_REDO				,onMenuRedo			);	// 再変換
	ONEVENT( IDM_CHANGE				,onMenuChange		);	// 変換元/変換結果切り替え
	ONEVENT( IDM_DATA				,onMenuData			);	// 設定の表示切り替え
	ONEVENT( IDM_ORIGNAL			,onMenuOriginal		);	// オリジナル画像イメージ
	ONEVENT( IDM_ZOOMUP				,onMenuZoomUp		);	// 拡大
	ONEVENT( IDM_ZOOMDOWN			,onMenuZoomDown		);	// 縮小
	// 設定メニュー
	ONEVENT( IDM_DEFSET				,onDefault			);	// 設定の初期化
	ONEVENT( IDM_SETTING			,onSetting			);	// 設定
	ONEVENT( IDM_PLUGIN				,onPlugIn			);	// プラグイン設定
	ONEVENT( IDM_COLOR_CALIBRATION, onColorCalibration	);
	// ヘルプメニュー
	ONEVENT( IDM_HELP				,onHelp				);	// ヘルプの表示
	ONEVENT( IDM_VER				,onVer				);	// バージョン情報
	}
	return DEFPROC();
}

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ生成時
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		-1	...	ウィンドウ生成失敗
//		0	...	なし
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onCreate )
{
	HDC		hDC;
	char	szFileName[ MAX_PATH ] = "";
	char	szBuf[ MAX_PATH ];
	int		i,j,n;
	// プレビューのクリア
	pvInitialize();
	// ウィンドウハンドル
	::hWnd = hWnd;
	// モード設定をクリアする
	bSave = !GetCfgFile( &NowMode, CfgFile );
	bSave = !GetPathFile( PathFile, g_sPlug, sizeof( g_sPlug ) ) || bSave;
	bSave = !GetCustomFile( CustomFile ) || bSave;
	spi_initialize( g_sPlug );
	LastMode.Mode = -1;
	// 仮想画面を作成する
	hDC = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hDC );
	hMemBM = CreateCompatibleBitmap( hDC, cnWidth, cnHeight );
	hBGDC  = CreateCompatibleDC( hDC );
	hBGBM  = LoadBitmap( hIns,MAKEINTRESOURCE( IDB_LOGO ) );
	ReleaseDC( hWnd,hDC );
	if( hMemDC==NULL || hMemBM==NULL || hBGDC ==NULL || hBGBM ==NULL ) return -1;
	SelectBitmap( hMemDC,hMemBM );
	SelectBitmap( hBGDC ,hBGBM  );
	// 画面の更新
	UpdateVScreen();
	// コマンドライン
	if( lstrlen( cmd ) ){
		// 文字列から " を取り除く
		i=j=0;
		while( cmd[i]!='\0' && j<MAX_PATH-1 ){
			if( cmd[i]!='"' ){
				szFileName[j]=cmd[i];
				j++;
			}
			i++;
		}
		szFileName[j]='\0';
		// ダイアログボックスを表示
		GetFileTitle( szFileName, szBuf, sizeof( szBuf ) );
		prShow( szFileName );
		// 変換
		if( (n=bdAddFile( szFileName ))==-1 ){
			MessageBox( hWnd,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
			prDestroy();
			return 0;
		}
		Converter( &NowMode,n );
		// 廃棄
		if( NowMode.PreView == 1 ){
			bdDelete(n);
		}
		prDestroy();
	}
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ閉じる
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onClose )
{
	DestroyWindow( hWnd );
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ廃棄時
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		ファイルドロップ
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onDropFiles )
{
	HDROP	hDrop;
	char	szBuf[ MAX_PATH ];
	int		i,j,n;
	hDrop = (HDROP)wp;
	SETTING	tmp;

	// ファイル数を取得
	j = DragQueryFile( hDrop, 0xFFFFFFFFL, szBuf, MAX_PATH );
	//	経過を表示
	prShow( "" );
	// 全ファイルを処理する
	for( i = 0; i < j; i++ ){
		// ドロップされたファイル名を取得する
		DragQueryFile( hDrop, i, szBuf, MAX_PATH );
		// 経過にファイル名を表示
		prSetText( szBuf );
		// ファイルを変換する
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
	// 終了
	prDestroy();
	DragFinish( hDrop );
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		マウスクリック
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onLButton )
{
	if( pvIndex==-1 ){
		MessageBox( hWnd, cszMainClk, MsgCapWrn, MB_OK | MB_ICONWARNING );
		return 0;
	}else{
		return CALLEVENT( onMenuRedo );	// 再変換
	}
}

// -----------------------------------------------------
//	1.	日本語名
//		マウス右クリック
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		画面の描画
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	int			z;
	// 更新要求のある部分のみ更新する
	hDC = BeginPaint( hWnd,&ps );
	z = nZoom + 1;
	StretchBlt( hDC, 0, 0, cnWidth * z, cnHeight * z,
			 hMemDC, 0, 0, cnWidth    , cnHeight    , SRCCOPY );
	EndPaint( hWnd,&ps );
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		メニューの初期化
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onInitMenu ) {
	HANDLE			hMem;
	MENUITEMINFO	mi;
	const int pvitem[]={
		IDM_NEXT,IDM_PREV,IDM_CHANGE,IDM_ORIGNAL,IDM_CLEAR,
		IDM_ALLCLEAR,IDM_SAVE,IDM_SAVEAS,IDM_REDO,IDM_BMPSAVE,
	};
	int i;

	// メニューアイテムの初期化
	memset( &mi,0,sizeof( mi ) );
	mi.cbSize	= sizeof( mi );
	mi.fMask	= MIIM_STATE;
	// クリップボードに画像があるか確認する
	OpenClipboard(hWnd);
	hMem=GetClipboardData( CF_DIB );
	CloseClipboard();
	// 編集のペースト
	mi.fState	= hMem==NULL ? MFS_GRAYED : MFS_ENABLED;
	SetMenuItemInfo( hMenu,IDM_PASTE,FALSE,&mi );
	// プレビュー
	mi.fState	= pvIndex==-1 ? MFS_GRAYED : MFS_ENABLED;
	for( i=0; i<ELMCNT( pvitem ); ++i ){
		SetMenuItemInfo( hMenu,pvitem[i],FALSE,&mi );
	}
	return 0;
}

// =====================================================
// [ファイル]メニューイベント

// -----------------------------------------------------
//	1.	日本語名
//		ファイルを開く
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onOpen )
{
	char	szFileName[ MAX_PATH ] = "";
	char	szBuf[ MAX_PATH ];
#ifdef _ENGLISH
	char	szFilter[ 64 + SPI_MAXLEN ]	= "All Files(*.*)\0*.*\0BMP file(*.BMP)\0*.BMP\0";
	const	int	nextidx = sizeof( "All Files(*.*)\0*.*\0BMP file(*.BMP)\0*.BMP\0" );
#else
	char	szFilter[ 64 + SPI_MAXLEN ]	= "すべてのﾌｧｲﾙ(*.*)\0*.*\0BMP ﾌｧｲﾙ(*.BMP)\0*.BMP\0";
	const	int	nextidx = sizeof( "すべてのﾌｧｲﾙ(*.*)\0*.*\0BMP ﾌｧｲﾙ(*.BMP)\0*.BMP\0" );
#endif
	OPENFILENAME	ofn;
	int n;
	SETTING	tmp;

	//	フィルタの作成
	memcpy( &szFilter[ nextidx - 1 ], spi_get_filter(), SPI_MAXLEN );
	
	// ファイル名を入力
	szFileName[0]='\0';
	ZeroMemory( &ofn,sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= szFilter;
#ifdef _ENGLISH
	ofn.lpstrTitle		= "Open File";
#else
	ofn.lpstrTitle		= "ファイルを開く";
#endif
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// カレント
	if( !GetOpenFileName( &ofn ) ) return 0;
	// ファイル読み込み
	if( (n=bdAddFile( szFileName ))==-1 ){
		MessageBox( hWnd,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
		return 0;
	}
	// ダイアログボックスを表示
	GetFileTitle( szFileName, szBuf, sizeof( szBuf ) );
	prShow( szBuf );
	// 変換
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
//	1.	日本語名
//		スクリプトモードウィンドウを表示する
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onScript )
{
	DialogBox( hIns,MAKEINTRESOURCE( IDD_SCRIPT ),hWnd,(DLGPROC)ScriptProc );
	return 0;
}

// =====================================================
// [プレビュー]メニューイベント

// -----------------------------------------------------
//	1.	日本語名
//		設定データの表示を切り替える
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuData )
{
	bData = !bData;
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		変換元/変換結果の表示を切り替える
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuChange )
{
	bSrc = !bSrc;
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		次のプレビュー
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuNext )
{
	pvIndex = pvNextPreview( pvIndex );
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		前のプレビュー
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuPrev )
{
	pvIndex = pvPrevPreview( pvIndex );
	UpdateVScreen();
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		プレビューの削除
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		プレビューの全削除
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		プレビュー画像の保存
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		プレビュー画像に名前を付けて保存
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		ＢＭＰ保存
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuBmpSave )
{
	SETTING	tMode;
	char	sFileName[ MAX_PATH ];
	
	if( pvIndex < 0 ) return 0;
	tMode = tPreview[pvIndex].tMode;

	// ファイル名
	if( !GetName( hWnd,sFileName, sizeof( sFileName ), cszBmpSave, cszBmpFilter, CfgBmp ) ) return 0;

	//	保存
	if( !cnvSaveBmpFile( sFileName, tPreview[ pvIndex ].bmp, cnWidth, cnHeight, &tMode ) ){
		MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
		return 0;
	}

	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		拡大
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuZoomUp )
{
	RECT	r = { 0 };
	int		z;

	//	拡大
	nZoom = ( nZoom + 1 ) % cnZoomMax;
	z = nZoom + 1;
	//	ウィンドウの調節
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
//	1.	日本語名
//		縮小
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuZoomDown )
{
	RECT	r = { 0 };
	int		z;

	//	拡大
	nZoom = ( nZoom + cnZoomMax - 1 ) % cnZoomMax;
	z = nZoom + 1;
	//	ウィンドウの調節
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
//	1.	日本語名
//		再変換
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuRedo )
{
	SETTING Mode;
	char	szBuf[ MAX_PATH ];
	int nSrc;
	if( pvIndex < 0 ) return 0;
	// 設定を行う
	EdtMode = tPreview[ pvIndex ].tMode;
	LastMode = EdtMode;
	lstrcpy( szCaption,cszRedo );
	bRedo = true;
	if( DialogBox( hIns,MAKEINTRESOURCE( IDD_SETTING ),hWnd,(DLGPROC)SetProc )==0 ){
		// 処理を行う
		Mode = EdtMode;
		nSrc = tPreview[ pvIndex ].nSrc;
		// ダイアログボックスを表示
		if( strcmp( cszClipName, tBmpview[ tPreview[ pvIndex ].nSrc ].sFileName ) ) {
			GetFileTitle( tBmpview[ tPreview[ pvIndex ].nSrc ].sFileName, szBuf, sizeof( szBuf ) );
			prShow( szBuf );
		} else {
			prShow( cszClipName );
		}
		// 変換
		if( Mode.PreView & 2 ) ++tBmpview[nSrc].n;
		Converter( &Mode,nSrc );
		// ダイアログを閉じる
		prDestroy();
	}
	bRedo = false;
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		オリジナル画像の表示
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onMenuOriginal )
{
	if( pvIndex == -1 ) return 0;	// プレビューが無い場合はオリジナルを表示できない
	swShow( tBmpview[ tPreview[ pvIndex ].nSrc ].image );
	return 0;
}

// =====================================================
// [編集]メニューイベント

// -----------------------------------------------------
//	1.	日本語名
//		ｸﾘｯﾌﾟﾎﾞｰﾄﾞから張り付け
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPaste	)
{
	HGLOBAL	hMem;
	DWORD	size;
	char *bmp,*mem;
	SETTING	tmp;
	int n;
	// ｸﾘｯﾌﾟﾎﾞｰﾄﾞを開く
	OpenClipboard(hWnd);
	// ビットマップを取得する
	hMem=GetClipboardData( CF_DIB );
	if( hMem==NULL ){
		CloseClipboard();
		return 0;
	}
	size = GlobalSize( hMem );
	// メモリを確保
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
	// ｸﾘｯﾌﾟﾎﾞｰﾄﾞを閉じる
	CloseClipboard();
	// 画像ファイルイメージを記憶する
	if( (n=bdAddImage( bmp )) == -1 ){
		MessageBox( hWnd,cszBDErr,MsgCap,MB_OK | MB_ICONWARNING );
		LocalFree( bmp );
		return 0;
	}
	// ダイアログボックスを表示
	prShow( cszClipName );
	// 変換する
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
	tmp.AutoName = false;	// ファイル名自動生成は強制的に解除
	Converter( &tmp,n );
	// 後始末
	if( tmp.PreView == 1 ){
		bdDelete(n);
	}
	prDestroy();
	return 0;
}

// =====================================================
// [設定]メニューイベント

// -----------------------------------------------------
//	1.	日本語名
// 		デフォルトの設定に戻す
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onDefault ) {
#ifdef _ENGLISH
	if( MessageBox( hWnd,"Load default setting?","Warning",
					MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )==IDNO ) return 0;
#else
	if( MessageBox( hWnd,"デフォルトの設定に戻しますか？","確認",
					MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 )==IDNO ) return 0;
#endif
	GetDefCfg(&NowMode);
	bSave = true;
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		メニュー：セッティング
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onSetting ) {
	// モード設定を編集エリアへコピーする
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
//	1.	日本語名
//		メニュー：プラグイン設定
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugIn ) {
	if( piShow( hWnd, g_sPlug, sizeof( g_sPlug ) ) == 0 ) {
		//	プラグインリストを更新する
		spi_initialize( g_sPlug );
		bSave = true;
	}
	return 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		メニュー：カラーキャリブレーション
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onColorCalibration )	{

	if( DialogBox( hIns, MAKEINTRESOURCE( IDD_CALIBRATION ), hWnd, (DLGPROC)CalibrationProc ) == 0 ){
		bSave = true;
	}
	return 0;
}

// =====================================================
// [ヘルプ]メニューイベント

// -----------------------------------------------------
//	1.	日本語名
//		メニュー：ヘルプ
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
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
//	1.	日本語名
//		メニュー：バージョン情報
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onVer )
{
	DialogBox( hIns,MAKEINTRESOURCE( IDD_VER ),hWnd,(DLGPROC)VerProc );
	return 0;
}

// =====================================================
// ユーティリティ

// -----------------------------------------------------
//	1.	日本語名
//		ファイルを変換する
//	2.	引数
//		Mode	...	(I)	設定
//		nSrc	...	(I)	画像ファイルイメージのインデックス
//	3.	返値
//		なし
//	4.	備考
//		なし
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

	// プレビューを作成する
	if( Mode->PreView & 2 ){
		prv=pvBlankIndex();
		if( !pvAddPreview( prv ) ){
			MessageBox( hWnd, cszCannotAddPV, MsgCapWrn, MB_OK | MB_ICONWARNING );
			return;
		}
	}
	// メモリを読み込む
	if( !GetBmp( &in,&width,&height,&ptr ) ){
		MessageBox( hWnd, cszNoSupp, MsgCap, MB_OK | MB_ICONWARNING );
		return;
	}
	// 小さすぎる場合はエラー
	if( (width < 8 || height < 8) && Mode->Resize ){
		LocalFree( in );
		MessageBox( hWnd, cszSmall, MsgCap, MB_OK | MB_ICONWARNING );
		return;
	}
	// 強制ゼロ化の色を取得
	if( Mode->FourceZero == FZ_MATRIX ){
		Mode->FZColor = GetPix( in, width, height, Mode->FZX,Mode->FZY );
	}
	// 変換後のサイズを求め、メモリを確保する
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
	// 背景色でクリアする
	for( i = 0; i < owidth * oheight; ++i ) {
		out[ i ] = Mode->FCColor;
	}
	// サイズを調節する
	if( Mode->Resize ) {
		//	アスペクト比
		switch( Mode->SizeMode ) {
		case RM_MSX:
			if( (Mode->Mode == MD_SC6) || (Mode->Mode == MD_SC7) || (Mode->Mode == MD_SC6_256L) || (Mode->Mode == MD_SC7_256L) ) {
				wwidth  = width  * 5 / 3;	//	横に 1.66倍
				wheight = height;
			} else {
				wwidth  = width;
				wheight = height * 6 / 5;	//	縦に 1.2倍
			}
			if( Mode->Inter ) {
				wheight = wheight * 2;
			}
			//	幅を調節
			if( wwidth > owidth ) {
				wheight = wheight * owidth / wwidth;
				wwidth  = owidth;
			}
			//	高さを調節
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
		//	リサンプル
		switch( Mode->Resample ) {
		case RS_NORMAL:		//	ノーマル拡大縮小
			cnvNormResize( in, width, height, out, owidth, oheight, wwidth, wheight, prProg,Mode->Seido );
			break;
		case RS_ANTIALIAS:	//	アンチエイリアス付き拡大縮小
			cnvAntiResize( in, width, height, out, owidth, oheight, wwidth, wheight, prProg,Mode->Seido );
			break;
		default:			//	サイズ調節無し
			if( Mode->Inter && Mode->Mode != MD_SC6 && Mode->Mode != MD_SC7 && Mode->Mode != MD_SC6_256L && Mode->Mode != MD_SC7_256L ) {
				cnvCopyInter( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
			}else{
				cnvCopy( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
			}
		}
	} else {
		//	サイズ調整無し
		if( Mode->Inter && Mode->Mode != MD_SC6 && Mode->Mode != MD_SC7 && Mode->Mode != MD_SC6_256L && Mode->Mode != MD_SC7_256L ) {
			cnvCopyInter( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
		}else{
			cnvCopy( in, width, height, out, owidth, oheight, prProg, Mode->Seido );
		}
	}

	LocalFree(in);
	in=NULL;
	// プレビューがあれば元画像を保存する
	if( Mode->PreView & 2 ) pvDrawBmp( hMemDC, prv, out, owidth, oheight );
	// パレットを求める
	if( !CreatePal( n, Mode, out, owidth, oheight, Pal ) ){
		LocalFree( out );
		return;
	}
	//	ﾊﾟﾚｯﾄをｿｰﾄする
	if( Mode->Mode < MD_SC8 || (Mode->Mode >= MD_SC5_256L && Mode->Mode < MD_SC8_256L) ) {
		if( !( Mode->Pal && (Mode->SortMode == SM_INCAUTO) ) ) {
			cnvSortPalette( Mode, Pal );
		}
	}
	// タイルパターンを求める
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
	// 変換結果を格納するメモリを確保する
	bmp = (BYTE*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,0x20000 );	//	128KB確保（VRAMイメージ改）
	if( bmp==NULL ){
		MessageBox( NULL,cszNotEnoughMem,MsgCap,MB_OK | MB_ICONWARNING );
		return;
	}
	// 変換する(COLORREF配列→ＭＳＸ画像)
	if( Mode->Mode < MD_SC10 || (Mode->Mode >= MD_SC5_256L && Mode->Mode <= MD_SC10_256L) ) {	// Screen10/12 以外
		cnvRecolor( out, owidth, oheight, bmp, Mode, prProg, Pal, tail, tailcnt );
	}else{							// Screen10/12
		cnvNtcolor( out, owidth, oheight, bmp, Mode, prProg );
	}
	LocalFree( out );
	out=NULL;
	// フォーカスを移す
	SetFocus( hWnd );
	// 変換結果を保存する
	if( n == 16) owidth /= 2; else if( n == 4 ) owidth /= 4;
	if( Mode->PreView & 1 ){
		tMode = *Mode;
		if( tBmpview[nSrc].sFileName[0]=='\0' ){			// クリップボードからの場合はファイル名自動決定は無効
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
		tPreview[prv].bmp=bmp;								// 変換後の VRAM イメージを記憶
		tPreview[prv].tMode=*Mode;							// 現在の設定を記憶
		tPreview[prv].nSrc = nSrc;							// プレビューの元ファイル
		SelectBitmap( hMemDC,tPreview[prv].hBmp );			// プレビューを作成する
		draw_screen( bmp,hMemDC,Mode );
		SelectBitmap( hMemDC,hMemBM );						// 元に戻す
		pvIndex = prv;										// このプレビューを表示させる
		bSrc=false;											// 変換結果の画像を表示する
		UpdateVScreen();									// 画面の更新
		return;
	}
	// メモリの解放
	LocalFree( bmp );
}

// -----------------------------------------------------
//	1.	日本語名
//		ピクセル値取得
//	2.	引数
//		in		...	(I)	画像メモリ
//		width	...	(I)	幅
//		height	...	(I)	高さ
//		x		...	(I)	Ｘ座標
//		y		...	(I)	Ｙ座標
//	3.	返値
//		色
//	4.	備考
//		なし
// -----------------------------------------------------
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y )
{
	// 範囲外修正
	if( x<0 ) x=0;
	if( y<0 ) y=0;
	if( x>=width  ) x=width -1;
	if( y>=height ) y=height-1;
	// 指定の位置の色を返す
	return( *( in + x + (height-y)*width ) );
}

// -----------------------------------------------------
//	1.	日本語名
//		パレットの作成
//	2.	引数
//		n		...	(O)		パレット数
//		Mode	...	(I/O)	設定
//		in		...	(I)		入力画像
//		w		...	(I)		入力画像の幅
//		h		...	(I)		入力画像の高さ
//		Pal		...	(O)		パレットの色（フルカラー）
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
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
		if( !Mode->Pal ){		// 最適パレット
			t = (COLORREF*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,sizeof( COLORREF)*w*h );	// 作業領域の確保
			if( t==NULL ){
				MessageBox( hWnd, cszNotEnoughMem, MsgCap, MB_OK | MB_ICONWARNING );
				return false;
			}
			CopyMemory( t, in, w * h * sizeof( COLORREF ) );
			cnvGetPalette( t, w, h, Pal, Mode->SelCol, n, pp, Mode->FourceZero != FZ_NONE,Mode->FZColor );
			LocalFree( t );
			// 最適パレットを固定パレットへ反映
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
		// 固定パレット
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
//	1.	日本語名
//		画面を更新する
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
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
	// DCの確保
	hDC=GetDC( hWnd );
	// 仮想画面に画像を描画
	if( pvIndex == -1 ){
		StretchBlt( hMemDC, 0, 0, cnWidth, cnHeight, hBGDC, 0, 0, cnBGWidth, cnBGHeight, SRCCOPY );
	}else{
		hMDC = CreateCompatibleDC( hDC );
		if( bSrc ){	// ソースの表示
			SelectBitmap( hMDC, tPreview[pvIndex].hBmpSrc );
		}else{		// ディスティネーションの表示
			SelectBitmap( hMDC, tPreview[pvIndex].hBmp );
		}
		FillRect( hMemDC, &r, (HBRUSH) GetStockObject( WHITE_BRUSH ) );
		BitBlt( hMemDC, 0, 0, cnWidth, cnHeight, hMDC,0,0, SRCCOPY );
		if( hMDC!=NULL ) DeleteDC( hMDC );
	}
	// 設定を描画
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
	// 画面に描画
	z = nZoom + 1;
	StretchBlt( hDC, 0, 0, cnWidth * z, cnHeight * z,
			 hMemDC, 0, 0, cnWidth    , cnHeight    , SRCCOPY );
	ReleaseDC( hWnd,hDC );
}

// -----------------------------------------------------
//	1.	日本語名
//		設定表示用文字列の作成
//	2.	引数
//		szBuf	...	(I)	設定文字列の出力先
//		szTitle	...	(I)	タイトル文字列
//		Mode	...	(I)	設定内容
//	3.	返値
//		なし
//	4.	備考
//		なし
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
