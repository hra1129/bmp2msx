// -------------------------------------------------------------
// BMP to MSX
// 設定ダイアログ
//											(C)2000/5/6 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include "Config.h"
#include "Setting.h"
#include "Resource.h"
#include <commctrl.h>
#include "Convert.h"
#include "utils.h"
#include "winuty.h"
#include "bsave.h"
#include "fileuty.h"

// -------------------------------------------------------------
// テーブル
// -------------------------------------------------------------
// 出力画面モードコンボボックスの内容
const char *ScrModeName[]={
	"SCREEN2/4",
	"SCREEN3",
	"SCREEN5",
	"SCREEN6",
	"SCREEN7",
	"SCREEN8",
	"SCREEN10/11",
	"SCREEN12",
	"SCREEN5(256lines)",
	"SCREEN6(256lines)",
	"SCREEN7(256lines)",
	"SCREEN8(256lines)",
	"SCREEN10/11(256lines)",
	"SCREEN12(256lines)",
};

//	ツールチップ定義
typedef struct {
	int		nId;
	char*	sStr;
} TOOLTIPS;

#ifdef _ENGLISH	// 英語(English)
// パレット出力モード
const char *PltMode[]={		// ※要素番号は PLT_XXXX の定義と一致する
	"no output",
	"into the picture file( COLOR=RESTORE ver. )",
	"*.PL? ( (C)BIT2 Graphsaurus v2.x ver. )",
	"*.CSV ( CSV text file )",
};

// ｱﾙｺﾞﾘｽﾞﾑ名
const char *AlgoName[]={		// ※要素番号は ALGO_XXXX の定義と一致する
	"no use",
	"error fix (give brightness)",
	"MSX2+ POWERFUL BOOK ver.",
	"NTSC fix(R:G:B=40:49:6)",
	"error fix (give tone)"
};

// ディザパターン名
const char *ErrAlgoName[]={	// ※要素番号は EALGO_XXXX の定義と一致する
	"no use",
	"random dots",
	"net dots",
	"net dots(double length)",
	"pattern of brick",
	"pattern of grain",
	"rough net",
	"lite net",
	"half tone",
};

// プレビューモード
const char *PreViewName[]={
	"file",
	"preview",
	"file and preview",
};

// 色再現精度
const char *ColorSeido[]={
	"8bit",
	"7bit",
	"6bit",
	"5bit",
	"4bit",
};

// ディザ加算方法
const char *ErrAlgo2Name[]={
	"rotate",
	"pile up",
	"move X",
	"move Y",
};

// パレット使用許可
const char *PalEnName[]={
	"automatic color",
	"no use",
	"customize color",
};

// 強制ゼロ化
const char *FZColorName[]={
	"no use",
	"appointed color No. -> No.0",
	"appointed (X,Y) color -> No.0",
};

const char *cOpenFile = "Open File";

static const TOOLTIPS ttips[] = {
	IDC_CMBSCREEN	,	"select a screen mode of MSX",
	IDC_PREVIEW		,	"select an output point (a preview, a file)",
	IDC_CMBERR		,	"select shape of pattern to increase the number of color",
	IDC_CMBERR2		,	"select it how I apply dither pattern in each element of RGB",
	IDC_SEIDO		,	"use high rank several bits of a former image as an effective value",
	IDC_GOSA		,	"scatter an error and do not stand out and do it",
	IDC_GOSAVAL		,	"appoint a value to let the error that spread splash it on each other. small and spread becomes weak",
	IDC_GOSAERR		,	"appoint the threshold value cutting off the error that spread. then spread becomes greatly weak",
	IDC_GOSARATIO	,	"Specifies the ratio of the right direction (X direction) to the bottom direction (Y direction) of the error to be spread. X:Y",
	IDC_INTER		,	"use an inter race mode of MSX and double resolution of perpendicular course",
	IDC_RESIZE		,	"an image is destroyed by the ratio regulating an input image in output size forcibly",
	IDC_JKRC		,	"I calculate J/K value which is most suitable for Y value which I calculated after Y decision again and revise it",
	IDC_AUTONAME	,	"I make the output file name from the input file name and do not display a file excellent input column",
	IDC_CMBALGO		,	"select a method of SCREEN10/12",
	IDC_SELCOL		,	"I select a lot of colors using with precedence (a color of outline lines may not be selected)",
	IDC_PAL			,	"I do not generate a color of a palette automatically",
	IDC_CHKZERO		,	"I do not use a color of the 0 and move it in other colors (for TPSET)",
	IDC_TILE		,	"A net of ratio handles a design as an usual color at 1:1 of extremely near color and increases a color",
	IDC_ENREVS		,	"apply automatic decision setting of color set beneath to all palettes",
	IDC_PALEN		,	"I select it whether I decide a color automatically whether I appoint a value",
	IDC_FZERO		,	"I select a method of the color appointment that turns a color of specification into a color of the 0 forcibly (for TPSET)",
	IDC_FZCOLOR		,	"It is a button to appoint a color to turn into the 0 forcibly",
	IDC_FZC			,	"It is a color to turn into the 0 forcibly",
	IDC_FZ_X		,	"It is a coordinate at gathering a color from an image to turn into the 0 forcibly",
	IDC_FZ_Y		,	"It is a coordinate at gathering a color from an image to turn into the 0 forcibly",
	IDC_CMBPLT		,	"I appoint an output method of a palette",
	IDC_LOADPLT		,	"Reading adds to a palette file",
	IDC_MSX1COLOR	,	"turn a palette into a color of MSX1",
	IDC_MSX2COLOR	,	"turn a palette into a color of MSX2 default",
	IDC_192			,	"I output an image for a 192 line (an inter race, 384 line) when I put a check.",
	IDC_SORT		,	"I appoint a method to let a color stand in line of a palette.",
	IDC_SAVEAS		,	"I store setting displaying in a file currently.",
	IDC_LOAD		,	"Reading adds to the setting file which I stored formerly.",
	IDC_DEFAULT		,	"I adopt setting contents displaying as \'default setting\'.",
	IDC_CHKDEF		,	"Setting at having done re-conversion is copied immediately by default setting when I put a check.",
	IDC_CMBCNV		,	"I appoint setting to adopt when I convert a new image file into.",
	IDC_FILLC		,	"I smash it, and a method of coating to initialize it is a color",
	IDC_SIZEMODE	,	"I appoint a decision method of aspect ratio",
	IDC_RESAMPLE	,	"I appoint a sample method of color.",
};

const char *SortMode[] = {
	"Ignore",
	"Incriment(auto only)",
	"Decriment(auto only)",
	"Incriment",
	"Decriment",
};

const char *UseSetting[] = {
	"Default",
	"Last convert",
	"Indication image",
};

const char *Resample[] = {
	"normal",
	"antialiasing",
};

const char *SizeMode[] = {
	"output size forcibly",
	"look like aspect ratio on MSX",
};

#else			// 日本語(Japanese)

// パレット出力モード
const char *PltMode[]={		// ※要素番号は PLT_XXXX の定義と一致する
	"出力しない",
	"画像に結合(COLOR=RESTORE用)",
	"グラフサウルス *.PL? 形式",
	"CSV形式",
};

// ｱﾙｺﾞﾘｽﾞﾑ名
const char *AlgoName[]={		// ※要素番号は ALGO_XXXX の定義と一致する
	"なし",
	"誤差訂正（輝度重視）",
	"MSX2+ﾊﾟﾜﾌﾙ活用法ﾚｲﾄﾚ方式",
	"NTSC改(R:G:B=40:49:6)",
	"誤差訂正（色合い重視）"
};

// ディザパターン名
const char *ErrAlgoName[]={	// ※要素番号は EALGO_XXXX の定義と一致する
	"なし",
	"疑似ランダム",
	"高度な網がけ",
	"縦長の網がけ",
	"レンガ模様",
	"つぶつぶ模様",
	"粗い網がけ",
	"薄い網がけ",
	"ハーフトーン",
};

// プレビューモード
const char *PreViewName[]={
	"ファイルへ出力",
	"プレビューのみ作成",
	"プレビューを作成してファイルへ出力",
};

// 色再現精度
const char *ColorSeido[]={
	"8bit",
	"7bit",
	"6bit",
	"5bit",
	"4bit",
};

// ディザ加算方法
const char *ErrAlgo2Name[]={
	"回転",
	"同位相",
	"Ｘずらし",
	"Ｙずらし",
};

// パレット使用許可
const char *PalEnName[]={
	"色自動決定",
	"使用無許可",
	"設定色を使用",
};

// 強制ゼロ化
const char *FZColorName[]={
	"使用しない",
	"指定色ｾﾞﾛ化",
	"指定座標色ｾﾞﾛ化",
};

const char *cOpenFile = "ファイルを開く";

static const TOOLTIPS ttips[] = {
	IDC_CMBSCREEN	,	"MSX の画面モードを選択します",
	IDC_PREVIEW		,	"出力先（プレビュー、ファイル）を選択します",
	IDC_CMBERR		,	"擬似的に色数を増やすためのパターンの形状を選択します",
	IDC_CMBERR2		,	"RGBの各要素にどのようにディザパターンを適用するか選択します",
	IDC_SEIDO		,	"元画像の上位何ビットを有効値として使用するか選択します",
	IDC_GOSA		,	"誤差を拡散して目立たなくします",
	IDC_GOSAVAL		,	"拡散した誤差に掛け合わせる値を指定します、小さくすると拡散が弱くなります",
	IDC_GOSAERR		,	"拡散した誤差を切り捨てる敷居値を指定します、大きくすると拡散が弱くなります",
	IDC_GOSARATIO	,	"拡散する誤差の右方向(X方向)と下方向(Y方向)の比率を指定します。X:Y",
	IDC_INTER		,	"MSXのインターレースモードを使って垂直方向の解像度を２倍にします",
	IDC_RESIZE		,	"入力画像のサイズ調節方法を使用するか否か指定します",
	IDC_JKRC		,	"算出した Y 値に最適な J/K 値を Y 決定後に再計算して補正します",
	IDC_AUTONAME	,	"出力ファイル名を入力ファイル名から作成し、ファイル名入力欄を表示しなくなります",
	IDC_CMBALGO		,	"自然画減色の方法を選択します",
	IDC_SELCOL		,	"たくさん使っている色を優先的に選びます（輪郭線などの色が選ばれない可能性があります）",
	IDC_PAL			,	"パレットの色を自動生成しません",
	IDC_CHKZERO		,	"0番の色を使わず、別の色に置き換えます（TPSET用）",
	IDC_TILE		,	"ごく近い色の 1:1 比率の網がけ模様を通常の色として扱い擬似的に色を増やします",
	IDC_ENREVS		,	"下に設定してある色の自動決定設定を全てのパレットに適用します",
	IDC_PALEN		,	"色を自動的に決定するか、値を指定するか選択します",
	IDC_FZERO		,	"特定の色を強制的に 0番の色にします（TPSET用）、その色指定の方法を選択します",
	IDC_FZCOLOR		,	"強制的に 0番にする色を指定するボタンです",
	IDC_FZC			,	"強制的に 0番にする色です",
	IDC_FZ_X		,	"強制的に 0番にする色を画像から採取する場合の座標です",
	IDC_FZ_Y		,	"強制的に 0番にする色を画像から採取する場合の座標です",
	IDC_CMBPLT		,	"パレットの出力方法を指定します",
	IDC_LOADPLT		,	"パレットファイルを読み込みます",
	IDC_MSX1COLOR	,	"パレットをＭＳＸ１風の色にします",
	IDC_MSX2COLOR	,	"パレットをＭＳＸ２の初期値の色にします",
	IDC_192			,	"チェックをいれると192ライン（インターレース時は384ライン）用の画像を出力します",
	IDC_SORT		,	"パレットの色を整列させる方法を指定します",
	IDC_SAVEAS		,	"現在表示している設定をファイルに保存します",
	IDC_LOAD		,	"以前保存した設定ファイルを読み込みます、現在の表示内容は消えてしまうのでご注意ください",
	IDC_DEFAULT		,	"現在表示している設定内容を デフォルトの設定 として採用します",
	IDC_CHKDEF		,	"チェックを入れると、再変換を行ったときの設定がすぐさまデフォルト設定にコピーされます",
	IDC_CMBCNV		,	"新しい画像ファイルを変換するときに採用する設定を指定します",
	IDC_FILLC		,	"画像の存在しない部分をこの色で埋めます。パレット決定にも影響する場合があるので注意",
	IDC_SIZEMODE	,	"縦横比の決定方法を指定します",
	IDC_RESAMPLE	,	"サイズ調節の際の、色のサンプリング方法を指定します。一般的にノーマルの場合、縮小画像は激しく劣化します。",
};

const char *SortMode[] = {
	"おまかせ",
	"自動決定のみ昇順ｿｰﾄ",
	"自動決定のみ降順ｿｰﾄ",
	"完全昇順ｿｰﾄ",
	"完全降順ｿｰﾄ",
};

const char *UseSetting[] = {
	"ﾃﾞﾌｫﾙﾄ設定",
	"最後の変換設定",
	"表示画像の設定",
};

const char *Resample[] = {
	"ﾉｰﾏﾙ",
	"ｱﾝﾁｴｲﾘｱｽ",
};

const char *SizeMode[] = {
	"強制的に出力ｻｲｽﾞにあわせる",
	"MSXで同じ縦横比に見えるｻｲｽﾞ",
};

#endif

// 各モードに応じたコントロールの有効・無効情報
static const int nCtrlCode[]={
	IDC_GOSA,IDC_GOSAVAL,IDC_GOSAERR,IDC_GOSARATIO,IDC_PAL,IDC_SELCOL,IDC_CMBPLT,IDC_COLTBL,IDC_LOADPLT,IDC_RED,IDC_GREEN,IDC_BLUE,IDC_CMBALGO,IDC_JKRC,IDC_PALEN,IDC_ENREVS,IDC_CMBERR2,IDC_TILE,IDC_SORT,IDC_INTER,IDC_MSX1COLOR, IDC_MSX2COLOR,IDC_192,IDC_SAVEPLT,
};

static const BOOL nCtrlEnbl[][ ELMCNT(nCtrlCode) ]={
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , FALSE   ,TRUE         ,TRUE   ,FALSE  ,TRUE  },   // SCREEN2/4
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , FALSE   ,TRUE         ,TRUE   ,FALSE  ,TRUE  },   // SCREEN3
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN5
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN6
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN7
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,FALSE      ,FALSE   ,FALSE    ,FALSE     ,TRUE       ,TRUE    ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN8
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN10/11
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN12
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN5 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN6 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN7 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,FALSE      ,FALSE   ,FALSE    ,FALSE     ,TRUE       ,TRUE    ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN8 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN10/11 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN12 (256 lines)
};

SETTING EdtMode;
extern SETTING NowMode;
extern bool bSave;		// WndProc.cpp

// -------------------------------------------------------------
// プロトタイプ
// -------------------------------------------------------------
EVENT( onSetClose		);
EVENT( onSetCommand		);
EVENT( onSetHScroll		);
EVENT( onSetInitDlg		);
EVENT( onSetInter		);
EVENT( onSetLoadPlt		);
EVENT( onSetMode		);
EVENT( onSetOk			);
EVENT( onSetGosa		);
EVENT( onSetPal			);
EVENT( onSetPltMode		);
EVENT( onSetResize		);
EVENT( onSetResample	);
EVENT( onSetSizeMode	);
EVENT( onSetSelCol		);
EVENT( onSetAutoName	);
EVENT( onSetAlgo		);
EVENT( onSetJKrc		);
EVENT( onSetErr			);
EVENT( onSetPreView		);
EVENT( onSetPalEnable	);
EVENT( onSetEnRevs		);
EVENT( onSetSaveAs		);
EVENT( onSetLoad		);
EVENT( onSetSeido		);
EVENT( onSetErrAdd		);
EVENT( onSetNonZero		);
EVENT( onSetFZero		);
EVENT( onSetFZColor		);
EVENT( onSetFCColor		);
EVENT( onSetFZX			);
EVENT( onSetFZY			);
EVENT( onSetTile		);
EVENT( onSetSort		);
EVENT( onSetDefault		);
EVENT( onSetChkDef		);
EVENT( onSetCmbCnv		);
EVENT( onSetMSX1		);
EVENT( onSetMSX2		);
EVENT( onSet192			);
EVENT( onSetSavePlt		);

EVENT( onColRButtonDown	);
EVENT( onColLButtonDown	);
EVENT( onColPaint		);
EVENT( onColMouseMove	);
EVENT( onColLButtonUp	);

EVENT( onFZPaint		);
EVENT( onFCPaint		);

static void UpdateColor( int c );
static void SetEditColor( HWND hWnd,int c,bool s );
static void ChangeColor( int p1, int p2 );

static void LoadPLFile( const char *szFileName );
static void SetCtlEnable( HWND hWnd );
static void UpdateAll( HWND hWnd );
static void _enddlg( HWND hWnd,int exitcode );

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
static HDC		hMemDC = NULL;			// カラーテーブル格納用デバイス
static HBITMAP	hMemBM = NULL;			// カラーテーブル格納用ビットマップ
static int w,h;							// カラーテーブルのサイズ
static int NowCol=0;					// 現在編集中の色
static WNDPROC	ColProc = NULL;			// パレットのデフォルトウィンドウ関数
static WNDPROC	FZProc = NULL;			// 強制ゼロ化色のデフォルトウィンドウ関数
static WNDPROC	FCProc = NULL;			// 背景色のデフォルトウィンドウ関数
static bool bDraging = false;			// パレットをドラッグ中か否か
extern HINSTANCE	hIns;				// インスタンスハンドル( main.cpp )
bool bRedo			= false;			// 再変換かﾃﾞﾌｫﾙﾄ設定か
char szCaption[ 256 ]="";				// タイトル
static COLORREF	CustColors[16]={		// 色選択ダイアログで使用するカスタムカラー
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

// -------------------------------------------------------------
// イベント分岐

// -------------------------------------------------------------
//	1.	日本語名
//		ウィンドウ関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
WINPROC( SetProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onSetClose	);
		ONEVENT( WM_COMMAND		, onSetCommand	);
		ONEVENT( WM_HSCROLL		, onSetHScroll	);
		ONEVENT( WM_INITDIALOG	, onSetInitDlg	);
	}
	return FALSE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットのサブクラス関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
WINPROC( SubColProc )
{
	switch( msg )
	{
		ONEVENT( WM_RBUTTONDOWN	, onColRButtonDown	);
		ONEVENT( WM_LBUTTONDOWN	, onColLButtonDown	);
		ONEVENT( WM_PAINT		, onColPaint		);
		ONEVENT( WM_MOUSEMOVE	, onColMouseMove	);
		ONEVENT( WM_LBUTTONUP	, onColLButtonUp	);
	}
	return CallWindowProc( ColProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
//	1.	日本語名
//		強制ゼロ化色表示部分のサブクラス関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
WINPROC( SubFZProc )
{
	switch( msg )
	{
		ONEVENT( WM_PAINT		, onFZPaint			);
	}
	return CallWindowProc( FZProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
//	1.	日本語名
//		背景色表示部のサブクラス関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル」
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
WINPROC( SubFCProc )
{
	switch( msg )
	{
		ONEVENT( WM_PAINT		, onFCPaint			);
	}
	return CallWindowProc( FCProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
// 一般イベント

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
EVENT( onSetInitDlg )
{
	HDC		hDC;
	RECT	r;
	HWND	hCol;
	HWND	hTip;
	int		i;

	// タイトル
	SetWindowText( hWnd,szCaption );
	SetDlgItemText( hWnd, IDOK, szCaption );

	//	強制ゼロ化色のサブクラス化
	hCol = GetDlgItem( hWnd, IDC_FZC );
	FZProc = (WNDPROC) GetWindowLong( hCol, GWL_WNDPROC );
	SetWindowLong( hCol, GWL_WNDPROC, (LONG)SubFZProc );

	//	背景色のサブクラス化
	hCol = GetDlgItem( hWnd, IDC_FILLCOLOR );
	FCProc = (WNDPROC) GetWindowLong( hCol, GWL_WNDPROC );
	SetWindowLong( hCol, GWL_WNDPROC, (LONG)SubFCProc );

	//	カラーパレットのサブクラス化とビットマップの作成
	hCol = GetDlgItem( hWnd, IDC_COLTBL );
	ColProc = (WNDPROC) GetWindowLong( hCol, GWL_WNDPROC );
	SetWindowLong( hCol, GWL_WNDPROC, (LONG)SubColProc );

	hDC = GetDC( hWnd );
	GetClientRect( hCol, &r );
	w = r.right;
	h = r.bottom;
	hMemDC = CreateCompatibleDC( hDC );
	hMemBM = CreateCompatibleBitmap( hDC, w, h );
	if( hMemDC == NULL || hMemBM == NULL ) return TRUE;
	SelectBrush( hMemDC,GetStockBrush( NULL_BRUSH ) );
	SelectBitmap( hMemDC,hMemBM );
	UpdateAll( hWnd );

	//	ツールチップを付ける
	hTip = wu_ttip_create( hWnd );
	for( i = 0; i < ELMCNT( ttips ); ++i ) {
		wu_ttip_set_item( hTip, ttips[i].nId, ttips[i].sStr );
	}

	return TRUE;	// 自動で最初のフォーカスを当ててもらう
}

// -------------------------------------------------------------
//	1.	日本語名
//		ウィンドウ閉じる / ボタン：キャンセル
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetClose )
{
	_enddlg( hWnd,-1 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		コマンド分岐
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetCommand )
{
	switch( LOWORD( wp ) )
	{
	// テキストボックス
	ONEVENT( IDC_FZ_X		, onSetFZX		);		// 強制ゼロ化Ｘ座標
	ONEVENT( IDC_FZ_Y		, onSetFZY		);		// 強制ゼロ化Ｙ座標
	// ボタン
	ONEVENT( IDOK			, onSetOk		);		// OK
	ONEVENT( IDCANCEL		, onSetClose	);		// ｷｬﾝｾﾙ
	ONEVENT( IDC_LOADPLT	, onSetLoadPlt	);		// ﾊﾟﾚｯﾄ読み込み
	ONEVENT( IDC_ENREVS		, onSetEnRevs	);		// 全て同じ許可
	ONEVENT( IDC_SAVEAS		, onSetSaveAs	);		// ファイルへ保存
	ONEVENT( IDC_LOAD		, onSetLoad		);		// ファイルから読み出し
	ONEVENT( IDC_FZCOLOR	, onSetFZColor	);		// 強制ｾﾞﾛ化色指定
	ONEVENT( IDC_FILLC		, onSetFCColor	);		// 背景色指定
	ONEVENT( IDC_DEFAULT	, onSetDefault	);		// ﾃﾞﾌｫﾙﾄへｺﾋﾟｰ
	ONEVENT( IDC_MSX1COLOR	, onSetMSX1		);		// MSX1色
	ONEVENT( IDC_MSX2COLOR	, onSetMSX2		);		// MSX2デフォルト色
	ONEVENT( IDC_SAVEPLT	, onSetSavePlt	);		// ﾊﾟﾚｯﾄ保存
	// チェックボックス
	ONEVENT( IDC_INTER		, onSetInter	);		// インターレース
	ONEVENT( IDC_PAL		, onSetPal		);		// 固定パレット
	ONEVENT( IDC_SELCOL		, onSetSelCol	);		// 選色モード
	ONEVENT( IDC_AUTONAME	, onSetAutoName	);		// 自動ファイル名決定
	ONEVENT( IDC_JKRC		, onSetJKrc		);		// 色差の再計算
	ONEVENT( IDC_GOSA		, onSetGosa		);		// 誤差拡散
	ONEVENT( IDC_CHKZERO	, onSetNonZero	);		// ０番を使わない
	ONEVENT( IDC_TILE		, onSetTile		);		// 網がけタイル
	ONEVENT( IDC_CHKDEF		, onSetChkDef	);		// 常にﾃﾞﾌｫﾙﾄへｺﾋﾟｰ
	ONEVENT( IDC_192		, onSet192		);		// 192ライン出力
	ONEVENT( IDC_RESIZE		, onSetResize	);		// ｻｲｽﾞ調整
	// コンボボックス
	ONEVENT( IDC_RESAMPLE	, onSetResample	);		// ｻｲｽﾞ調整ﾘｻﾝﾌﾟﾙ
	ONEVENT( IDC_CMBSCREEN	, onSetMode		);		// Screen5
	ONEVENT( IDC_CMBPLT		, onSetPltMode	);		// パレットモード
	ONEVENT( IDC_CMBALGO	, onSetAlgo		);		// 自然画生成アルゴリズム
	ONEVENT( IDC_CMBERR		, onSetErr		);		// ディザパターン
	ONEVENT( IDC_PREVIEW		, onSetPreView	);		// 出力先
	ONEVENT( IDC_SEIDO		, onSetSeido	);		// 精度
	ONEVENT( IDC_CMBERR2	, onSetErrAdd	);		// ディザ加算方法
	ONEVENT( IDC_PALEN		, onSetPalEnable);		// パレット使用許可・未許可
	ONEVENT( IDC_FZERO		, onSetFZero	);		// 強制ｾﾞﾛ化
	ONEVENT( IDC_SORT		, onSetSort		);		// ソート
	ONEVENT( IDC_CMBCNV		, onSetCmbCnv	);		// 変換設定
	ONEVENT( IDC_SIZEMODE	, onSetSizeMode	);		// サイズ調整サイズモード
	}
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットで右クリックした
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onColRButtonDown )
{
	POINT	pos;
	int		col;
	HWND	hPWnd;

	// ダイアログのハンドル
	hPWnd = GetParent( hWnd );

	// マウスカーソルの位置を得る
	pos.x = GET_X_LPARAM( lp );
	pos.y = GET_Y_LPARAM( lp );

	// 指定している色番号を得る
	col = (((pos.x*8)/w) & 7) + (((pos.y*2)/h) & 1)*8;

	// パレットの有効無効設定を変更する
	EdtMode.PalEn[ col ] = ( EdtMode.PalEn[ col ] + 1 ) % ELMCNT( PalEnName );
	SetEditColor( hPWnd, col, true );

	InvalidateRect( hWnd,NULL,FALSE );

	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットで左クリックした
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onColLButtonDown )
{
	POINT	pos;
	int		c;

	// マウスカーソルの位置を得る
	pos.x = GET_X_LPARAM( lp );
	pos.y = GET_Y_LPARAM( lp );

	// 指定している色番号を得る
	c = (( ( pos.x * 8 ) / w ) & 7 ) + (( ( pos.y * 2 ) / h ) & 1 ) * 8;
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && (c > 3) ) return 0;
	SetEditColor( GetParent( hWnd ), c, true );
	InvalidateRect( hWnd, NULL, FALSE );

	//	マウスを占有
	SetCapture( hWnd );
	bDraging = true;

	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットの再描画
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onColPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd,&ps );

	BitBlt( hDC, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY );

	EndPaint( hWnd,&ps );

	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットで左ボタンを離した
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onColLButtonUp )
{
	POINT	pos;
	int		p;

	if( !bDraging ) goto skip;
	//	マウスを解放
	ReleaseCapture();
	bDraging = false;

	// マウスカーソルの位置を得る
	pos.x = GET_X_LPARAM( lp );
	pos.y = GET_Y_LPARAM( lp );

	//	範囲内判定
	if( pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h ) goto skip;

	// 選択色とカーソル位置の色を入れ替える
	p = (( ( pos.x * 8 ) / w ) & 7 ) + (( ( pos.y * 2 ) / h ) & 1 ) * 8;
	if( p < 0 || p > 15 ) goto skip;
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && (p > 3) ) goto skip;
	ChangeColor( NowCol, p );

	//	選択の更新
	SetEditColor( hWnd, p,true );
	InvalidateRect( hWnd, NULL, FALSE );
skip:
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットでカーソルが移動した
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		カーソルの形状を変える
// -------------------------------------------------------------
EVENT( onColMouseMove )
{
	if( bDraging ) {
		SetCursor( LoadCursor( hIns, MAKEINTRESOURCE( IDC_CHGPAL ) ) );
	} else {
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	}

	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		強制ゼロ化の再描画
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onFZPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		r;
	HBRUSH		hBr;

	hDC = BeginPaint( hWnd,&ps );

	GetClientRect( hWnd, &r );
	hBr = CreateSolidBrush( EdtMode.FZColor );
	if( hBr != NULL ) {
		FillRect( hDC, &r, hBr );
		DeleteBrush( hBr );
	}

	EndPaint( hWnd,&ps );

	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		背景色の再描画
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		0
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onFCPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		r;
	HBRUSH		hBr;

	hDC = BeginPaint( hWnd,&ps );

	GetClientRect( hWnd, &r );
	hBr = CreateSolidBrush( EdtMode.FCColor );
	if( hBr != NULL ) {
		FillRect( hDC, &r, hBr );
		DeleteBrush( hBr );
	}

	EndPaint( hWnd,&ps );

	return 0;
}

// -------------------------------------------------------------
// テキストボックス

// -------------------------------------------------------------
//	1.	日本語名
//		強制ゼロ化　Ｘ座標
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetFZX )
{
	EdtMode.FZX = 0;
	EdtMode.FZX = GetDlgItemInt( hWnd,IDC_FZ_X,NULL,FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		強制ゼロ化　Ｙ座標
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetFZY )
{
	EdtMode.FZY = 0;
	EdtMode.FZY = GetDlgItemInt( hWnd,IDC_FZ_Y,NULL,FALSE );
	return TRUE;
}

// -------------------------------------------------------------
// ボタンイベント

// -------------------------------------------------------------
//	1.	日本語名
//		ボタン：常にデフォルトへコピー
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetOk )
{
	if( EdtMode.Mode == MD_SC2 || EdtMode.Mode == MD_SC3 ) {
		EdtMode.Inter = false;
	}

	if( NowMode.bDefault ) {
		CALLEVENT( onSetDefault );
	}

	_enddlg( hWnd,0 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		強制ｾﾞﾛ化の色指定イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetFZColor )
{
	CHOOSECOLOR	cc;
	// 色選択ダイアログの設定
	memset( &cc, 0, sizeof( cc ) );
	cc.lStructSize	= sizeof( cc );
	cc.hwndOwner	= hWnd;
	cc.lpCustColors	= CustColors;
	cc.Flags		= CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult	= EdtMode.FZColor;
	if( !ChooseColor( &cc ) ) return TRUE;
	EdtMode.FZColor	= cc.rgbResult;
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		強制ｾﾞﾛ化の色指定イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetFCColor )
{
	CHOOSECOLOR	cc;
	// 色選択ダイアログの設定
	memset( &cc, 0, sizeof( cc ) );
	cc.lStructSize	= sizeof( cc );
	cc.hwndOwner	= hWnd;
	cc.lpCustColors	= CustColors;
	cc.Flags		= CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult	= EdtMode.FCColor;
	if( !ChooseColor( &cc ) ) return TRUE;
	EdtMode.FCColor	= cc.rgbResult;
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		全て同じ許可イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetEnRevs		)
{
	int i, j;

	if( EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L ) {
		j = 4;
	} else {
		j = 16;
	}

	for( i = 0; i < j; ++i ){
		EdtMode.PalEn[ i ] = EdtMode.PalEn[ NowCol ];
		UpdateColor( i );
	}
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ﾊﾟﾚｯﾄ読み込みイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetLoadPlt )
{
	char			szFileName[ MAX_PATH ];
	OPENFILENAME	ofn;
	int				i;
	// ファイル名を入力
	szFileName[ 0 ] = '\0';
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefPLL;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// カレント
	ofn.lpstrTitle		= PLLoad;
	if( !GetOpenFileName( &ofn ) ) return TRUE;
	// 読み込み
	i = strlen( szFileName );
	if( (i > 4) && ( toupper(szFileName[ i-3 ]) == 'P' ) ) {	//	*.P?? かどうか判定
		LoadPLFile( szFileName );
	} else {
		bsvLoadPLBsaveFile( &EdtMode, szFileName );
	}
	// 画面の更新
	for( i = 0; i < 16; ++i ){
		UpdateColor( i );
	}
	SetEditColor( hWnd, NowCol, true );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ﾊﾟﾚｯﾄ保存イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetSavePlt )
{
	char			szFileName[ MAX_PATH ];
	OPENFILENAME	ofn;
	// ファイル名を入力
	szFileName[ 0 ] = '\0';
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefPL;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// カレント
	ofn.lpstrTitle		= PLSave;
	if( !GetSaveFileName( &ofn ) ) return TRUE;
	// 読み込み
	bsvSavePLFile( hWnd, szFileName, &EdtMode );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定を保存イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetSaveAs )
{
	char szFileName[ MAX_PATH ];
	// ﾌｧｲﾙ名を入力
	if( !GetName( hWnd, szFileName, MAX_PATH, NULL, cszDefCfg, CfgExt ) ) return TRUE;
	// 保存処理
	SaveCfgFile( &EdtMode, szFileName );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定の読み出しイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetLoad )
{
	char szFileName[ MAX_PATH ];
	OPENFILENAME	ofn;
	// ファイルを開く
	szFileName[0]='\0';
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefCfg;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// カレント
	ofn.lpstrTitle		= cOpenFile;
	if( !GetOpenFileName( &ofn ) ) return TRUE;
	// 設定を反映
	GetCfgFile( &EdtMode, szFileName );
	// 画面の更新
	UpdateAll( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ﾃﾞﾌｫﾙﾄへｺﾋﾟｰ
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetDefault )
{
	bool	bDefault;

	//	再変換の時のみ処理する
	if( !bRedo ) return TRUE;

	bDefault = NowMode.bDefault;
	NowMode = EdtMode;
	NowMode.bDefault = bDefault;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＭＳＸ１色
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetMSX1 )
{
	set_msx1_palette( EdtMode.Col );
	UpdateAll( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＭＳＸ２デフォルト色
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetMSX2 ){
	set_msx2_palette( EdtMode.Col );
	UpdateAll( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
// チェックボックス

// -------------------------------------------------------------
//	1.	日本語名
//		０番を使わないチェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetNonZero )
{
	EdtMode.NonZero = !EdtMode.NonZero;
	UpdateColor( 0 );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}


// -------------------------------------------------------------
//	1.	日本語名
//		誤差拡散チェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetGosa )
{
	EdtMode.diffusion_error_enable = !EdtMode.diffusion_error_enable;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		誤差拡散チェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetPalEnable )
{
	EdtMode.PalEn[ NowCol ] = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_PALEN ) );
	UpdateColor( NowCol );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		インターレースチェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetInter )
{
	EdtMode.Inter = !EdtMode.Inter;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		自動ファイル名決定チェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetAutoName )
{
	EdtMode.AutoName = !EdtMode.AutoName;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		色差再計算チェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetJKrc )
{
	EdtMode.JKrc = !EdtMode.JKrc;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		全固定パレットチェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetPal )
{
	int i;
	EdtMode.Pal = !EdtMode.Pal;
	for( i=0; i<16; ++i ){
		UpdateColor( i );
	}
	InvalidateRect( hWnd,NULL,FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		選色モードチェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetSelCol )
{
	EdtMode.SelCol = !EdtMode.SelCol;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		網がけタイルチェックイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetTile )
{
	EdtMode.Tile = !EdtMode.Tile;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		常にデフォルト化
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetChkDef )
{
	EdtMode.bDefault = !EdtMode.bDefault;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		192ラインモード
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSet192 )
{
	EdtMode.b192 = !EdtMode.b192;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		サイズ調節イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetResize )
{
	EdtMode.Resize = !EdtMode.Resize;
	SetCtlEnable( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
// スライダ

// -------------------------------------------------------------
//	1.	日本語名
//		スライダイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetHScroll )
{
	HWND	hCol = (HWND)lp;
	int		pos  = SendMessage( hCol, TBM_GETPOS, 0, 0 );
	char	szBuf[32];

	switch( GetDlgCtrlID( hCol ) )
	{
	case IDC_GOSAVAL:	// 誤差拡散係数
		EdtMode.diffusion_error_coef = float( pos ) / 1000;
		wsprintf( szBuf, "0.%03d", pos );
		SetDlgItemText( hWnd, IDC_GOSAVALS, szBuf );
		return TRUE;
	case IDC_GOSAERR:	// 誤差切り捨て値
		EdtMode.err = pos;
		wsprintf( szBuf, "%d", pos );
		SetDlgItemText( hWnd, IDC_GOSAERRS, szBuf );
		return TRUE;
	case IDC_GOSARATIO:	// 誤差比率
		EdtMode.diffusion_error_x_weight = float( pos ) / 100;
		wsprintf( szBuf, "%3d%% : %3d%%", pos, 100 - pos );
		SetDlgItemText( hWnd, IDC_GOSARATIOS, szBuf );
		return TRUE;
	case IDC_RED:		// 赤
		EdtMode.Col[ NowCol ].red   = pos;
		break;
	case IDC_GREEN:		// 緑
		EdtMode.Col[ NowCol ].green = pos;
		break;
	case IDC_BLUE:		// 青
		EdtMode.Col[ NowCol ].blue  = pos;
		break;
	default:
		return FALSE;
	}
	SetEditColor( hWnd, NowCol, false );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
// コンボボックス

// -------------------------------------------------------------
//	1.	日本語名
//		サイズ調節リサンプルイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetResample )
{
	EdtMode.Resample = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_RESAMPLE ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		サイズ調節サイズモード
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetSizeMode )
{
	EdtMode.SizeMode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_SIZEMODE ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		モード設定コンボボックスイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetFZero )
{
	EdtMode.FourceZero = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_FZERO ) );
	// コントロールの ON/OFF
	SetCtlEnable( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		モード設定コンボボックスイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetMode )
{
	if( HIWORD( wp ) != CBN_SELCHANGE ) return TRUE;
	EdtMode.Mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBSCREEN ) );
	// コントロールの ON/OFF
	SetCtlEnable( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		自然画生成アルゴリズムイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetAlgo )
{
	EdtMode.AlgoMode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBALGO ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ディザパターンイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetErr )
{
	EdtMode.ErrAlgo = ComboBox_GetCurSel( GetDlgItem( hWnd,IDC_CMBERR ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ディザ加算方法イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetErrAdd )
{
	EdtMode.ErrAdd = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBERR2 ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		出力先イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetPreView )
{
	EdtMode.PreView = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_PREVIEW ) ) + 1;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		精度イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetSeido )
{
	EdtMode.Seido = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_SEIDO ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットモードイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetPltMode )
{
	EdtMode.PltMode = ComboBox_GetCurSel( GetDlgItem( hWnd,IDC_CMBPLT ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ソートモードイベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetSort )
{
	EdtMode.SortMode = ComboBox_GetCurSel( GetDlgItem( hWnd,IDC_SORT ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		変換設定
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSetCmbCnv )
{
	EdtMode.CnvMode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBCNV ) );
	return TRUE;
}

// -------------------------------------------------------------
// ユーティリティ

// -------------------------------------------------------------
//	1.	日本語名
//		画面を更新する
//	2.	引数
//		hWnd	..	(I)	ダイアログのハンドル
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void UpdateAll( HWND hWnd )
{
	int		i;
	char	szBuf[ 32 ];
	// カラーテーブルを描画する
	for( i = 0; i < 16; i++ ) UpdateColor( i );
	// 出力モード
	Button_SetCheck( GetDlgItem( hWnd, IDC_GOSA		), EdtMode.diffusion_error_enable		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_INTER	), EdtMode.Inter	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_PAL		), EdtMode.Pal		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_SELCOL	), EdtMode.SelCol	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_RESIZE	), EdtMode.Resize	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_AUTONAME	), EdtMode.AutoName	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_JKRC		), EdtMode.JKrc		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_CHKZERO	), EdtMode.NonZero	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_TILE		), EdtMode.Tile		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_CHKDEF	), EdtMode.bDefault	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_192		), EdtMode.b192		? BST_CHECKED : BST_UNCHECKED );
	// スライダの範囲を設定
	SetSliderRange( GetDlgItem( hWnd, IDC_GOSAVAL   ), 1, 500 );
	SetSliderRange( GetDlgItem( hWnd, IDC_GOSAERR   ), 0, 255 );
	SetSliderRange( GetDlgItem( hWnd, IDC_GOSARATIO ), 0, 100 );
	SetSliderRange( GetDlgItem( hWnd, IDC_RED       ), 0, 7   );
	SetSliderRange( GetDlgItem( hWnd, IDC_GREEN     ), 0, 7   );
	SetSliderRange( GetDlgItem( hWnd, IDC_BLUE      ), 0, 7   );
	// スライダの位置を調節
	SetSliderValue( GetDlgItem( hWnd, IDC_GOSAVAL   ), (short int)(EdtMode.diffusion_error_coef * 1000) );
	SetSliderValue( GetDlgItem( hWnd, IDC_GOSAERR   ), (short int)(EdtMode.err) );
	SetSliderValue( GetDlgItem( hWnd, IDC_GOSARATIO ), (short int)(EdtMode.diffusion_error_x_weight * 100) );
	wsprintf( szBuf, "0.%03d", int(EdtMode.diffusion_error_coef * 1000) );
	SetDlgItemText( hWnd, IDC_GOSAVALS, szBuf );
	wsprintf( szBuf, "%d", EdtMode.err );
	SetDlgItemText( hWnd, IDC_GOSAERRS, szBuf );
	wsprintf( szBuf, "%3d%% : %3d%%", int(EdtMode.diffusion_error_x_weight * 100), 100 - int(EdtMode.diffusion_error_x_weight * 100) );
	SetDlgItemText( hWnd, IDC_GOSARATIOS, szBuf );
	SetEditColor( hWnd, 0, true );
	InvalidateRect( hWnd, NULL, FALSE );
	// コンボボックス
	ComboBox_Init( hWnd, IDC_CMBSCREEN	,ScrModeName,	ELMCNT( ScrModeName ),	EdtMode.Mode );
	ComboBox_Init( hWnd, IDC_CMBPLT		,PltMode,		PLT_COUNT,				EdtMode.PltMode );
	ComboBox_Init( hWnd, IDC_CMBALGO		,AlgoName,		ELMCNT( AlgoName ),		EdtMode.AlgoMode );
	ComboBox_Init( hWnd, IDC_CMBERR		,ErrAlgoName,	ELMCNT( ErrAlgoName ),	EdtMode.ErrAlgo );
	ComboBox_Init( hWnd, IDC_PREVIEW		,PreViewName,	ELMCNT( PreViewName ),	EdtMode.PreView - 1 );
	ComboBox_Init( hWnd, IDC_SEIDO		,ColorSeido,	ELMCNT( ColorSeido ),	EdtMode.Seido );
	ComboBox_Init( hWnd, IDC_CMBERR2		,ErrAlgo2Name,	ELMCNT( ErrAlgo2Name ),	EdtMode.ErrAdd );
	ComboBox_Init( hWnd, IDC_PALEN		,PalEnName,		ELMCNT( PalEnName ),	EdtMode.PalEn[ NowCol ] );
	ComboBox_Init( hWnd, IDC_FZERO		,FZColorName,	ELMCNT( FZColorName ),	EdtMode.FourceZero );
	ComboBox_Init( hWnd, IDC_SORT		,SortMode,		ELMCNT( SortMode ),		EdtMode.SortMode );
	ComboBox_Init( hWnd, IDC_CMBCNV		,UseSetting,	ELMCNT( UseSetting ),	EdtMode.CnvMode );
	ComboBox_Init( hWnd, IDC_RESAMPLE	,Resample,		ELMCNT( Resample ),		EdtMode.Resample );
	ComboBox_Init( hWnd, IDC_SIZEMODE	,SizeMode,		ELMCNT( SizeMode ),		EdtMode.SizeMode );
	// テキストボックス
	SetDlgItemInt( hWnd, IDC_FZ_X, EdtMode.FZX, FALSE );
	SetDlgItemInt( hWnd, IDC_FZ_Y, EdtMode.FZY, FALSE );
	// コントロールの ON/OFF
	SetCtlEnable( hWnd );
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーテーブルの色を更新する
//	2.	引数
//		c	...	(I)	更新する色番号
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void UpdateColor( int c )
{
	RECT	r;
	HBRUSH	hBr;
	HPEN	hOld, hPen1, hPen2;
	bool	bDisable;

	//	SCREEN 6 対策
	bDisable = ((EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && c > 3);

	r.left		= w * (  c & 7 )	   / 8;
	r.right		= w * (( c & 7 ) + 1 ) / 8;
	if( c & 8 ){
		r.top		= h / 2;
		r.bottom	= h;
	}else{
		r.top		= 0;
		r.bottom	= h / 2;
	}
	if( bDisable ) {
		hBr = CreateSolidBrush( GetSysColor( COLOR_APPWORKSPACE ) );
	} else {
		hBr = CreateSolidBrush( RGB( convert7to255_r[ EdtMode.Col[ c ].red   ],
									 convert7to255_g[ EdtMode.Col[ c ].green ],
									 convert7to255_b[ EdtMode.Col[ c ].blue  ] ) );
	}
	FillRect( hMemDC, &r, hBr );
	DeleteBrush( hBr );
	if( !bDisable ) {
		hPen1 = CreatePen( PS_SOLID, 0, COLCUR1 );
		hPen2 = CreatePen( PS_SOLID, 0, COLCUR2 );
		if( EdtMode.PalEn[ c ] == PALEN_NONE || ( c == 0 && EdtMode.NonZero ) ){
			// 使用無許可の場合は斜線を入れる
			DrawIcon( hMemDC, r.left, r.top, LoadIcon( wu_get_instance(), MAKEINTRESOURCE(IDI_NOUSE) ) );
		}
		if( EdtMode.PalEn[ c ] == PALEN_USE || EdtMode.Pal ){
			// 固定なら横線を入れる
			DrawIcon( hMemDC, r.left, r.top, LoadIcon( wu_get_instance(), MAKEINTRESOURCE(IDI_HOLD) ) );
		}
		if( c == NowCol ){
			// 現在の着目点なら輪郭の表示
			hOld = SelectPen( hMemDC, hPen1 );
			Rectangle( hMemDC, r.left + 1, r.top + 1, r.right - 1, r.bottom - 1 );
			SelectPen( hMemDC, hPen2 );
			Rectangle( hMemDC, r.left, r.top, r.right, r.bottom );
			SelectPen( hMemDC, hOld );
		}
		DeletePen( hPen2 );
		DeletePen( hPen1 );
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		２つのパレットの内容を交換する
//	2.	引数
//		p1		...	(I)	パレット番号１
//		p2		...	(I)	パレット番号２
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void ChangeColor( int p1, int p2 )
{
	int	t;

	//	SCREEN6 対策
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && ((p1 > 3) || (p2 > 3)) ) return;

	//	パレット値の交換
	t = EdtMode.Col[ p1 ].red;		EdtMode.Col[ p1 ].red	= EdtMode.Col[ p2 ].red;	EdtMode.Col[ p2 ].red	= t;
	t = EdtMode.Col[ p1 ].green;	EdtMode.Col[ p1 ].green	= EdtMode.Col[ p2 ].green;	EdtMode.Col[ p2 ].green	= t;
	t = EdtMode.Col[ p1 ].blue;		EdtMode.Col[ p1 ].blue	= EdtMode.Col[ p2 ].blue;	EdtMode.Col[ p2 ].blue	= t;

	//	許可状態の交換
	t = EdtMode.PalEn[ p1 ];		EdtMode.PalEn[ p1 ]		= EdtMode.PalEn[ p2 ];		EdtMode.PalEn[ p2 ]		= t;
}

// -------------------------------------------------------------
//	1.	日本語名
//		編集色を変更する
//	2.	引数
//		hWnd	...	(I)	ダイアログハンドル
//		c		...	(I)	編集したい色
//		s		...	(I)	スライダを更新するか否か
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void SetEditColor( HWND hWnd, int c, bool s )
{
	char szBuf[ 32 ];
	int back;
	back	= NowCol;
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && c > 3) return;
	NowCol	= c;
	UpdateColor( back );
	if( s ){
		SetSliderValue( GetDlgItem( hWnd,IDC_RED     ),EdtMode.Col[c].red   );
		SetSliderValue( GetDlgItem( hWnd,IDC_GREEN   ),EdtMode.Col[c].green );
		SetSliderValue( GetDlgItem( hWnd,IDC_BLUE    ),EdtMode.Col[c].blue  );
	}
	wsprintf( szBuf, "COLOR=%d", c );
	SetDlgItemText( hWnd, IDC_COL	, szBuf );
	wsprintf( szBuf, "R=%d", EdtMode.Col[ c ].red   );
	SetDlgItemText( hWnd, IDC_NRED	, szBuf );
	wsprintf( szBuf, "G=%d", EdtMode.Col[ c ].green );
	SetDlgItemText( hWnd, IDC_NGREEN, szBuf );
	wsprintf( szBuf, "B=%d", EdtMode.Col[ c ].blue  );
	SetDlgItemText( hWnd, IDC_NBLUE	, szBuf );
	UpdateColor( NowCol );
	ComboBox_SetCurSel( GetDlgItem( hWnd, IDC_PALEN	), EdtMode.PalEn[ NowCol ] );
}

// -------------------------------------------------------------
//	1.	日本語名
//		PL? 形式のパレットを読み込む
//	2.	引数
//		szFileName	...	(I)	ファイル名
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void LoadPLFile( const char *szFileName )
{
	FU_FILE	*hFile;
	BYTE	Buf[ 32 ];
	char	szBuf[ MAX_PATH + 256 ];
	int		i;

	// ファイルを開く
	hFile = fu_open( szFileName, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
#ifdef _ENGLISH
		wsprintf( szBuf,"Read error (%s)",szFileName );
#else
		wsprintf( szBuf,"%s の読み込みに失敗しました。",szFileName );
#endif
		MessageBox( NULL, szBuf, MsgCap, MB_OK | MB_ICONWARNING );
		return;		// ファイルが開けない
	}
	// データを入力する
	memset( Buf, 0,sizeof( Buf ) );
	fu_read( hFile, Buf, sizeof( Buf ) );
	// 入力データを反映する
	for( i = 0; i < 16; ++i ){
		EdtMode.Col[ i ].red  = (Buf[ i * 2 + 0 ] >> 4 ) & 0x07;
		EdtMode.Col[ i ].blue =  Buf[ i * 2 + 0 ]        & 0x07;
		EdtMode.Col[ i ].green=  Buf[ i * 2 + 1 ]        & 0x07;
	}
	// 終了
	fu_close( hFile );
}

// -------------------------------------------------------------
//	1.	日本語名
//		モードに合わせた各コントロールの ON/OFF
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void SetCtlEnable( HWND hWnd )
{
	int i;
	//	一般のコントロール
	for( i = 0; i < ELMCNT( nCtrlCode ); ++i ){
		EnableWindow( GetDlgItem( hWnd,nCtrlCode[ i ] ), nCtrlEnbl[ EdtMode.Mode ][ i ] );
	}
	// 強制ｾﾞﾛ化
	if( EdtMode.Mode < MD_SC10 || (EdtMode.Mode >= MD_SC5_256L && EdtMode.Mode < MD_SC10_256L) ) {
		EnableWindow( GetDlgItem( hWnd, IDC_FZERO ), TRUE );
		switch( EdtMode.FourceZero ){
		case FZ_NONE:
			EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), FALSE );
			break;
		case FZ_COLOR:
			EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), TRUE  );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), FALSE );
			break;
		case FZ_MATRIX:
			EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), TRUE  );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), TRUE  );
			break;
		}
	}else{
		EnableWindow( GetDlgItem( hWnd, IDC_FZERO	), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), FALSE );
	}
	//	パレット表示
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && (NowCol > 3) ) {
		SetEditColor( hWnd, 3, true );
	}
	if( EdtMode.Mode < MD_SC8 || (EdtMode.Mode >= MD_SC5_256L && EdtMode.Mode < MD_SC8_256L) ) {
		for( i = 0; i < 16; ++i ){
			UpdateColor( i );
		}
		InvalidateRect( hWnd, NULL, FALSE );
		ShowWindow( GetDlgItem( hWnd, IDC_COLTBL ), SW_SHOW );
	} else {
		ShowWindow( GetDlgItem( hWnd, IDC_COLTBL ), SW_HIDE );
	}
	//	再変換とﾃﾞﾌｫﾙﾄ設定の違い
	if( bRedo ) {
		EnableWindow( GetDlgItem( hWnd, IDC_DEFAULT ), EdtMode.bDefault ? FALSE : TRUE );
		ShowWindow( GetDlgItem( hWnd, IDC_DEFAULT ), SW_SHOW );
		ShowWindow( GetDlgItem( hWnd, IDC_CHKDEF  ), SW_HIDE );
		ShowWindow( GetDlgItem( hWnd, IDC_CMBCNV  ), SW_HIDE );
		ShowWindow( GetDlgItem( hWnd, IDC_LBLSSET ), SW_HIDE );
	} else {
		ShowWindow( GetDlgItem( hWnd, IDC_DEFAULT ), SW_HIDE );
		ShowWindow( GetDlgItem( hWnd, IDC_CHKDEF  ), SW_SHOW );
		ShowWindow( GetDlgItem( hWnd, IDC_CMBCNV  ), SW_SHOW );
		ShowWindow( GetDlgItem( hWnd, IDC_LBLSSET ), SW_SHOW );
	}
	//	サイズ調整
	if( EdtMode.Resize ) {
		EnableWindow( GetDlgItem( hWnd, IDC_RESAMPLE ), TRUE );
		EnableWindow( GetDlgItem( hWnd, IDC_SIZEMODE ), TRUE );
	} else {
		EnableWindow( GetDlgItem( hWnd, IDC_RESAMPLE ), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_SIZEMODE ), FALSE );
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		終了
//	2.	引数
//		hWnd		...	(I)	ウィンドウハンドル
//		exitcode	...	(I)	ダイアログ終了コード
//	3.	返値
//		なし
//	4.	備考
//		リソースの解放を行う。
// -------------------------------------------------------------
static void _enddlg( HWND hWnd, int exitcode )
{
	// リソースの解放
	if( hMemDC!=NULL ) DeleteDC( hMemDC );
	if( hMemBM!=NULL ) DeleteBitmap( hMemBM );
	hMemDC=NULL;
	hMemBM=NULL;
	// ダイアログの終了
	EndDialog( hWnd,exitcode );
}
