// -------------------------------------------------------------
//	TITLE	:	BMP to MSX プレビュー管理関連
//	INPUT	:	HRA!
// -------------------------------------------------------------

// =============================================================
//	インクルード

#include <windows.h>
#include <windowsx.h>

#include "config.h"
#include "convert.h"
#include "PreView.h"
#include "ProgProc.h"
#include "splug.h"
#include "fileuty.h"

// =============================================================
//	関数プロトタイプ

static int _bdAddFile( const char *cszFileName );
static int WINAPI ProgressCallback( int nNum, int nDenom, long lData );

// =============================================================
//	変数

BMPDATA	tBmpview[ PV_MAX ];		// ＢＭＰファイル情報
PVDATA	tPreview[ PV_MAX ];		// プレビュー情報
int		pvIndex = -1;			// 現在表示中のプレビュー
int		nTime	= 0;			// プレビュー登録時刻（古さの決定基準）

// =====================================================
// プレビュー関連

// -----------------------------------------------------
//	1.	日本語名
//		プレビュー変数の初期化
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void pvInitialize( void )
{
	ZeroMemory( tPreview, sizeof( tPreview ) );
	ZeroMemory( tBmpview, sizeof( tBmpview ) );
}

// -----------------------------------------------------
//	1.	日本語名
//		未使用のプレビューを検索する
//	2.	引数
//		なし
//	3.	返値
//		-1	...	空き無し
//		n	...	空きプレビューへのインデックス
//	4.	備考
//		通常、空きが無くなっていた場合はもっとも古い
//		プレビューを削除して空きを作成する。したがって
//		-1 が返るのは特殊な場合のみ。
// -----------------------------------------------------
int pvBlankIndex( void )
{
	int i, j, p;
	j = 0x7FFFFFFF;
	for( i=0; i<PV_MAX; ++i ){
		if( tPreview[i].hBmp == NULL ) return i;
		if( tPreview[i].nTime < j ) {
			j = tPreview[i].nTime;	//	最もふるいプレビュ
			p = i;
		}
	}
	//	空きが無い場合は最もふるいものを削除する
	if( j == 0x7FFFFFFF ) return -1;						//	プレビューデータが壊れてる場合 or ２０億枚以上変換した場合 (^_^;
	pvDelete( p );
	return p;
}

// -----------------------------------------------------
//	1.	日本語名
//		指定のプレビューを削除する
//	2.	引数
//		Index	...	(I)	削除対象のインデックス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void pvDelete( int Index )
{
	if( tPreview[Index].hBmp == NULL ) return;
	DeleteObject( tPreview[Index].hBmpSrc );
	DeleteObject( tPreview[Index].hBmp );
	LocalFree( tPreview[Index].bmp );
	tPreview[Index].hBmpSrc = NULL;
	tPreview[Index].hBmp = NULL;
	tPreview[Index].bmp = NULL;
	tPreview[Index].nTime = 0x7FFFFFFF;
	bdDelete(tPreview[Index].nSrc);
	if( pvIndex == Index ) pvIndex = pvNextPreview( pvIndex );
}

// -----------------------------------------------------
//	1.	日本語名
//		全プレビューを削除する
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void pvAllDelete( void )
{
	int i;
	for( i=0; i<PV_MAX; ++i ){
		pvDelete( i );
	}
	nTime = 0;
}

// -----------------------------------------------------
//	1.	日本語名
//		次のプレビューを検索する
//	2.	引数
//		Index	...	(I)	現在のインデックス
//	3.	返値
//		-1	...	プレビューが空っぽ
//		n	...	次のインデックス
//	4.	備考
//		なし
// -----------------------------------------------------
int pvNextPreview( int Index )
{
	int i;
	Index=(Index+1)%PV_MAX;
	for( i=0; i<PV_MAX; ++i,Index=(Index+1)%PV_MAX ){
		if( tPreview[Index].hBmp != NULL ) return Index;
	}
	return -1;
}

// -----------------------------------------------------
//	1.	日本語名
//		前のプレビューを検索する
//	2.	引数
//		Index	...	(I)	現在のインデックス
//	3.	返値
//		-1	...	プレビューが空っぽ
//		n	...	前のインデックス
//	4.	備考
//		なし
// -----------------------------------------------------
int pvPrevPreview( int Index )
{
	int i;
	Index=(Index-1+PV_MAX)%PV_MAX;
	for( i=0; i<PV_MAX; ++i,Index=(Index-1+PV_MAX)%PV_MAX ){
		if( tPreview[Index].hBmp != NULL ) return Index;
	}
	return -1;
}

// -----------------------------------------------------
//	1.	日本語名
//		プレビューにビットマップを追加する
//	2.	引数
//		Index	...	(I)	空きプレビューのインデックス
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -----------------------------------------------------
bool pvAddPreview( int Index )
{
	HDC hDC;
	if( tPreview[ Index ].hBmp != NULL || Index<0 ) return false;
	hDC = GetDC( NULL );
	tPreview[ Index ].hBmp    = CreateCompatibleBitmap( hDC, cnMAXWidth, cnMAXHeight );
	tPreview[ Index ].hBmpSrc = CreateCompatibleBitmap( hDC, cnMAXWidth, cnMAXHeight );
	ReleaseDC( NULL, hDC );
	if( tPreview[ Index ].hBmp    == NULL ||
		tPreview[ Index ].hBmpSrc == NULL ){	// 作成に失敗したらメモリを解放してエラーを返す
		if( tPreview[ Index ].hBmp    != NULL ) DeleteObject( tPreview[ Index ].hBmp    );
		if( tPreview[ Index ].hBmpSrc != NULL ) DeleteObject( tPreview[ Index ].hBmpSrc );
		tPreview[ Index ].hBmp    = NULL;
		tPreview[ Index ].hBmpSrc = NULL;
		tPreview[ Index ].nTime   = 0x7FFFFFFF;
		return false;
	}
	tPreview[ Index ].nTime = nTime;
	if( nTime < 0x7FFFFFFF ) ++nTime;
	return true;
}

// -----------------------------------------------------
//	1.	日本語名
//		プレビュー（変換元）にビットマップを描画する
//	2.	引数
//		hDC		...	(I)	メモリデバイス（一時的に使用する）
//		Index	...	(I)	プレビューのインデックス
//		in		...	(I)	画像イメージ
//		width	...	(I)	画像イメージの幅
//		height	...	(I)	画像イメージの高さ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -----------------------------------------------------
void pvDrawBmp( HDC hDC, int Index, COLORREF *in, int width, int height )
{
	RECT	r = { 0, 0, cnMAXWidth, cnMAXHeight };
	int x,y,t,h;
	COLORREF c;

	HBITMAP hMemBM = SelectBitmap( hDC,tPreview[Index].hBmpSrc );	// 一時的に仮想画面用メモリＤＣを使用する

	if( (height < 212) || (height == 384) ) {
		t = 212 - 192;
		h = cnMAXHeight - t*2;
		FillRect( hDC, &r, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
		for( y=0; y<h; ++y ){
			for( x=0; x<cnMAXWidth; ++x ){
				c=*( in + x*width/cnMAXWidth + (y*height/h)*width );
				SetPixel( hDC,x,t+y,c );
			}
		}
	} else {
		for( y=0; y<cnMAXHeight; ++y ){
			for( x=0; x<cnMAXWidth; ++x ){
				c=*( in + x*width/cnMAXWidth + (y*height/cnMAXHeight)*width );
				SetPixel( hDC,x,y,c );
			}
		}
	}
	SelectBitmap( hDC,hMemBM );							// 元に戻す
}

// -----------------------------------------------------
//	1.	日本語名
//		プレビュー番号を連番へ変換する
//	2.	引数
//		Index	...	(I)	プレビューのインデックス
//	3.	返値
//		対応する番号
//	4.	備考
//		なし
// -----------------------------------------------------
int pvGetNum( int Index )
{
	int i,r;
	if( Index==-1 ) return -1;
	r=0;
	for( i=0; i<=Index; ++i ){
		if( tPreview[i].bmp != NULL ) ++r;
	}
	return r;
}

// -----------------------------------------------------
//	1.	日本語名
//		プレビューの登録数を取得する
//	2.	引数
//		なし
//	3.	返値
//		登録数
//	4.	備考
//		なし
// -----------------------------------------------------
int pvGetCnt( void )
{
	int i,r;
	r=0;
	for( i=0; i<PV_MAX; ++i ){
		if( tPreview[i].bmp != NULL ) ++r;
	}
	return r;
}

// =====================================================
// 画像ファイルイメージ関連

// -----------------------------------------------------
//	1.	日本語名
//		未使用の画像ファイルイメージを検索する
//	2.	引数
//		なし
//	3.	返値
//		-1	...	空き無し
//		n	...	空きプレビューのインデックス
//	4.	備考
//		なし
// -----------------------------------------------------
int bdBlankIndex( void )
{
	int i;

	//	プレビューに空きが無ければ空きを作る
	pvBlankIndex();

	//	空きを検索する
	for( i=0; i<PV_MAX; ++i ){
		if( tBmpview[i].n == 0 ) return i;
	}
	return -1;
}

// -----------------------------------------------------
//	1.	日本語名
//		指定の画像ファイルイメージを削除する
//	2.	引数
//		Index	...	(I)	削除対象の画像インデックス
//	3.	返値
//		なし
//	4.	備考
//		画像が存在しなくても問題ない。
//		（但し、画像管理配列は初期化済みの場合に限る）
// -----------------------------------------------------
void bdDelete( int Index )
{
	if( tBmpview[Index].n == 0 ) return;
	if( (--tBmpview[Index].n)==0 ){
		LocalFree( tBmpview[Index].image );
	}
}

// -----------------------------------------------------
//	1.	日本語名
//		BMPファイルを画像ファイルイメージに追加する
//	2.	引数
//		cszFileName	...	(I)	ファイル名
//	3.	返値
//		-1	...	追加に失敗
//		n	...	ファイルイメージのインデックス
//	4.	備考
//		画像ファイルイメージの参照カウンタは 1 になる
// -----------------------------------------------------
static int _bdAddFile( const char *cszFileName ) {
	BITMAPFILEHEADER	bfh;
	int					nSize;
	FU_FILE				*hFile;
	int					nIndex;
	char				szBuf[ MAX_PATH ];

	// 画像ファイルイメージの空きを検索
	nIndex = bdBlankIndex();
	if( nIndex == -1 ) return -1;
	// ファイルを開く
	hFile = fu_open( cszFileName, "rb" );
	if( hFile == FU_INVALID_HANDLE ) {
		wsprintf( szBuf,"%s を開けません。",cszFileName );
		MessageBox( NULL,szBuf,MsgCap,MB_OK | MB_ICONWARNING );
		return -1;	// ファイルオープン失敗
	}
	// ファイルサイズ
	nSize = (int)fu_length( hFile )-sizeof(bfh);
	if( nSize < sizeof(BITMAPINFOHEADER) ) goto exit;

		// ファイルヘッダを読み込む
	if( !fu_read( hFile, &bfh, sizeof(bfh) ) ) goto exit;

	// ヘッダを確認する
	if( bfh.bfType != 'MB' ) goto exit;

	// メモリを確保する
	tBmpview[ nIndex ].image = (char*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,nSize );
	if( tBmpview[ nIndex ].image==NULL ) goto exit;

	// ファイルを読み込む
	fu_read( hFile, tBmpview[ nIndex ].image, nSize );
	// 終了
	tBmpview[ nIndex ].n = 1;
	lstrcpy( tBmpview[ nIndex ].sFileName, cszFileName );
	fu_close( hFile );
	return nIndex;
exit:
	fu_close( hFile );
	return -1;
}

// -----------------------------------------------------
//	1.	日本語名
//		画像ファイルを画像ファイルイメージに追加する
//	2.	引数
//		cszFileName	...	(I)	ファイル名
//	3.	返値
//		-1	...	追加に失敗
//		n	...	ファイルイメージのインデックス
//	4.	備考
//		画像ファイルイメージの参照カウンタは 1 になる
// -----------------------------------------------------
int bdAddFile( const char *cszFileName )
{
	int		nIndex;
	bool	bProg = false;
	char	szBuf[ MAX_PATH ];

	//	BMPファイルとして読み込んでみる
	nIndex = _bdAddFile( cszFileName );
	if( nIndex > -1 ) return nIndex;

	// 画像ファイルイメージの空きを検索
	nIndex = bdBlankIndex();
	if( nIndex == -1 ) return -1;

	//	経過表示用ダイアログ表示
	GetFileTitle( cszFileName, szBuf, sizeof( szBuf ) );
	prShow( szBuf );

	//	プラグインに読み込ませる
	tBmpview[ nIndex ].image = spi_load_image( cszFileName, (FARPROC)ProgressCallback );

	//	経過表示用ダイアログ消去
	prDestroy();

	//	エラーチェック
	if( tBmpview[ nIndex ].image == NULL ) return -1;

	// 終了
	tBmpview[ nIndex ].n = 1;
	lstrcpy( tBmpview[ nIndex ].sFileName , cszFileName );

	return nIndex;
}

// -----------------------------------------------------
//	1.	日本語名
//		BMPファイルイメージを画像ファイルイメージに追加する
//	2.	引数
// 		bmp	...	(I)	ＢＭＰファイルイメージ
// 3.	返値
//		-1	...	追加に失敗
//		n	...	ファイルイメージのインデックス
// 4.	備考
//		画像ファイルイメージの参照カウンタは 1 になる
// -----------------------------------------------------
int bdAddImage( char *bmp )
{
	int					nIndex;
	// 画像ファイルイメージの空きを検索
	nIndex = bdBlankIndex();
	if( nIndex == -1 ) return -1;
	// メモリを記憶する
	tBmpview[ nIndex ].image = bmp;
	// 終了
	tBmpview[ nIndex ].n = 1;
	tBmpview[ nIndex ].sFileName[0]='\0';
	return nIndex;
}

// =====================================================
//	コールバック

// -----------------------------------------------------
//	1.	日本語名
//		Susie PlugIn 用の経過表示関数
//	2.	引数
//		nNum	...	(I)	経過
//		nDenom	...	(I)	nNum の終着値
//		lData	...	(I)	アプリ定義の値（未使用）
//	3.	返値
//		0	...	続行
//		!0	...	中止
//	4.	備考
//		prProg を呼び出す I/F 矯正関数
// -----------------------------------------------------
static int WINAPI ProgressCallback( int nNum, int nDenom, long lData )
{
	if( nDenom == 0 ) return 0;
	prProg( nNum * 100 / nDenom );
	return 0;
}
