// -------------------------------------------------------------
// BMP to MSX
// キャリブレーションダイアログ
//											(C)2022/7/20 HRA!
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
static const char *p_screen_mode_name[]={
	"RGB 512 Colors palette",
	"SCREEN8",
	"SCREEN12",
};

// --------------------------------------------------------------------
// 要素の名前
static const char *p_element_pal512[] = {
	"RED 0", "RED 1", "RED 2", "RED 3", "RED 4", "RED 5", "RED 6", "RED 7",
	"GREEN 0", "GREEN 1", "GREEN 2", "GREEN 3", "GREEN 4", "GREEN 5", "GREEN 6", "GREEN 7",
	"BLUE 0", "BLUE 1", "BLUE 2", "BLUE 3", "BLUE 4", "BLUE 5", "BLUE 6", "BLUE 7",
};

// --------------------------------------------------------------------
static const char *p_element_screen8[] = {
	"RED 0", "RED 1", "RED 2", "RED 3", "RED 4", "RED 5", "RED 6", "RED 7",
	"GREEN 0", "GREEN 1", "GREEN 2", "GREEN 3", "GREEN 4", "GREEN 5", "GREEN 6", "GREEN 7",
	"BLUE 0", "BLUE 1", "BLUE 2", "BLUE 3",
};

// --------------------------------------------------------------------
static const char *p_element_screen12[] = {
	"RED 0", "RED 1", "RED 2", "RED 3", "RED 4", "RED 5", "RED 6", "RED 7",
	"RED 8", "RED 9", "RED 10", "RED 11", "RED 12", "RED 13", "RED 14", "RED 15",
	"RED 16", "RED 17", "RED 18", "RED 19", "RED 20", "RED 21", "RED 22", "RED 23",
	"RED 24", "RED 25", "RED 26", "RED 27", "RED 28", "RED 29", "RED 30", "RED 31",
	"GREEN 0", "GREEN 1", "GREEN 2", "GREEN 3", "GREEN 4", "GREEN 5", "GREEN 6", "GREEN 7",
	"GREEN 8", "GREEN 9", "GREEN 10", "GREEN 11", "GREEN 12", "GREEN 13", "GREEN 14", "GREEN 15",
	"GREEN 16", "GREEN 17", "GREEN 18", "GREEN 19", "GREEN 20", "GREEN 21", "GREEN 22", "GREEN 23",
	"GREEN 24", "GREEN 25", "GREEN 26", "GREEN 27", "GREEN 28", "GREEN 29", "GREEN 30", "GREEN 31",
	"BLUE 0", "BLUE 1", "BLUE 2", "BLUE 3", "BLUE 4", "BLUE 5", "BLUE 6", "BLUE 7",
	"BLUE 8", "BLUE 9", "BLUE 10", "BLUE 11", "BLUE 12", "BLUE 13", "BLUE 14", "BLUE 15",
	"BLUE 16", "BLUE 17", "BLUE 18", "BLUE 19", "BLUE 20", "BLUE 21", "BLUE 22", "BLUE 23",
	"BLUE 24", "BLUE 25", "BLUE 26", "BLUE 27", "BLUE 28", "BLUE 29", "BLUE 30", "BLUE 31",
};

// -------------------------------------------------------------
// 変数
static HDC		hMemDC = NULL;				// カラーテーブル格納用デバイス
static HBITMAP	hMemBM = NULL;				// カラーテーブル格納用ビットマップ
static WNDPROC	ColorBarProc = NULL;		// カラーバーのデフォルトウィンドウ関数
static int w;								// カラーバーのサイズ
static int h;								// カラーバーのサイズ

//	調整中の階調情報
static int pal512[ 3 * 8 ];					//	0～7: Red, 8～15: Green, 16～23: Blue
static int screen8[ 2 * 8 + 4 ];
static int screen12[ 3 * 32 ];

// -------------------------------------------------------------
// プロトタイプ
EVENT( onCalibrationInitDlg		);
EVENT( onCalibrationClose		);
EVENT( onCalibrationCommand		);
EVENT( onCalHScroll				);

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
WINPROC( CalibrationProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onCalibrationClose	);
		ONEVENT( WM_COMMAND		, onCalibrationCommand	);
		ONEVENT( WM_INITDIALOG	, onCalibrationInitDlg	);
		ONEVENT( WM_HSCROLL		, onCalHScroll			);
	}
	return FALSE;
}

// -------------------------------------------------------------
// 一般イベント

// -------------------------------------------------------------
//	1.	日本語名
//		座標変換
//	2.	引数
//		x ....	(I/O)	X座標
//		y ....	(I/O)	Y座標
//	3.	返値
//		なし
//	4.	備考
//		256 x 192 な座標系を rc の内側に配置した座標系に変換する
// -------------------------------------------------------------
static void convert_xy( LONG &x, LONG &y ){

	x = (int)( (x * ( w * 0.8 ) / 256.) + (w * 0.1) );
	y = (int)( (y * ( h * 0.8 ) / 192.) + (h * 0.1) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーバーの表示内容を更新する
//	2.	引数
//		hWnd	.....	(I)	ウィンドウハンドル
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void update_color_bar_pal512( HWND hWnd, RECT &rc ){
	HBRUSH hBrush;
	int x, y, index;
	int r, g, b;
	RECT fill_rc;

	index = 0;
	for( y = 0; y < 32; y++ ){
		for( x = 0; x < 16; x++ ){
			fill_rc.left = x * 16;
			fill_rc.top = y * 6;
			fill_rc.right = fill_rc.left + 16;
			fill_rc.bottom = fill_rc.top + 6;
			convert_xy( fill_rc.left, fill_rc.top );
			convert_xy( fill_rc.right, fill_rc.bottom );
			r = pal512[ index & 7 ];
			g = pal512[ (( index >> 3 ) & 7) + 8 ];
			b = pal512[ (( index >> 6 ) & 7) + 16 ];
			hBrush = CreateSolidBrush( RGB( r, g, b ) );
			FillRect( hMemDC, &fill_rc, hBrush );
			DeleteObject( hBrush );
			index++;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーバーの表示内容を更新する
//	2.	引数
//		hWnd	.....	(I)	ウィンドウハンドル
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void update_color_bar_screen8( HWND hWnd, RECT &rc ){
	HBRUSH hBrush;
	int x, y, index;
	int r, g, b;
	RECT fill_rc;

	index = 0;
	for( y = 0; y < 16; y++ ){
		for( x = 0; x < 16; x++ ){
			fill_rc.left = x * 16;
			fill_rc.top = y * 12;
			fill_rc.right = fill_rc.left + 16;
			fill_rc.bottom = fill_rc.top + 12;
			convert_xy( fill_rc.left, fill_rc.top );
			convert_xy( fill_rc.right, fill_rc.bottom );
			r = screen8[ index & 7 ];
			g = screen8[ (( index >> 3 ) & 7) + 8 ];
			b = screen8[ (( index >> 6 ) & 3) + 16 ];
			hBrush = CreateSolidBrush( RGB( r, g, b ) );
			FillRect( hMemDC, &fill_rc, hBrush );
			DeleteObject( hBrush );
			index++;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーバーの表示内容を更新する
//	2.	引数
//		hWnd	.....	(I)	ウィンドウハンドル
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void update_color_bar_screen12( HWND hWnd, RECT &rc ){
	HBRUSH hBrush;
	int x, y;
	int r, g, b;
	RECT fill_rc;

	for( y = 0; y < 8; y++ ){
		for( x = 0; x < 32; x++ ){
			fill_rc.left = x * 8;
			fill_rc.top = y * 24;
			fill_rc.right = fill_rc.left + 8;
			fill_rc.bottom = fill_rc.top + 24;
			convert_xy( fill_rc.left, fill_rc.top );
			convert_xy( fill_rc.right, fill_rc.bottom );
			r = screen12[  x * ( y       & 1 )       ];
			g = screen12[ (x * ((y >> 1) & 1 )) + 32 ];
			b = screen12[ (x * ((y >> 2) & 1 )) + 64 ];
			hBrush = CreateSolidBrush( RGB( r, g, b ) );
			FillRect( hMemDC, &fill_rc, hBrush );
			DeleteObject( hBrush );
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーバーの表示内容を更新する
//	2.	引数
//		hWnd	.....	(I)	ウィンドウハンドル
//		index ...	(I) SCREEN MODE のインデックス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void update_color_bar( HWND hWnd, int index ){
	HWND hColorBar;
	RECT rc;
	HBRUSH hBrush;

	hColorBar = GetDlgItem( hWnd, IDC_COLORBAR );

	//	まず全体を黒で塗りつぶす
	hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );
	GetClientRect( hColorBar, &rc );
	FillRect( hMemDC, &rc, hBrush );
	DeleteObject( hBrush );

	switch( index ){
	default:
	case 0:
		update_color_bar_pal512( hColorBar, rc );
		break;
	case 1:
		update_color_bar_screen8( hColorBar, rc );
		break;
	case 2:
		update_color_bar_screen12( hColorBar, rc );
		break;
	}
	InvalidateRect( hColorBar, NULL, FALSE );
}

// -------------------------------------------------------------
//	1.	日本語名
//		要素のコンボボックスの内容を更新する
//	2.	引数
//		hWnd	.....	(I)	ウィンドウハンドル
//		index ...	(I) SCREEN MODE のインデックス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void update_cmb_element( HWND hWnd, int index ){

	switch( index ){
	default:
	case 0:
		ComboBox_Init( hWnd, IDC_CMB_ELEMENT, p_element_pal512, sizeof( p_element_pal512 ) / sizeof( p_element_pal512[ 0 ] ), 0 );
		break;
	case 1:
		ComboBox_Init( hWnd, IDC_CMB_ELEMENT, p_element_screen8, sizeof( p_element_screen8 ) / sizeof( p_element_screen8[ 0 ] ), 0 );
		break;
	case 2:
		ComboBox_Init( hWnd, IDC_CMB_ELEMENT, p_element_screen12, sizeof( p_element_screen12 ) / sizeof( p_element_screen12[ 0 ] ), 0 );
		break;
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーバー表示の内容を更新する
//	2.	引数
//		hWnd	.....	(I)	ウィンドウハンドル
//		index ...	(I) SCREEN MODE のインデックス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onColorBarPaint ){
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd, &ps );

	BitBlt( hDC, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY );

	EndPaint( hWnd, &ps );
	return 0;
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
WINPROC( SubColorBarProc ){
	switch( msg ){
		ONEVENT( WM_PAINT	, onColorBarPaint );
	}
	return CallWindowProc( ColorBarProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
//	1.	日本語名
//		現在の設定をロード
//	2.	引数
//		なし
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
static void load_palette_data( void ){

	memcpy( pal512 +  0, convert7to255_r, sizeof( convert7to255_r ) );
	memcpy( pal512 +  8, convert7to255_g, sizeof( convert7to255_g ) );
	memcpy( pal512 + 16, convert7to255_b, sizeof( convert7to255_b ) );

	memcpy( screen8 + 0, convert7to255_s8r, sizeof( convert7to255_s8r ) );
	memcpy( screen8 + 8, convert7to255_s8g, sizeof( convert7to255_s8g ) );
	memcpy( screen8 + 16, convert3to255_s8b, sizeof( convert3to255_s8b ) );

	memcpy( screen12 +  0, convert31to255_s12r, sizeof( convert31to255_s12r ) );
	memcpy( screen12 + 32, convert31to255_s12g, sizeof( convert31to255_s12g ) );
	memcpy( screen12 + 64, convert31to255_s12b, sizeof( convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		初期値をロード
//	2.	引数
//		なし
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
static void load_initial_data( void ){

	memcpy( pal512 + 0, init_convert7to255_r, sizeof( init_convert7to255_r ) );
	memcpy( pal512 + 8, init_convert7to255_g, sizeof( init_convert7to255_g ) );
	memcpy( pal512 + 16, init_convert7to255_b, sizeof( init_convert7to255_b ) );

	memcpy( screen8 + 0, init_convert7to255_s8r, sizeof( init_convert7to255_s8r ) );
	memcpy( screen8 + 8, init_convert7to255_s8g, sizeof( init_convert7to255_s8g ) );
	memcpy( screen8 + 16, init_convert3to255_s8b, sizeof( init_convert3to255_s8b ) );

	memcpy( screen12 + 0, init_convert31to255_s12r, sizeof( init_convert31to255_s12r ) );
	memcpy( screen12 + 32, init_convert31to255_s12g, sizeof( init_convert31to255_s12g ) );
	memcpy( screen12 + 64, init_convert31to255_s12b, sizeof( init_convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		現在の設定をロード
//	2.	引数
//		なし
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
static void save_palette_data( void ){

	memcpy( convert7to255_r, pal512 + 0, sizeof( convert7to255_r ) );
	memcpy( convert7to255_g, pal512 + 8, sizeof( convert7to255_g ) );
	memcpy( convert7to255_b, pal512 + 16, sizeof( convert7to255_b ) );

	memcpy( convert7to255_s8r, screen8 + 0, sizeof( convert7to255_s8r ) );
	memcpy( convert7to255_s8g, screen8 + 8, sizeof( convert7to255_s8g ) );
	memcpy( convert3to255_s8b, screen8 + 16, sizeof( convert3to255_s8b ) );

	memcpy( convert31to255_s12r, screen12 + 0, sizeof( convert31to255_s12r ) );
	memcpy( convert31to255_s12g, screen12 + 32, sizeof( convert31to255_s12g ) );
	memcpy( convert31to255_s12b, screen12 + 64, sizeof( convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		値調整スライダー用の値表示更新
//	2.	引数
//		hWnd .... (I) ウィンドウハンドル
//		Value ... (I) 値
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void set_adjust_value_text( HWND hWnd, int Value ){
	char	szBuf[ 32 ];

	wsprintf( szBuf, "%d", Value );
	SetDlgItemText( hWnd, IDC_VALUE, szBuf );
}

// -------------------------------------------------------------
//	1.	日本語名
//		値調整スライダー用の値表示更新
//	2.	引数
//		hWnd .... (I) ウィンドウハンドル
//		Value ... (I) 値
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void update_adjust_value( HWND hWnd ){
	int mode, pos, index;

	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );
	index = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_ELEMENT ) );

	switch( mode ){
	default:
	case 0:
		pos = pal512[ index ];
		break;
	case 1:
		pos = screen8[ index ];
		break;
	case 2:
		pos = screen12[ index ];
		break;
	}
	set_adjust_value_text( hWnd, pos );
	SetSliderValue( GetDlgItem( hWnd, IDC_ADJUSTER ), pos );
}

// -------------------------------------------------------------
//	1.	日本語名
//		元に戻すボタン
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onBtnUndo ){
	int mode;

	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );
	load_palette_data();
	update_cmb_element( hWnd, mode );
	update_color_bar( hWnd, mode );
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		初期値に戻すボタン
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onBtnLoadDefault ){
	int mode;

	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );
	load_initial_data();
	update_cmb_element( hWnd, mode );
	update_color_bar( hWnd, mode );
	update_adjust_value( hWnd );
	return TRUE;
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
EVENT( onCalibrationInitDlg ) {
	HDC		hDC;
	RECT	rc;

	//	設定値をコピーする
	load_palette_data();

	//	カラーパレットのサブクラス化とビットマップの作成
	HWND hColorBar = GetDlgItem( hWnd, IDC_COLORBAR );
	GetClientRect( hColorBar, &rc );
	w = rc.right;
	h = rc.bottom;

	hDC = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hDC );
	hMemBM = CreateCompatibleBitmap( hDC, w, h );
	if( hMemDC == NULL || hMemBM == NULL ) return TRUE;
	SelectBrush( hMemDC, GetStockBrush( NULL_BRUSH ) );
	SelectBitmap( hMemDC, hMemBM );

	ColorBarProc = (WNDPROC)GetWindowLong( hColorBar, GWL_WNDPROC );
	SetWindowLong( hColorBar, GWL_WNDPROC, (LONG)SubColorBarProc );

	SetSliderRange( GetDlgItem( hWnd, IDC_ADJUSTER ), 0, 255 );

	ComboBox_Init( hWnd, IDC_CMB_SCREEN_MODE, p_screen_mode_name, sizeof(p_screen_mode_name)/sizeof( p_screen_mode_name[0]), 0 );
	update_color_bar( hWnd, 0 );
	update_cmb_element( hWnd, 0 );
	update_adjust_value( hWnd );
	return TRUE;	// 自動で最初のフォーカスを当ててもらう
}

// -------------------------------------------------------------
//	1.	日本語名
//		ウィンドウ閉じる / ボタン：OK
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onCalibrationOk ){

	// 設定した階調情報を採用する
	save_palette_data();

	// リソースの解放
	if( hMemDC != NULL ) DeleteDC( hMemDC );
	if( hMemBM != NULL ) DeleteBitmap( hMemBM );
	hMemDC = NULL;
	hMemBM = NULL;
	EndDialog( hWnd, 0 );
	return TRUE;
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
EVENT( onCalibrationClose ) {

	// リソースの解放
	if( hMemDC != NULL ) DeleteDC( hMemDC );
	if( hMemBM != NULL ) DeleteBitmap( hMemBM );
	hMemDC = NULL;
	hMemBM = NULL;
	EndDialog( hWnd, -1 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		SCREEN MODE コンボボックスが選択された
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onCmbScreenMode ) {
	int mode;

	if( HIWORD( wp ) != CBN_SELCHANGE ) return TRUE;
	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );

	update_color_bar( hWnd, mode );
	update_cmb_element( hWnd, mode );
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		SCREEN MODE コンボボックスが選択された
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onCmbElement ){

	if( HIWORD( wp ) != CBN_SELCHANGE ) return TRUE;
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	日本語名
//		値調整スライダー
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onCalHScroll ){
	HWND		hCol = (HWND)lp;

	if( GetDlgCtrlID( hCol ) != IDC_ADJUSTER ){
		return TRUE;
	}

	int pos = SendMessage( hCol, TBM_GETPOS, 0, 0 );
	set_adjust_value_text( hWnd, pos );

	int mode = wu_combo_get_index( hWnd, IDC_CMB_SCREEN_MODE );
	int index = wu_combo_get_index( hWnd, IDC_CMB_ELEMENT );
	switch( mode ){
	default:
	case 0:
		pal512[ index ] = pos;
		break;
	case 1:
		screen8[ index ] = pos;
		break;
	case 2:
		screen12[ index ] = pos;
		break;
	}
	update_color_bar( hWnd, mode );
	InvalidateRect( GetDlgItem( hWnd, IDC_COLORBAR ), NULL, FALSE );
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
EVENT( onCalibrationCommand ) {
	switch( LOWORD( wp ) ) {
	ONEVENT( IDOK					, onCalibrationOk		);
	ONEVENT( IDCANCEL				, onCalibrationClose	);
	ONEVENT( IDC_CMB_SCREEN_MODE	, onCmbScreenMode		);
	ONEVENT( IDC_CMB_ELEMENT		, onCmbElement			);
	ONEVENT( IDC_BTN_LOAD_DEFAULT	, onBtnLoadDefault		);
	ONEVENT( IDC_BTN_UNDO			, onBtnUndo				);
	}
	return TRUE;
}
