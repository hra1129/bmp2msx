// -------------------------------------------------------------
// BMP to MSX
// オリジナル画像表示ウィンドウ
//											(C)2000 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include "Config.h"
#include "winEvent.h"
#include "Resource.h"
#include "SpitProc.h"

// -------------------------------------------------------------
// プロトタイプ宣言

// 一般イベント
EVENT( onSubClose		);	// 閉じる
EVENT( onSubCreate		);	// 生成
EVENT( onSubDestroy		);	// 廃棄
EVENT( onSubPaint		);	// 描画
EVENT( onSubCommand		);	// コマンド分岐
EVENT( onSubMouseMove	);	// マウス移動
EVENT( onSubLButton		);	// 左クリック
EVENT( onSubSize		);	// サイズ変更

// メニューイベント
EVENT( onSubSingle		);	// 等倍表示
EVENT( onSubDouble		);	// ２倍表示

// -------------------------------------------------------------
// グローバル変数

static HBITMAP	hMemBM	= NULL;
static HDC		hMemDC	= NULL;
static int w;						// 画像幅
static int h;						// 画像高さ
static int ww;						// 等倍の時のウィンドウ幅
static int wh;						// 等倍の時のウィンドウ高さ
extern HWND			hOrgWnd;		// main.cpp
extern HINSTANCE	hIns;			// main.cpp

// =====================================================
// ユーティリティ

// -----------------------------------------------------
//	1.	日本語名
//		オリジナル画像ダイアログの表示
//	2.	引数
//		bmp	...	(I)	画像イメージ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void swShow( const char *bmp )
{
	BITMAPINFOHEADER *bih;
	const char *bits;
	RECT r;

	bih = (BITMAPINFOHEADER*)bmp;
	bits = bmp + bih->biSize;
	if( bih->biClrUsed ){
		bits+=bih->biClrUsed * sizeof(RGBQUAD);
	}else if( bih->biBitCount <= 8 ){
		bits+=(1 << bih->biBitCount) * sizeof(RGBQUAD);
	}
	// サイズの取得
	r.top = r.left = 0;
	r.right = w = bih->biWidth;
	r.bottom= h = bih->biHeight;
	AdjustWindowRect( &r,CNWSS,TRUE );
	ww = r.right-r.left;
	wh = r.bottom-r.top;
	SetWindowPos( hOrgWnd,NULL,0,0,ww,wh,SWP_NOMOVE | SWP_NOZORDER );	
	// ビットマップを作成する
	HDC	hDC;
	HBITMAP hBM;
	hDC = GetDC( hOrgWnd );
	hBM = CreateCompatibleBitmap( hDC,w,h );
	ReleaseDC( hOrgWnd,hDC );
	if( hBM==NULL ) return;
	SelectBitmap( hMemDC,hBM );
	if( hMemBM ) DeleteBitmap( hMemBM );
	hMemBM = hBM;
	// ビットマップに画像を描画
	StretchDIBits( hMemDC,0,0,w,h,0,0,w,h,bits,(BITMAPINFO*)bih,DIB_RGB_COLORS,SRCCOPY );
	ShowWindow( hOrgWnd,SW_SHOW );
}

// =====================================================

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
WINPROC( SubProc )
{
	switch( msg ){
	ONEVENT( WM_CLOSE		,onSubClose		);
	ONEVENT( WM_CREATE		,onSubCreate	);
	ONEVENT( WM_DESTROY		,onSubDestroy	);
	ONEVENT( WM_PAINT		,onSubPaint		);
	ONEVENT( WM_COMMAND		,onSubCommand	);
	ONEVENT( WM_MOUSEMOVE	,onSubMouseMove	);
	ONEVENT( WM_LBUTTONDOWN	,onSubLButton	);
	ONEVENT( WM_SIZE		,onSubSize		);	
	}
	return DEFPROC();
}

// =====================================================
// 一般イベント

// -------------------------------------------------------------
//	1.	日本語名
//		閉じる
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubClose )
{
	ShowWindow( hWnd,SW_HIDE );
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		生成
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubCreate )
{
	HDC	hDC;
	hDC = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hDC );
	ReleaseDC( hWnd,hDC );
	if( hMemDC == NULL ) return -1;
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		廃棄
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubDestroy )
{
	if( hMemDC!=NULL ) DeleteDC( hMemDC );
	if( hMemBM!=NULL ) DeleteBitmap( hMemBM );
	hMemDC = NULL;
	hMemBM = NULL;
	return 0;
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
EVENT( onSubPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		r;

	hDC = BeginPaint( hWnd,&ps );
	GetClientRect( hWnd,&r );
	SetStretchBltMode( hDC,COLORONCOLOR );
	StretchBlt( hDC,0,0,r.right,r.bottom,hMemDC,0,0,w,h,SRCCOPY );
	EndPaint( hWnd,&ps );
	return 0;
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
EVENT( onSubCommand )
{
	switch( LOWORD( wp ) ){
	// 機能メニュー
	ONEVENT( IDM_EXIT		, onSubClose		);	// 閉じる
	ONEVENT( IDM_SINGLE		, onSubSingle		);	// 等倍表示
	ONEVENT( IDM_DOUBLE		, onSubDouble		);	// ２倍表示
	}
	return DEFPROC();
}

// -------------------------------------------------------------
//	1.	日本語名
//		マウス移動
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubMouseMove )
{
	// カーソル形状
	SetCursor( LoadCursor( hIns,MAKEINTRESOURCE( IDC_SPOIT ) ) );
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		左クリック
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubLButton )
{
	SPOITDATA sd;
	POINT pos;
	HDC	hDC;
	// スポイト
	GetCursorPos( &pos );
	ScreenToClient( hWnd,&pos );
	hDC = GetDC( hWnd );
	sd.x			= pos.x;
	sd.y			= pos.y;
	sd.colorcode	= GetPixel( hDC,pos.x,pos.y );
	sd.red			= GET_RED( sd.colorcode );
	sd.green		= GET_GREEN( sd.colorcode );
	sd.blue			= GET_BLUE( sd.colorcode );
	ReleaseDC( hWnd,hDC );
	spShow( hWnd,&sd );
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		サイズ変更
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubSize )
{
	InvalidateRect( hWnd,NULL,FALSE );
	return 0;
}

// =====================================================
// メニューイベント

// -------------------------------------------------------------
//	1.	日本語名
//		等倍表示
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubSingle		)
{
	SetWindowPos( hOrgWnd,NULL,0,0,ww,wh,SWP_NOMOVE | SWP_NOZORDER );	
	return 0;
}

// -------------------------------------------------------------
//	1.	日本語名
//		２倍表示
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -------------------------------------------------------------
EVENT( onSubDouble		)
{
	SetWindowPos( hOrgWnd,NULL,0,0,ww+w,wh+h,SWP_NOMOVE | SWP_NOZORDER );	
	return 0;
}
