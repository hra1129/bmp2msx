// -----------------------------------------------------
// BMP to MSX
// スポイトダイアログ
//											(C)2000 HRA!
// -----------------------------------------------------

#include "SpitProc.h"
#include "winEvent.h"
#include "Resource.h"
#include "winuty.h"
#include "convert.h"

// -----------------------------------------------------
// プロトタイプ宣言

// 一般イベント
EVENT( onSpInitDlg		);
EVENT( onSpClose		);
EVENT( onSpCommand		);
EVENT( onSpPaint		);

// -----------------------------------------------------
// グローバル変数

static SPOITDATA	*sd;
static int	r, g, b;			//	MSX色

// =====================================================
// イベント分岐

// -----------------------------------------------------
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
// -----------------------------------------------------
WINPROC( SpitProc )
{
	switch( msg ){
	ONEVENT( WM_INITDIALOG	,onSpInitDlg	);
	ONEVENT( WM_CLOSE		,onSpClose		);
	ONEVENT( WM_COMMAND		,onSpCommand	);
	ONEVENT( WM_PAINT		,onSpPaint		);
	}
	return FALSE;
}

// =====================================================
// 一般イベント

// -----------------------------------------------------
//	1.	日本語名
//		初期化
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理した
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onSpInitDlg )
{
	char szBuf[ 256 ];
	// 表示を更新する
	SetDlgItemInt( hWnd,IDC_RED			,sd->red		,FALSE );
	SetDlgItemInt( hWnd,IDC_GREEN		,sd->green		,FALSE );
	SetDlgItemInt( hWnd,IDC_BLUE		,sd->blue		,FALSE );
	r = (sd->red	+ 18 ) * 7 / 255;
	g = (sd->green	+ 18 ) * 7 / 255;
	b = (sd->blue	+ 18 ) * 7 / 255;
	SetDlgItemInt( hWnd,IDC_MSXRED		,r ,FALSE );
	SetDlgItemInt( hWnd,IDC_MSXGREEN	,g ,FALSE );
	SetDlgItemInt( hWnd,IDC_MSXBLUE		,b ,FALSE );
	wsprintf( szBuf,"%08XH",sd->colorcode );
	SetDlgItemText( hWnd,IDC_COLORCODE	,szBuf );
	wsprintf( szBuf,"(%d,%d)",sd->x,sd->y );	
	SetDlgItemText( hWnd,IDC_ZAHYO		,szBuf );
	return TRUE;
}

// -----------------------------------------------------
//	1.	日本語名
//		閉じる
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理した
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onSpClose )
{
	EndDialog( hWnd,0 );
	return TRUE;
}

// -----------------------------------------------------
//	1.	日本語名
//		コマンド分岐
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理した
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onSpCommand )
{
	switch( LOWORD( wp ) ){
	ONEVENT( IDOK		, onSpClose		);	// 閉じる
	}
	return TRUE;
}

// -----------------------------------------------------
//	1.	日本語名
//		画面の描画
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理した
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onSpPaint )
{
	RECT	rc;
	HWND	hCol;
	HBRUSH	hBr;
	PAINTSTRUCT	ps;
	HDC	hDC;

	//	サンプル色
	hCol = GetDlgItem( hWnd,IDC_COLOR );
	hDC = GetDC( hCol );
	GetClientRect( hCol,&rc );
	hBr = CreateSolidBrush( sd->colorcode );
	FillRect( hDC,&rc,hBr );
	DeleteBrush( hBr );
	ReleaseDC( hCol,hDC );

	//	ＭＳＸ色
	hCol = GetDlgItem( hWnd,IDC_MSXCOLOR );
	hDC = GetDC( hCol );
	GetClientRect( hCol,&rc );
	hBr = CreateSolidBrush( RGB( convert7to255[r], convert7to255[g], convert7to255[b] ) );
	FillRect( hDC,&rc,hBr );
	DeleteBrush( hBr );
	ReleaseDC( hCol,hDC );

	hDC = BeginPaint( hWnd,&ps );
	EndPaint( hWnd,&ps );

	return TRUE;
}

// =====================================================
// ユーティリティ

// -----------------------------------------------------
//	1.	日本語名
//		ダイアログの表示
//	2.	引数
//		hWnd	...	(I)	親ウィンドウハンドル
//		pdata	...	(I)	スポイト情報データのアドレス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void spShow( HWND hWnd,SPOITDATA *pdata )
{
	sd = pdata;
	DialogBox( wu_get_instance(), MAKEINTRESOURCE( IDD_DOTVIEW ), hWnd, (DLGPROC)SpitProc );
}
