// -------------------------------------------------------------
//	susie plug in インターフェースルーチン
//	(C)2001 t.hara
// -------------------------------------------------------------

// -------------------------------------------------------------
//	インクルード

#include <windows.h>
#include <string.h>
#include "splug.h"

// -------------------------------------------------------------
//	設定

// -------------------------------------------------------------
//	グローバル

static int		spicount = 0;				//	対応形式認識数
static SPINAME	spiname[ SPI_COUNT ];		//	プラグインファイル名
static char		spifilter[ SPI_MAXLEN ];	//	ファイルフィルタ一覧

// -------------------------------------------------------------
//	関数プロトタイプ

static bool spi_add_string( char* buf, const char* str, int size );

// -------------------------------------------------------------
//	関数定義

// -------------------------------------------------------------
//	1.	日本語名
//		プラグインの認識
//	2.	引数
//		s_path	...	(I)	PlugIn フォルダの存在するパスにあるファイルのパス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void spi_initialize( const char* s_path )
{
	char				path[ MAX_PATH ], *ptr;
	HANDLE				hFind;
	WIN32_FIND_DATA		ff;
	SPIIF				spiif;
	BOOL				bOut;
	char				s_filter[ 128 ];
	char				s_type[ 128 ];
	char				s_folder[ MAX_PATH ];
	int					i;

	//	変数のクリア
	spicount = 0;
	memset( spiname, 0, sizeof( spiname ) );
	memset( spifilter, 0, sizeof( spifilter ) );

	//	プラグインフォルダパスの作成
	strcpy( path, s_path );
	ptr = strrchr( path, '\\' );
	if( ptr == NULL ) return;
	*ptr = '\0';
	strcpy( s_folder, path );
	strcpy( ptr, "\\*.spi" );

	//	検索
	memset( &ff, 0, sizeof( ff ) );
	ff.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	hFind = FindFirstFile( path, &ff );
	if( hFind == NULL ) return;

	do {
		bOut = TRUE;
		//	アタッチ
		wsprintf( path, "%s\\%s", s_folder, ff.cFileName );
		if( spi_load( path, &spiif ) ) {

			//	対応形式を全て登録
			i = 0;
			do {
				if( (spiif.GetPluginInfo( 2*i+2, s_filter, sizeof( s_filter	) ) == 0) ||
					(spiif.GetPluginInfo( 2*i+3, s_type	, sizeof( s_type	) ) == 0) ) break;
				s_filter[ sizeof( s_filter ) - 1 ] = '\0';
				s_type[ sizeof( s_type ) - 1 ] = '\0';
				if( !spi_add_string( spifilter, s_type	, sizeof( spifilter	) ) || 
					!spi_add_string( spifilter, s_filter, sizeof( spifilter	) ) ) break;
				if( i == 0 ) strcpy( spiname[ spicount ].name, path );
				++spicount;
				++i;
			} while( spicount < SPI_COUNT );

			//	デタッチ
			spi_release( &spiif );
		}

		//	次のファイル
		bOut = bOut && FindNextFile( hFind, &ff );
	} while( bOut && (spicount < SPI_COUNT) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		プラグインにアタッチする
//	2.	引数
//		s_path	...	(I)	プラグインファイル名
//		p_if	...	(I)	インターフェース変数のアドレス
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool spi_load( const char* s_path, SPIIF *p_if )
{
	char	s_buf[8];

	//	プラグインの読み込み
	p_if->hSpi = LoadLibrary( s_path );
	if( p_if->hSpi == NULL ) return false;	//	DLL じゃない

	//	関数アドレスの取得
	p_if->GetPluginInfo		= (SPI_GetPluginInfo) GetProcAddress( p_if->hSpi, "GetPluginInfo" );
	p_if->GetPicture		= (SPI_GetPicture)	GetProcAddress( p_if->hSpi, "GetPicture" );
	p_if->IsSupported		= (SPI_IsSupported)	GetProcAddress( p_if->hSpi, "IsSupported" );

	//	エラーチェック
	if( p_if->GetPicture == NULL || p_if->GetPluginInfo == NULL || p_if->IsSupported == NULL ) {
		FreeLibrary( p_if->hSpi );
		p_if->hSpi = NULL;
		return false;
	}

	//	バージョンチェック
	if( (p_if->GetPluginInfo)( 0, s_buf, sizeof( s_buf ) ) < 4 ) {
		FreeLibrary( p_if->hSpi );
		p_if->hSpi = NULL;
		return false;
	}
	if( strcmp( s_buf, "00IN" ) ) {
		FreeLibrary( p_if->hSpi );
		p_if->hSpi = NULL;
		return false;
	}

	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		プラグインをデタッチする
//	2.	引数
//		p_if	...	(O)	インターフェース変数のアドレス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void spi_release( SPIIF *p_if )
{
	if( p_if->hSpi != NULL ) {
		FreeLibrary( p_if->hSpi );
	}

	memset( p_if, 0, sizeof( SPIIF ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		文字列の追加
//	2.	引数
//		buf		...	(O)	追加先
//		str		...	(I)	追加する文字列
//		size	...	(I)	buf のサイズ
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		buf の未使用部分はあらかじめ 0 で埋めておくこと
// -------------------------------------------------------------
static bool spi_add_string( char* buf, const char* str, int size )
{
	char	*ptr;

	//	buf の尻尾を検出する
	ptr = buf;
	--size;		//	最後の '\0' の分
	while( *ptr != '\0' ) {
		size -= strlen( ptr ) + 1;
		ptr  += strlen( ptr ) + 1;
	}

	//	文字列を追加
	if( (signed)strlen( str ) > size ) return false;
	strcpy( ptr, str );
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		フィルタを返す
//	2.	引数
//		なし
//	3.	返値
//		フィルタ文字列
//	4.	備考
//		なし
// -------------------------------------------------------------
const char* spi_get_filter( void )
{
	return spifilter;
}

// -------------------------------------------------------------
//	1.	日本語名
//		画像を読み込む
//	2.	引数
//		cszFileName			...	(I)	ファイル名
//		ProgressCallback	...	(I)	経過表示関数
//	3.	返値
//		!NULL	...	BMPファイルイメージポインタ
//		NULL	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
char* spi_load_image( const char* cszFileName, FARPROC ProgressCallback )
{
	int					i, n, w;
	SPIIF				sif;
	HLOCAL				pBmp, pInfo;
	char*				pImage;
	BITMAPINFOHEADER	*bi;
	int					c;

	//	プラグインを順番に呼び出す
	pBmp		= NULL;
	pInfo		= NULL;
	sif.hSpi	= NULL;
	for( i = 0; i < spicount; ++i ) {
		//	空
		if( spiname[i].name[0] == '\0' ) continue;
		//	プラグインをアタッチ
		if( !spi_load( spiname[i].name, &sif ) ) continue;
		//	画像を読み込む
		n = (sif.GetPicture)( (LPSTR)cszFileName, 0, 0, &pInfo, &pBmp, ProgressCallback, 0 );
		//	読み込めたか判定
		if( n == 0 ) break;
		//	プラグインをデタッチ
		spi_release( &sif );
	}

	//	読み込めなかった場合
	if( pBmp == NULL || pInfo == NULL ) {
		if( pInfo != NULL ) LocalFree( pInfo );
		if( pBmp  != NULL ) LocalFree( pBmp  );
		spi_release( &sif );
		return NULL;
	}

	//	読み込めた場合
	bi = (BITMAPINFOHEADER*) LocalLock( pInfo );

	//	画像の幅に対応するバイト数 と RGBQUAD の数
	switch( bi->biBitCount ) {
	case 1:
		w = ( (bi->biWidth + 7) / 8 + 3 ) & ~3;
		c = 2;		//	パレット
		break;
	case 4:
		w = ( (bi->biWidth + 1) / 2 + 3 ) & ~3;
		c = 16;		//	パレット
		break;
	case 8:
		w = ( bi->biWidth + 3 ) & ~3;
		c = 256;	//	パレット
		break;
	case 16:
		w = ( bi->biWidth * 2 + 3 ) & ~3;
		c = 3;		//	ビットマスク
		break;
	case 24:
		w = ( bi->biWidth * 3 + 3 ) & ~3;
		c = 0;		//	なし
		break;
	case 32:
		w = bi->biWidth * 4;
		c = 3;		//	ビットマスク
		break;
	}
	c = bi->biSize + c * sizeof( RGBQUAD );

	//	メモリを確保
	pImage = (char*) LocalAlloc( LPTR, c + w * bi->biHeight );
	if( pImage == NULL ) {
		LocalFree( pInfo );
		LocalFree( pBmp  );
		spi_release( &sif );
		return NULL;
	}

	//	メモリへコピー
	memcpy( pImage    , bi, c );
	memcpy( pImage + c, LocalLock( pBmp ), w * bi->biHeight );

	//	終了
	LocalFree( pInfo );
	LocalFree( pBmp  );
	spi_release( &sif );
	return pImage;
}
