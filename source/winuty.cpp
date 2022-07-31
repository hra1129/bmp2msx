// -----------------------------------------------------
//	TITLE		:	Windows Utility
//	FILE		:	winuty.cpp
//	COPYRIGHT	:	(C)2001 t.hara
//	COMENT		:	Windows 補助関数群
//	HISTORY		:	2001/02/11	v0.00	製作開始
// -----------------------------------------------------

// =====================================================
//	インクルード

#include "winuty.h"
#include <windowsx.h>

// =====================================================
//	設定

#define	WU_DEF_FILTER	"All files(*.*)\0*.*\0"

// =====================================================
//	変数

static HINSTANCE	h_ins = NULL;								//	アプリケーションインスタンスハンドル

// =====================================================
//	関数定義

// =====================================================
//	アプリケーション標準

// -----------------------------------------------------
//	1.	日本語名
//		インスタンスハンドルを登録する
//	2.	引数
//		h_ins	...	(I)	インスタンスハンドル
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_set_instance( HINSTANCE h_ins )
{
	::h_ins = h_ins;
}

// -----------------------------------------------------
//	1.	日本語名
//		インスタンスハンドルを取得する
//	2.	引数
//		なし
//	3.	返値
//		登録したインスタンスハンドル
//	4.	備考
//		なし
// -----------------------------------------------------
HINSTANCE wu_get_instance( void )
{
	return ::h_ins;
}

// -----------------------------------------------------
//	1.	日本語名
//		メッセージ処理
//	2.	引数
//		なし
//	3.	返値
//		true	...	正常
//		false	...	アプリケーション終了要求
//	4.	備考
//		なし
// -----------------------------------------------------
bool wu_do_events( HWND h_wnd, HACCEL h_accel )
{
	MSG	msg;

	//	メッセージが無ければ CPU を解放して待機する
	if( !PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ){
		WaitMessage();
		return true;
	}

	//	メッセージが WM_QUIT なら終了要求を返す
	if( !GetMessage( &msg, NULL, 0, 0 ) ) return false;

	//	アクセラレータが指定されていれば処理する
	if( h_wnd != NULL && h_accel != NULL ) {
		if( TranslateAccelerator( h_wnd, h_accel, &msg ) ) return true;
	}

	//	トランスレートとディスパッチ
	TranslateMessage( &msg );
	DispatchMessage( &msg );

	return true;
}

// -----------------------------------------------------
//	1.	日本語名
//		一般的なウィンドウクラスの設定
//	2.	引数
//		p_wndclass	...	(O)	ウィンドウクラス構造体
//		p_class		...	(I)	ウィンドウクラス名
//		n_icon		...	(I)	アイコンＩＤ
//		p_wndclass	...	(I)	ウィンドウ関数
//	3.	返値
//		なし
//	4.	備考
//		この関数を使う前に wu_set_instance でインスタンスハンドル
//		を指定しておかねばならない
// -----------------------------------------------------
void wu_get_wndclassex( WNDCLASSEX* p_wndclass, const char* p_class, int n_icon, WNDPROC p_wndproc )
{
	WNDCLASSEX*	&wc = p_wndclass;

	memset( wc, 0, sizeof( WNDCLASSEX ) );
	wc->cbSize			= sizeof( WNDCLASSEX );
	wc->hbrBackground	= ( HBRUSH ) GetStockObject( WHITE_BRUSH );
	wc->hCursor			= LoadCursor( NULL, IDC_ARROW );
	if( n_icon != NULL ) {
		wc->hIcon		= LoadIcon( h_ins, MAKEINTRESOURCE( n_icon ) );
		wc->hIconSm		= ( HICON ) LoadImage( h_ins, MAKEINTRESOURCE( n_icon ), 
							IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	} else {
		wc->hIcon		= LoadIcon( NULL, IDI_APPLICATION );
		wc->hIconSm		= LoadIcon( NULL, IDI_APPLICATION );
	}
	wc->hInstance		= h_ins;
	wc->lpfnWndProc		= p_wndproc;
	wc->lpszClassName	= p_class;
	wc->lpszMenuName	= NULL;
	wc->style			= 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウ設定
//	2.	引数
//		*p_create		...	(O)	ウィンドウ生成構造体
//		*p_class		...	(I)	ウィンドウクラス名
//		*p_caption		...	(I)	ウィンドウタイトル
//		n_width			...	(I)	ウィンドウ幅
//									0 ... 省略と見なしデフォルト値を設定する
//		n_height		...	(I)	ウィンドウ高さ
//									0 ... 省略と見なしデフォルト値を設定する
//	3.	返値
//		なし
//	4.	備考
//		ウィンドウ位置は画面の中央付近に設定される
// -----------------------------------------------------
void wu_get_createstruct( CREATESTRUCT* p_create,const char* p_class, const char* p_caption, int n_width, int n_height )
{
	RECT			r_size;
	CREATESTRUCT*	&c	= p_create;

	memset( p_create, 0, sizeof( CREATESTRUCT ) );

	c->lpszClass		= p_class;
	c->lpszName			= p_caption;

	GetClientRect( GetDesktopWindow(), &r_size );

	if( n_width ){
		c->cx			= n_width;
	}else{
		c->cx			= 100;
	}

	if( n_height ){
		c->cy			= n_height;
	}else{
		c->cy			= 100;
	}

	c->x				= ( r_size.right  - c->cx ) / 2;
	c->y				= ( r_size.bottom - c->cy ) / 2;
	c->hMenu			= NULL;
	c->hwndParent		= NULL;
	c->dwExStyle		= 0;
	c->style			= WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX;
	c->hInstance		= h_ins;
	c->lpCreateParams	= NULL;
}

// -----------------------------------------------------
//	1.	日本語名
//		ウィンドウの作成
//	2.	引数
//		p_create	...	(I)	ウィンドウ生成パラメータ
//	3.	返値
//		NULL		...	ウィンドウ生成失敗
//		他			...	生成したウィンドウのウィンドウハンドル
//	4.	備考
//		ウィンドウ生成が失敗する要因は次の２種類
//			(1)	ウィンドウ生成パラメータに不備がある場合
//			(2)	WM_CREATE イベントで -1 が返された場合
//		この関数の呼び出しのみではこれらを区別できない。関数呼び
//		出し前にグローバル変数に規定値を格納し、WM_CREATE の処理
//		先頭にて別の規定値を格納する。そのグローバル変数を参照す
//		ることで区別することができる。
// -----------------------------------------------------
HWND wu_create_window( CREATESTRUCT* p_create )
{
	return( CreateWindowEx( p_create->dwExStyle, p_create->lpszClass, p_create->lpszName,
				p_create->style, p_create->x, p_create->y, p_create->cx, p_create->cy,
				p_create->hwndParent, p_create->hMenu, p_create->hInstance, p_create->lpCreateParams ) );
}

// -----------------------------------------------------
//	1.	日本語名
//		実行ファイルの存在するディレクトリを取得する
//	2.	引数
//		p_buffer	...	(O)	パスを出力するバッファのアドレス
//	3.	返値
//		なし
//	4.	備考
//		バッファは MAX_PATH のサイズだけ確保しておけば十分
// -----------------------------------------------------
void wu_get_exe_path( char* p_buffer )
{
	char		*p_path;

	p_path	= GetCommandLine();
	lstrcpy( p_buffer, p_path );
	p_path	= strrchr( p_buffer, '\\' );
	*p_path	= '\0';
}

// -----------------------------------------------------
//	1.	日本語名
//		ファイルを開くダイアログ表示（単独ファイル）
//	2.	引数
//		h_wnd		...	(I)		親ウィンドウハンドル
//		p_name		...	(I/O)	デフォルトファイル
//								選択されたファイルが格納される。
//		p_filter	...	(I/O)	拡張子
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		p_name は MAX_PATH のサイズある必要がある
// -----------------------------------------------------
bool wu_dialog_select_open_file( HWND h_wnd, char* p_name, const char* p_filter )
{
	OPENFILENAME	ofn;

	memset( &ofn, 0, sizeof( ofn ) );
	ofn.hInstance	= h_ins;
	ofn.hwndOwner	= h_wnd;
	ofn.lStructSize	= sizeof( ofn );
	if( p_filter != NULL ) {
		ofn.lpstrFilter	= p_filter;
	} else {
		ofn.lpstrFilter	= WU_DEF_FILTER;
	}
	ofn.Flags		= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER
					| OFN_ENABLESIZING;
	ofn.lpstrFile	= p_name;
	ofn.nMaxFile	= MAX_PATH;
	ofn.nFilterIndex= 1;
	return( GetOpenFileName( &ofn ) != FALSE );
}

// -----------------------------------------------------
//	1.	日本語名
//		ファイルを開くダイアログ表示（複数ファイル）
//	2.	引数
//		h_wnd		...	(I)		親ウィンドウハンドル
//		p_name		...	(I/O)	デフォルトファイル
//								選択されたファイルが格納される。
//		p_filter	...	(I/O)	拡張子
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		p_name は MAX_PATH のサイズある必要がある
// -----------------------------------------------------
bool wu_dialog_multi_select_open_file( HWND h_wnd, char* p_name, const char* p_filter )
{
	OPENFILENAME	ofn;

	memset( &ofn, 0, sizeof( ofn ) );
	ofn.hInstance	= h_ins;
	ofn.hwndOwner	= h_wnd;
	ofn.lStructSize	= sizeof( ofn );
	if( p_filter != NULL ) {
		ofn.lpstrFilter	= p_filter;
	} else {
		ofn.lpstrFilter	= WU_DEF_FILTER;
	}
	ofn.Flags		= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER
					| OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.lpstrFile	= p_name;
	ofn.nMaxFile	= MAX_PATH;
	ofn.nFilterIndex= 1;
	return( GetOpenFileName( &ofn ) != FALSE );
}

// =====================================================
//	ウィンドウ汎用操作関数

// -----------------------------------------------------
//	1.	日本語名
//		クライアント領域のサイズ指定
//	2.	引数
//		h_wnd		...	(I)	ウィンドウハンドル
//		n_width		...	(I)	幅
//		n_height	...	(I)	高さ
//	3.	返値
//		なし
//	4.	備考
//		数回の試行後、設定不能ならあきらめる
// -----------------------------------------------------
void wu_set_client_rect( HWND h_wnd, int n_width, int n_height )
{
	RECT	r_size, r_winsize;
	int		i;

	for( i = 0; i < 3; ++i ) {
		//	サイズを取得
		GetClientRect( h_wnd, &r_size );
		GetWindowRect( h_wnd, &r_winsize );

		//	サイズの比較
		if( r_size.right == n_width && r_size.bottom == n_height ) break;

		//	サイズの調節
		r_size.right	= r_winsize.right  + ( n_width  - r_size.right  ) - r_winsize.left;
		r_size.bottom	= r_winsize.bottom + ( n_height - r_size.bottom ) - r_winsize.top;

		//	ウィンドウサイズの変更
		SetWindowPos( h_wnd, 0, 0, 0, r_size.right, r_size.bottom,
			SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER );
	}
}

// -----------------------------------------------------
//	1. 関数名
//		ウィンドウ を 指定のウィンドウ の中央へ移動する
//	2. 引数
//		h_wnd		...	(I)	対象のウィンドウハンドル
//		h_basewnd	...	(I)	基準のウィンドウハンドル
//	3. 返値
//		なし
//	4. 備考
//		基準と対象のウィンドウの親子関係などは問わない
//		基準を NULL にするとデスクトップを基準にする
// -----------------------------------------------------
void wu_move_center( HWND h_wnd, HWND h_basewnd )
{
	RECT	r_wsize, r_bsize;
	HWND	h_pwnd;
	POINT	p;

	// 基準ウィンドウが未指定の場合はデスクトップを基準にする
	if( h_basewnd == NULL ) h_basewnd = GetDesktopWindow();

	// ウィンドウの位置とサイズを求める
	memset( &r_wsize, 0, sizeof( RECT ) );
	memset( &r_bsize, 0, sizeof( RECT ) );
	GetWindowRect( h_wnd,		&r_wsize );
	GetWindowRect( h_basewnd,	&r_bsize );

	// 中央位置を算出する(スクリーン座標)
	p.x = ( ( r_bsize.left + r_bsize.right  ) - ( r_wsize.right  - r_wsize.left ) ) / 2;	
	p.y = ( ( r_bsize.top  + r_bsize.bottom ) - ( r_wsize.bottom - r_wsize.top  ) ) / 2;
	
	// ウィンドウを移動する
	h_pwnd = GetParent( h_wnd );
	
	if( h_pwnd != NULL && (GetWindowLong( h_wnd, GWL_STYLE ) & WS_CHILD) ){

		// 親を持っている場合は親ウィンドウ座標系に変換する
		ScreenToClient( h_pwnd, &p );
	}

	SetWindowPos( h_wnd, 0, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE );
}

// =====================================================
//	リストビュー関連

// -----------------------------------------------------
//	1.	日本語名
//		列の数を取得
//	2.	引数
//		h_wnd		...	(I)	親ウィンドウハンドル
//		n_id		...	(I)	リストビューのＩＤ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
int wu_listview_get_column_count( HWND h_wnd, int n_id )
{
	HWND		h_list;
	LVCOLUMN	lvcolumn;
	int			n_cnt;

	h_list			= GetDlgItem( h_wnd, n_id );
	n_cnt			= 0;
	memset( &lvcolumn, 0, sizeof( lvcolumn ) );
	lvcolumn.mask	= LVCF_WIDTH;
	while( ListView_GetColumn( h_list, n_cnt, &lvcolumn ) ) ++n_cnt;
	return n_cnt;
}

// -----------------------------------------------------
//	1.	日本語名
//		列を追加する
//	2.	引数
//		h_wnd		...	(I)	親ウィンドウハンドル
//		n_id		...	(I)	リストビューのＩＤ
//		s_name		...	(I)	列名
//		n_width		...	(I)	列幅
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_listview_add_column( HWND h_wnd, int n_id, const char* s_name, int n_width )
{
	HWND		h_list;
	LVCOLUMN	lvcolumn;
	int			n_cnt;

	h_list = GetDlgItem( h_wnd, n_id );

	//	列数を取得する（先頭か否か判定するため）
	n_cnt = wu_listview_get_column_count( h_wnd, n_id );

	//	追加する
	memset( &lvcolumn, 0, sizeof( lvcolumn ) );
	lvcolumn.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	if( n_cnt ) lvcolumn.mask |= LVCF_SUBITEM;
	lvcolumn.fmt		= LVCFMT_LEFT;
	lvcolumn.pszText	= (char*)s_name;
	lvcolumn.cx			= n_width;
	lvcolumn.iSubItem	= n_cnt;
	ListView_InsertColumn( h_list, n_cnt, &lvcolumn );
}

// -----------------------------------------------------
//	1.	日本語名
//		項目数を取得
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウ
//		n_id	...	(I)	リストビューのＩＤ
//	3.	返値
//		項目数
//	4.	備考
//		なし
// -----------------------------------------------------
int wu_listview_get_count( HWND h_wnd, int n_id )
{
	HWND		h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	return ListView_GetItemCount( h_list );
}

// -----------------------------------------------------
//	1.	日本語名
//		新しい項目の追加
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウ
//		n_id	...	(I)	リストビューのＩＤ
//		s_name	...	(I)	追加する項目名
//	3.	返値
//		挿入した項目のインデックス（行番号）
//	4.	備考
//		なし
// -----------------------------------------------------
int wu_listview_add_item( HWND h_wnd, int n_id, const char* s_name )
{
	LV_ITEM		lvitem;
	HWND		h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	memset( &lvitem, 0, sizeof( lvitem ) );

	lvitem.mask		= LVIF_TEXT;
	lvitem.pszText	= (char*)s_name;
	lvitem.iItem	= wu_listview_get_count( h_wnd, n_id );
	lvitem.iSubItem	= 0;
	return ListView_InsertItem( h_list, &lvitem );
}

// -----------------------------------------------------
//	1.	日本語名
//		項目のサブアイテムの設定
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	リストビューのＩＤ
//		n_row	...	(I)	サブアイテムの行番号
//		n_col	...	(I)	サブアイテムの列番号
//		s_name	...	(I)	サブアイテムに設定する文字列
//	3.	返値
//		なし
//	4.	備考
//		n_col を 0 にすると項目名も変更できる
//		n_row を -1 にすると最後の行を対象とする
// -----------------------------------------------------
void wu_listview_set_subitem( HWND h_wnd, int n_id, int n_row, int n_col,const char* s_name )
{
	LV_ITEM		lvitem;
	HWND		h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	if( n_row == -1 ) n_row = wu_listview_get_count( h_wnd, n_id )-1;

	memset( &lvitem, 0, sizeof( lvitem ) );

	lvitem.mask		= LVIF_TEXT;
	lvitem.pszText	= (char*)s_name;
	lvitem.iItem	= n_row;
	lvitem.iSubItem	= n_col;
	ListView_SetItem( h_list, &lvitem );
}

// =====================================================
//	コンボボックス関連

// -----------------------------------------------------
//	1.	日本語名
//		コンボボックスに複数の文字列を追加する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	コンボボックスのID
//		s_array	...	(I)	文字列ポインタの配列アドレス
//		n_count	...	(I)	s_array の要素数
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_combo_set_items( HWND h_wnd, int n_id, const char* s_array[], int n_count )
{
	int		i;
	HWND	h_combo;

	h_combo = GetDlgItem( h_wnd, n_id );

	for( i = 0; i < n_count; ++i ) {
		ComboBox_AddString( h_combo, s_array[ i ] );
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		コンボボックスの初期化
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		id		...	(I)	コンボボックスのＩＤ
//		sArray	...	(I)	文字列配列
//		n		...	(I)	文字列配列の要素数
//		index	...	(I)	コンボボックスの初期化インデックス
// -------------------------------------------------------------
void ComboBox_Init( HWND hWnd, int id, const char **sArray, int n, int index ){
	HWND hCmb;
	int i;
	hCmb = GetDlgItem( hWnd, id );
	ComboBox_ResetContent( hCmb );
	for( i = 0; i < n; ++i ){
		ComboBox_AddString( hCmb, sArray[ i ] );
	}
	ComboBox_SetCurSel( hCmb, index );
}

// -----------------------------------------------------
//	1.	日本語名
//		コンボボックスの選択カーソル位置を指定する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	コンボボックスのID
//		n_index	...	(I)	選択位置
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_combo_set_index( HWND h_wnd, int n_id, int n_index )
{
	HWND	h_combo;

	h_combo = GetDlgItem( h_wnd, n_id );
	
	ComboBox_SetCurSel( h_combo, n_index );
}

// -----------------------------------------------------
//	1.	日本語名
//		コンボボックスの選択カーソル位置を取得する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	コンボボックスのID
//	3.	返値
//		インデックス値
//	4.	備考
//		なし
// -----------------------------------------------------
int wu_combo_get_index( HWND h_wnd, int n_id )
{
	HWND	h_combo;

	h_combo = GetDlgItem( h_wnd, n_id );
	
	return ComboBox_GetCurSel( h_combo );
}

// =====================================================
//	ボタン関連

// -----------------------------------------------------
//	1.	日本語名
//		ボタンにアイコンをセットする
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	ボタンのID
//		hIcon	...	(I)	セットするアイコン
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_button_set_icon( HWND h_wnd, int n_id, HICON hIcon )
{
	HWND	h_button;

	h_button = GetDlgItem( h_wnd, n_id );
	
	SendMessage( h_button, BM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon ); 
}

// -----------------------------------------------------
//	1.	日本語名
//		ボタンのチェック状態をセットする
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	ボタンのID
//		b_check	...	(I)	チェック状態
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_button_set_check( HWND h_wnd, int n_id, bool b_check )
{
	HWND	h_button;

	h_button = GetDlgItem( h_wnd, n_id );
	
	SendMessage( h_button, BM_SETCHECK, b_check ? BST_CHECKED : BST_UNCHECKED, 0 ); 
}

// -----------------------------------------------------
//	1.	日本語名
//		ボタンのチェック状態を取得する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	ボタンのID
//	3.	返値
//		true	...	チェック済み
//		false	...	未チェック、グレイ
//	4.	備考
//		なし
// -----------------------------------------------------
bool wu_button_get_check( HWND h_wnd, int n_id )
{
	HWND	h_button;

	h_button = GetDlgItem( h_wnd, n_id );
	
	return( SendMessage( h_button, BM_GETCHECK, 0, 0 ) == BST_CHECKED ); 
}

// =====================================================
//	メニュー関連

// -----------------------------------------------------
//	1.	日本語名
//		メニューのチェック状態を指定する
//	2.	引数
//		h_wnd	...	(I)	メニューを持ったウィンドウハンドル
//		n_id	...	(I)	メニューアイテムID
//		b_check	...	(I)	チェック状態
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_menu_set_check( HWND h_wnd, int n_id, BOOL b_check )
{
	HMENU	h_menu;

	//	メニューの取得
	h_menu = GetMenu( h_wnd );
	if( h_menu == NULL ) return;

	//	チェック状態の設定
	MENUITEMINFO	mi;

	memset( &mi, 0, sizeof( mi ) );
	mi.cbSize	= sizeof( mi );
	GetMenuItemInfo( h_menu, n_id, FALSE, &mi );
	mi.fMask	= MIIM_STATE;
	if( b_check ) {
		mi.fState	= MFS_CHECKED;
	} else {
		mi.fState	= MFS_UNCHECKED;
	}
	SetMenuItemInfo( h_menu, n_id, FALSE, &mi );

	SetMenu( h_wnd, h_menu );
}

// -----------------------------------------------------
//	1.	日本語名
//		メニューの有効無効を設定する
//	2.	引数
//		h_wnd	...	(I)	メニューを持ったウィンドウハンドル
//		n_id	...	(I)	メニューアイテムID
//		b_enable	...	(I)	有効無効状態
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_menu_enabled( HWND h_wnd, int n_id, BOOL b_enable )
{
	HMENU	h_menu;

	//	メニューの取得
	h_menu = GetMenu( h_wnd );
	if( h_menu == NULL ) return;

	//	チェック状態の設定
	MENUITEMINFO	mi;

	memset( &mi, 0, sizeof( mi ) );
	mi.cbSize	= sizeof( mi );
	GetMenuItemInfo( h_menu, n_id, FALSE, &mi );
	mi.fMask	= MIIM_STATE;
	if( b_enable ) {
		mi.fState	= MFS_ENABLED;
	} else {
		mi.fState	= MFS_DISABLED | MFS_GRAYED;
	}
	SetMenuItemInfo( h_menu, n_id, FALSE, &mi );

	SetMenu( h_wnd, h_menu );
}

// =====================================================
//	リストボックス関連

// -----------------------------------------------------
//	1.	日本語名
//		リストボックスに複数の文字列を追加する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	リストボックスのID
//		s_array	...	(I)	文字列ポインタの配列アドレス
//		n_count	...	(I)	s_array の要素数
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_list_set_items( HWND h_wnd, int n_id, const char* s_array[], int n_count )
{
	int		i;
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	for( i = 0; i < n_count; ++i ) {
		ListBox_AddString( h_list, s_array[ i ] );
	}
}

// -----------------------------------------------------
//	1.	日本語名
//		リストボックスに文字列を追加する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	リストボックスのID
//		s_name	...	(I)	文字列
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_list_add_item( HWND h_wnd, int n_id, const char* s_name )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	ListBox_AddString( h_list, s_name );
}

// -----------------------------------------------------
//	1.	日本語名
//		リストボックスを空にする
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	リストボックスのID
//		s_name	...	(I)	文字列
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_list_clear( HWND h_wnd, int n_id )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	ListBox_ResetContent( h_list );
}

// -----------------------------------------------------
//	1.	日本語名
//		リストボックスの選択カーソル位置を指定する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	リストボックスのID
//		n_index	...	(I)	選択位置
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_list_set_index( HWND h_wnd, int n_id, int n_index )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );
	
	ListBox_SetCurSel( h_list, n_index );
}

// -----------------------------------------------------
//	1.	日本語名
//		リストボックスの選択カーソル位置を取得する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	リストボックスのID
//	3.	返値
//		インデックス値
//	4.	備考
//		なし
// -----------------------------------------------------
int wu_list_get_index( HWND h_wnd, int n_id )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );
	
	return ListBox_GetCurSel( h_list );
}

// =====================================================
//	タブコントロール関連

// -----------------------------------------------------
//	1.	日本語名
//		タブコントロールの項目数を取得する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	タブコントロールのID
//	3.	返値
//		項目数
//	4.	備考
//		失敗すると 0 が返る
// -----------------------------------------------------
int wu_tab_get_count( HWND h_wnd, int n_id )
{
	HWND	h_tab;

	h_tab = GetDlgItem( h_wnd, n_id );

	return TabCtrl_GetItemCount( h_tab );
}

// -----------------------------------------------------
//	1.	日本語名
//		タブコントロールに項目を追加する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	タブコントロールのID
//		s_name	...	(I)	追加するタブ名
//	3.	返値
//		-1	...	失敗
//		他	...	新しいタブのインデックス
//	4.	備考
//		登録済みタブの最後に追加する
// -----------------------------------------------------
int wu_tab_add_item( HWND h_wnd, int n_id, const char* s_name )
{
	HWND	h_tab;
	int		n_index;
	TC_ITEM	tc;

	h_tab = GetDlgItem( h_wnd, n_id );
	memset( &tc, 0, sizeof( tc ) );
	tc.mask		= TCIF_TEXT;
	tc.pszText	= (LPSTR)s_name;
	n_index		= wu_tab_get_count( h_wnd, n_id );

	return TabCtrl_InsertItem( h_tab, n_index, &tc );
}

// -----------------------------------------------------
//	1.	日本語名
//		タブコントロールに複数の文字列を追加する
//	2.	引数
//		h_wnd	...	(I)	親ウィンドウハンドル
//		n_id	...	(I)	タブコントロールのID
//		s_array	...	(I)	文字列ポインタの配列アドレス
//		n_count	...	(I)	s_array の要素数
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_tab_set_items( HWND h_wnd, int n_id, const char* s_array[], int n_count )
{
	int		i;

	for( i = 0; i < n_count; ++i ) {
		wu_tab_add_item( h_wnd, n_id, s_array[ i ] );
	}
}

// =====================================================
//	ツールチップ

// -----------------------------------------------------
//	1.	日本語名
//		ツールチップを作成する
//	2.	引数
//		hWnd	...	(I)	親ウィンドウハンドル
//	3.	返値
//		!NULL	...	ツールチップのウィンドウハンドル
//		NULL	...	失敗
//	4.	備考
//		なし
// -----------------------------------------------------
HWND wu_ttip_create( HWND hWnd )
{
    HWND	hToolTip;

	hToolTip = CreateWindowEx(
				0,						//	拡張スタイル
				TOOLTIPS_CLASS,
				NULL,
				TTS_ALWAYSTIP,			//	スタイル
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				hWnd,					// 親
				NULL,
				h_ins,
				NULL );

    SendMessage( hToolTip, TTM_ACTIVATE, (WPARAM)TRUE, 0);
	
	return hToolTip;
}

// -----------------------------------------------------
//	1.	日本語名
//		ツールチップに表示対象を追加
//	2.	引数
//		hTip	...	ツールチップのハンドル
//		nId		...	コントロールのＩＤ
//		s_tips	...	表示する文字列
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void wu_ttip_set_item( HWND hTip, int nId, char* s_tips )
{
    TOOLINFO	ti;
	RECT		rect;
	POINT		pt;
	HWND		hWnd = GetParent( hTip );

	GetWindowRect( GetDlgItem( hWnd, nId ), &rect );

	pt.x = rect.left;
	pt.y = rect.top;
	ScreenToClient( hWnd, &pt );
	ti.rect.left = pt.x;
	ti.rect.top  = pt.y;

	pt.x = rect.right;
	pt.y = rect.bottom;
	ScreenToClient( hWnd, &pt );
	ti.rect.right  = pt.x;
	ti.rect.bottom = pt.y;

	memset( &ti, 0, sizeof( ti ) );
	ti.cbSize	= sizeof( TOOLINFO );
    ti.hwnd		= hWnd;
    ti.hinst	= h_ins;
    ti.uFlags	= TTF_SUBCLASS | TTF_IDISHWND;
	ti.uId		= (WPARAM)GetDlgItem( hWnd, nId );
	ti.lpszText = (LPTSTR)s_tips;
	SendMessage( hTip, TTM_ADDTOOL, 0, (LPARAM)&ti );
}

// -------------------------------------------------------------
//	1.	日本語名
//	スライダ（トラックバー）の最大値と最小値を設定する
//	2.	引数
//		max	...	(I)	最大値
//		min	...	(I)	最小値
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void SetSliderRange( HWND hWnd, short int max, short int min ){
	SendMessage( hWnd, TBM_SETRANGE, TRUE, MAKELONG( max, min ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		スライダ（トラックバー）の位置を変更する
//	2.	引数
//		val	...	(I)	位置
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void SetSliderValue( HWND hWnd, short int val ){
	SendMessage( hWnd, TBM_SETPOS, TRUE, (LONG)val );
}
