// -------------------------------------------------------------
//	susie plug in インターフェースルーチン
//	(C)2001 t.hara
// -------------------------------------------------------------

#ifndef	_SPLUG_H_

// -------------------------------------------------------------
//	インクルード

#include <windows.h>

// -------------------------------------------------------------
//	設定

#define	SPI_COUNT	64		//	最大プラグイン数
#define	SPI_MAXLEN	3120	//	フィルタ・形式名バッファのサイズ

// -------------------------------------------------------------
//	型

typedef struct {
	char	name[ MAX_PATH ];
} SPINAME;

typedef	int ( WINAPI *SPI_GetPluginInfo )( int infono, LPSTR buf, int buflen );
typedef int ( WINAPI *SPI_IsSupported )( LPSTR filename, DWORD dw );
typedef int ( WINAPI *SPI_GetPicture )( LPSTR buf, long len, unsigned int flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC lpPrgressCallback, long lData );

typedef struct {
	HINSTANCE			hSpi;
	SPI_GetPluginInfo	GetPluginInfo;
	SPI_IsSupported		IsSupported;
	SPI_GetPicture		GetPicture;
} SPIIF;

// -------------------------------------------------------------
//	関数プロトタイプ

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
void spi_initialize( const char* s_path );

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
bool spi_load( const char* s_path, SPIIF *p_if );

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
void spi_release( SPIIF *p_if );

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
const char* spi_get_filter( void );

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
char* spi_load_image( const char* cszFileName, FARPROC ProgressCallback );

#endif	//	_SPLUG_H_
