// -----------------------------------------------------
//	TITLE		:	Converter for Windows
//	FILE		:	winConvert.cpp
//	COPYRIGHT	:	(C)2022 t.hara
//	COMENT		:	Windows 用の変換補助艦数群
//	HISTORY		:	2022/08/01	v1.00	初版 Convert.cpp から分離
// -----------------------------------------------------

#include "winConvert.h"

// -----------------------------------------------------
//	1.	日本語名
//		ライン数の取得
//	2.	引数
//		Mode	...	(I)	画像の設定
//	3.	返値
//		ライン数
//	4.	備考
//		なし
// -----------------------------------------------------
static int _get_height( const SETTING *Mode ){
	if( Mode->b192 ){
		return 192;
	}
	else{
		return 212;
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN2)の作成
//	2.	引数
//		bmp	...	(I)	ＶＲＡＭイメージ
//		hDC	...	(I)	プレビュー出力デバイス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _draw_screen2( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r[2] = { 0 },g[2] = { 0 },b[2] = { 0 };
	int x, y, adr, i, c, bit;

	for( y = 0; y < 384; y += 2 ){
		for( x = 0; x < 512; x += 16 ){
			adr = y / 2;
			adr = ( adr >> 3 ) * 256 + ( x / 16 ) * 8 + ( adr & 7 );
			// 下位４ビット（バックグランド）
			r[ 0 ] = convert7to255_r[ Mode->color_palette[ bmp[ adr + SC2COLOR ] & 0x0F ].red ];
			g[ 0 ] = convert7to255_g[ Mode->color_palette[ bmp[ adr + SC2COLOR ] & 0x0F ].green ];
			b[ 0 ] = convert7to255_b[ Mode->color_palette[ bmp[ adr + SC2COLOR ] & 0x0F ].blue ];
			// 上位４ビット（フォアグランド）
			r[ 1 ] = convert7to255_r[ Mode->color_palette[ bmp[ adr + SC2COLOR ] >> 4 ].red ];
			g[ 1 ] = convert7to255_g[ Mode->color_palette[ bmp[ adr + SC2COLOR ] >> 4 ].green ];
			b[ 1 ] = convert7to255_b[ Mode->color_palette[ bmp[ adr + SC2COLOR ] >> 4 ].blue ];

			//	８ドットペアの描画
			c = bmp[ adr ];
			for( i = 0; i < 8; ++i ){
				bit = ( c & 0x80 ) >> 7;
				SetPixel( hDC, x + i * 2 + 0, y + 20, GET_RGB( r[ bit ], g[ bit ], b[ bit ] ) );
				SetPixel( hDC, x + i * 2 + 1, y + 20, GET_RGB( r[ bit ], g[ bit ], b[ bit ] ) );
				SetPixel( hDC, x + i * 2 + 0, y + 21, GET_RGB( r[ bit ], g[ bit ], b[ bit ] ) );
				SetPixel( hDC, x + i * 2 + 1, y + 21, GET_RGB( r[ bit ], g[ bit ], b[ bit ] ) );
				c = c << 1;
			}

		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN3)の作成
//	2.	引数
//		bmp	...	(I)	ＶＲＡＭイメージ
//		hDC	...	(I)	プレビュー出力デバイス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _draw_screen3( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,i;

	for( y = 0; y < 384; ++y ){
		for( x = 0; x < 512; x += 16 ){
			adr = y / 8;
			adr = ( adr >> 3 ) * 256 + ( x / 16 ) * 8 + ( adr & 7 );
			// 上位４ビット（左側）
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] >> 4 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] >> 4 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] >> 4 ].blue ];
			for( i = 0; i < 8; ++i ){
				SetPixel( hDC, x + i, y + 20, GET_RGB( r, g, b ) );
			}
			// 下位４ビット（右側）
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x0F ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x0F ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x0F ].blue ];
			for( i = 0; i < 8; ++i ){
				SetPixel( hDC, x + i + 8, y + 20, GET_RGB( r, g, b ) );
			}

		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN5)の作成
//	2.	引数
//		bmp	...	(I)	ＶＲＡＭイメージ
//		hDC	...	(I)	プレビュー出力デバイス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _draw_screen5( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 4 ){
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 128 + x / 4;
			// 上位４ビット（左側）
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] >> 4 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] >> 4 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] >> 4 ].blue ];
			SetPixel( hDC, x + 0, y + t, GET_RGB( r, g, b ) );
			SetPixel( hDC, x + 1, y + t, GET_RGB( r, g, b ) );
			// 下位４ビット（右側）
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x0F ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x0F ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x0F ].blue ];
			SetPixel( hDC, x + 2, y + t, GET_RGB( r, g, b ) );
			SetPixel( hDC, x + 3, y + t, GET_RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN6)の作成
//	2.	引数
//		bmp	...	(I)	ＶＲＡＭイメージ
//		hDC	...	(I)	プレビュー出力デバイス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _draw_screen6( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 4 ){
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 128 + x / 4;
			// 上位２ビット（左側）
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] >> 6 ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] >> 6 ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] >> 6 ) & 0x03 ].blue ];
			SetPixel( hDC, x + 0, y + t, GET_RGB( r, g, b ) );
			// 上位２ビット（中左側）
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] >> 4 ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] >> 4 ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] >> 4 ) & 0x03 ].blue ];
			SetPixel( hDC, x + 1, y + t, GET_RGB( r, g, b ) );
			// 下位２ビット（中右側）
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] >> 2 ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] >> 2 ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] >> 2 ) & 0x03 ].blue ];
			SetPixel( hDC, x + 2, y + t, GET_RGB( r, g, b ) );
			// 下位２ビット（右側）
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] ) & 0x03 ].blue ];
			SetPixel( hDC, x + 3, y + t, GET_RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN7)の作成
//	2.	引数
//		bmp	...	(I)	ＶＲＡＭイメージ
//		hDC	...	(I)	プレビュー出力デバイス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _draw_screen7( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 2 ){
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			// 上位４ビット（左側）
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] >> 4 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] >> 4 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] >> 4 ].blue ];
			SetPixel( hDC, x + 0, y + t, GET_RGB( r, g, b ) );
			// 下位４ビット（右側）
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x0F ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x0F ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x0F ].blue ];
			SetPixel( hDC, x + 1, y + t, GET_RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN8)の作成
//	2.	引数
//		bmp	...	(I)	ＶＲＡＭイメージ
//		hDC	...	(I)	プレビュー出力デバイス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _draw_screen8( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 2 ){
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r = convert7to255_s8r[(bmp[ adr ] & 0x1C) >> 2 ];
			g = convert7to255_s8g[(bmp[ adr ] & 0xE0) >> 5 ];
			b = convert3to255_s8b[ bmp[ adr ] & 0x03       ];
			SetPixel( hDC, x + 0, y+t, GET_RGB( r, g, b ) );
			SetPixel( hDC, x + 1, y+t, GET_RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビュ(SCREEN10/12)の作成
//	2.	引数
//		bmp		...	(I)	ＶＲＡＭイメージ
//		hDC		...	(I)	プレビュー出力先デバイス
//	3.	返値
//		なし
//	4.	備考
//		※ＶＲＡＭイメージはＭＳＸ上の構造とは異なる
// -------------------------------------------------------------
static void _draw_screen12( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b,j,k,yy[4] = { 0 };
	int x,y,z,adr,t,h;

	h = _get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 8 ){
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			//	輝度成分の取得
			for( z = 0; z < 4; ++z ){
				yy[ z ] = ( bmp[ adr + z ] & 0xF8 ) >> 3;
			}
			//	色成分の取得
			k = ( bmp[ adr + 0 ] & 0x07 ) | ( ( bmp[ adr + 1 ] & 0x07 ) << 3 );
			if( k > 31 ) k = k - 64;	//	5bit の２の歩数
			j = ( bmp[ adr + 2 ] & 0x07 ) | ( ( bmp[ adr + 3 ] & 0x07 ) << 3 );
			if( j > 31 ) j = j - 64;	//	5bit の２の歩数
			// 描画
			for( z = 0; z < 4; ++z ){
				r = range_limiter( yy[ z ] + j							, 0, 31 )* 255 / 31;
				g = range_limiter( yy[ z ] + k							, 0, 31 )* 255 / 31;
				b = range_limiter( 5 * yy[ z ] / 4 - j / 2 - k / 4		, 0, 31 )* 255 / 31;
				if( r == 246 && g == 246 && b == 255 ) {
					r = r;
				}
				SetPixel( hDC, x + z * 2 + 0, y + t, GET_RGB( r, g, b ) );
				SetPixel( hDC, x + z * 2 + 1, y + t, GET_RGB( r, g, b ) );
			}
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ＶＲＡＭイメージからプレビューの作成
//	2.	引数
//		bmp		...	(I)	ＶＲＡＭイメージ
//		hDC		...	(I)	プレビュー出力デバイス
//		Mode	...	(I)	画像の設定
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void draw_screen( const unsigned char *bmp, HDC hDC, const SETTING *Mode ){
	RECT	rc = { 0, 0, 512, 424 };

	FillRect( hDC, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

	switch( Mode->screen_mode ){
	case MD_SC2:
		_draw_screen2( bmp, hDC, Mode );	return;
	case MD_SC3:
		_draw_screen3( bmp, hDC, Mode );	return;
	case MD_SC5:
	case MD_SC5_256L:
		_draw_screen5( bmp, hDC, Mode );	return;
	case MD_SC6:
	case MD_SC6_256L:
		_draw_screen6( bmp, hDC, Mode );	return;
	case MD_SC7:
	case MD_SC7_256L:
		_draw_screen7( bmp, hDC, Mode );	return;
	case MD_SC8:
	case MD_SC8_256L:
		_draw_screen8( bmp, hDC, Mode );	return;
	case MD_SC10:
	case MD_SC12:
	case MD_SC10_256L:
	case MD_SC12_256L:
		_draw_screen12( bmp, hDC, Mode );	return;
	}
}
