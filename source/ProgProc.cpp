// -------------------------------------------------------------
// BMP to MSX
// 経過表示プログレスバーダイアログ
//											(C)2000 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include "ProgProc.h"
#include "winuty.h"
#include "resource.h"

// -------------------------------------------------------------
//	変数

static int	nCount = 0;		//	表示カウンタ
static HWND	hProg  = NULL;	//	ダイアログハンドル

// -------------------------------------------------------------
// プロトタイプ

WINPROC( ProgProc );
EVENT( onPrgInitDlg		);

// -------------------------------------------------------------
//	関数定義

// -----------------------------------------------------
//	1.	日本語名
//		プログレスバーダイアログの表示
//	2.	引数
//		cszText	...	(I)	表示する文字列
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void prShow( const char* cszText )
{
	if( hProg == NULL ) {
		hProg = CreateDialog( wu_get_instance(), MAKEINTRESOURCE( IDD_PROGRESS ), NULL, (DLGPROC)ProgProc );
		ShowWindow( hProg,SW_SHOW );
	}
	SendMessage( GetDlgItem( hProg, IDC_PROGRESS ), PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
	wu_move_center( hProg, NULL );
	prSetText( cszText );
	++nCount;
}

// -----------------------------------------------------
//	1.	日本語名
//		テキストの変更
//	2.	引数
//		cszText	...	(I)	表示する文字列
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void prSetText( const char* cszText )
{
	if( hProg != NULL ) {
		SetDlgItemText( hProg, IDC_FILENAME, cszText );
		SendMessage( GetDlgItem( hProg, IDC_PROGRESS), PBM_SETPOS, 0, 0 );
		SetFocus( hProg );
		InvalidateRect( GetDlgItem( hProg, IDC_FILENAME), NULL, TRUE );
	}
}

// -----------------------------------------------------
//	1.	日本語名
//		プログレスバー表示の廃棄
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void prDestroy( void )
{
	--nCount;
	if( nCount < 1 ) {
		if( hProg != NULL ) DestroyWindow( hProg );
		hProg = NULL;
		nCount = 0;
	}
}

// -----------------------------------------------------
//	1.	日本語名
//		経過表示（コールバック用関数）
//	2.	引数
//		p		...	(I)	0～100 による経過の具合
//	3.	返値
//		true	...	続行
//		false	...	中止
//	4.	備考
//		変換処理のコールバックとして指定する。
// -----------------------------------------------------
bool prProg( int p )
{
	SendMessage( GetDlgItem(hProg,IDC_PROGRESS),PBM_SETPOS,p,0 );
	return true;
}

// -----------------------------------------------------
//	1.	日本語名
//		イベント分岐
//	2.	引数
//		hWnd	...	(I)	ウィンドウ関数
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		イベント処理関数に依存
//	4.	備考
//		なし
// -----------------------------------------------------
WINPROC( ProgProc )
{
	switch( msg )
	{
	ONEVENT( WM_INITDIALOG	, onPrgInitDlg		);
	}
	return FALSE;
}

// =====================================================
// イベント

// -----------------------------------------------------
//	1.	日本語名
//		初期化イベント
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		何もしない
// -----------------------------------------------------
EVENT( onPrgInitDlg )
{
	return TRUE;
}
