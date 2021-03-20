// -----------------------------------------------------
// BMP to MSX
// プラグインダイアログ
//										(C)2001/5/11 HRA!
// -----------------------------------------------------

#include <windowsx.h>

#include "PlugProc.h"
#include "Resource.h"
#include "config.h"
#include "winEvent.h"
#include "winuty.h"

// -----------------------------------------------------
// プロトタイプ

WINPROC( PlugProc );

EVENT( onPlugClose		);
EVENT( onPlugCommand	);
EVENT( onPlugInitDlg	);
EVENT( onPlugOk			);
EVENT( onPlugPaint		);
EVENT( onPlugFolder		);

// -----------------------------------------------------
// グローバル変数

static int		g_nMax	= 0;
static char*	g_pText	= NULL;

// -----------------------------------------------------
//	関数定義

// -----------------------------------------------------
//	1.	日本語名
//		ダイアログの表示
//	2.	引数
//		hWnd	...	(I)	親ウィンドウ
//		pText	...	(O)	パスを格納するアドレス
//		nMax	...	(I)	pText のサイズ
//	3.	返値
//		-1	...	更新無し
//		0	...	更新あり
//	4.	備考
//		キャンセルが押されると pText の内容は変化しない
// -----------------------------------------------------
int piShow( HWND hWnd, char *pText, int nMax )
{
	int i;

	g_nMax	= nMax;
	g_pText	= pText;
	i = DialogBox( wu_get_instance(), MAKEINTRESOURCE( IDD_PLUGIN ), hWnd, (DLGPROC)PlugProc );
	g_nMax	= 0;
	return i;
}

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ関数
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		msg		...	(I)	メッセージ
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理済み
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
WINPROC( PlugProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onPlugClose	);
		ONEVENT( WM_COMMAND		, onPlugCommand	);
		ONEVENT( WM_INITDIALOG	, onPlugInitDlg	);
		ONEVENT( WM_PAINT		, onPlugPaint	);
	}
	return FALSE;
}

// =====================================================
// イベント

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ閉じる
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理済み
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugClose )
{
	EndDialog( hWnd, -1 );
	return TRUE;
}

// -----------------------------------------------------
//	1.	日本語名
//		コマンド
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理済み
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugCommand )
{
	switch( LOWORD( wp ) )
	{
	ONEVENT( IDOK		, onPlugOk			);		// [OK]
	ONEVENT( IDCANCEL	, onPlugClose		);		// [Cancel]
	ONEVENT( IDC_FOLDER	, onPlugFolder		);		// [...]
	}
	return TRUE;
}

// -----------------------------------------------------
//	1.	日本語名
//		ダイアログの初期化
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理済み
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugInitDlg )
{
	//	パスを更新する
	SetDlgItemText( hWnd, IDC_PATH, g_pText );
	return TRUE;	// 自動で最初のフォーカスを当ててもらう
}

// -----------------------------------------------------
//	1.	日本語名
//		ＯＫボタン
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugOk )
{
	//	パスを更新する
	GetDlgItemText( hWnd, IDC_PATH, g_pText, g_nMax );
	EndDialog( hWnd, 0 );
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
//		TRUE	...	処理済み
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd,&ps );
	EndPaint( hWnd,&ps );
	return TRUE;
}

// -----------------------------------------------------
//	1.	日本語名
//		フォルダの選択
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		wp		...	(I)	Ｗパラメータ
//		lp		...	(I)	Ｌパラメータ
//	3.	返値
//		TRUE	...	処理済み
//		FALSE	...	未処理
//	4.	備考
//		なし
// -----------------------------------------------------
EVENT( onPlugFolder )
{
	char	szFileName[ MAX_PATH ];
	char	*ptr;
	OPENFILENAME	ofn;

	// ファイル名を入力
	szFileName[0]='\0';
	ZeroMemory( &ofn,sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= wu_get_instance();
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefPlug;
#ifdef _ENGLISH
	ofn.lpstrTitle		= "PlugIn Folder";
#else
	ofn.lpstrTitle		= "ﾌﾟﾗｸﾞｲﾝﾌｫﾙﾀﾞ";
#endif
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= g_pText;
	if( !GetOpenFileName( &ofn ) ) return 0;

	//	パスの設定
	ptr = strrchr( szFileName, '\\' );
	if( ptr != NULL ) {
		ptr[1] = '\0';
	}
	SetDlgItemText( hWnd, IDC_PATH, szFileName );

	return 0;
}
