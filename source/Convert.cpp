// -------------------------------------------------------------
// BMP to MSX
// 変換処理
//										(C)2000/5/6 HRA!
// -------------------------------------------------------------

#include <malloc.h>
#include "Convert.h"
#include "math.h"
#include "fileuty.h"

// -------------------------------------------------------------
// 型
// -------------------------------------------------------------
struct C_COLOR_TABLE {
	C_COLOR				c;		// 色
	int					n;		// その数
};

// -------------------------------------------------------------
// 設定
// -------------------------------------------------------------

const int screen_mode[] = { 2, 3, 5, 6, 7, 8, 10, 12, 5, 6, 7, 8, 10, 12 };	// 画面モード番号

// 画面サイズ
const C_SCREEN_SIZE screen_size[] = {
	{ 256, 192 },	// screen 2/4
	{ 64, 48 },	// screen 3
	{ 256, 212 },	// screen 5
	{ 512, 212 },	// screen 6
	{ 512, 212 },	// screen 7
	{ 256, 212 },	// screen 8
	{ 256, 212 },	// screen 10/11
	{ 256, 212 },	// screen 12
	{ 256, 256 },	// screen 5 (256 lines)
	{ 512, 256 },	// screen 6 (256 lines)
	{ 512, 256 },	// screen 7 (256 lines)
	{ 256, 256 },	// screen 8 (256 lines)
	{ 256, 256 },	// screen 10/11 (256 lines)
	{ 256, 256 },	// screen 12 (256 lines)
};


// ディザリングパターン
const int dith[][8][8]={
	{	// ディザ１（疑似ランダム）
		{0 ,15,7 ,14,6 ,15,5 ,15},
		{13,4 ,9 ,2 ,12,7 ,8 ,2 },
		{6 ,12,1 ,10,7 ,13,4 ,10},
		{11,3 ,8 ,5 ,11,0 ,12,3 },
		{5 ,15,0 ,14,2 ,9 ,1 ,14},
		{9 ,6 ,10,4 ,12,3 ,13,0 },
		{1 ,13,3 ,8 ,5 ,10,4 ,11},
		{14,2 ,9 ,6 ,11,1 ,8 ,7 },
	},{	// ディザ２（タイル）
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{12,7 ,14,5 ,12,7 ,14,5 },
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{12,7 ,14,5 ,12,7 ,14,5 },
	},{	// ディザ３（縦長タイル）
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{12,7 ,14,5 ,12,7 ,14,5 },
		{12,7 ,14,5 ,12,7 ,14,5 },
	},{	// ディザ４（レンガ）
		{0 ,1 ,2 ,3 ,3 ,4 ,5 ,6 },
		{11,12,13,4 ,2 ,13,14,7 },
		{10,15,14,5 ,1 ,12,15,8 },
		{9 ,8 ,7 ,6 ,0 ,11,10,9 },
		{5 ,6 ,0 ,1 ,2 ,3 ,3 ,4 },
		{14,7 ,11,12,13,4 ,2 ,13},
		{15,8 ,10,15,14,5 ,1 ,12},
		{10,9 ,9 ,8 ,7 ,6 ,0 ,11},
	},{	// ディザ５（つぶつぶ）
		{9 ,10,7 ,0 ,5 ,8 ,9 ,7 },
		{11,13,8 ,4 ,6 ,11,13,12},
		{14,15,14,10,11,14,15,14},
		{12,13,11,11,12,12,13,11},
		{9 ,8 ,10,12,13,11,12,10},
		{5 ,4 ,9 ,14,15,14,11,8 },
		{4 ,0 ,6 ,11,13,12,10,7 },
		{8 ,5 ,7 ,5 ,9 ,10,9 ,9 },
	},{	// ディザ６（粗タイル）
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
	},{	// ディザ７（弱タイル）
		{0 ,7 ,1 ,6 ,0 ,7 ,1 ,6 },
		{10,2 ,8 ,4 ,10,2 ,8 ,4 },
		{2 ,5 ,0 ,6 ,2 ,5 ,0 ,6 },
		{8 ,4 ,9 ,3 ,8 ,4 ,9 ,3 },
		{0 ,7 ,1 ,6 ,0 ,7 ,1 ,6 },
		{10,2 ,8 ,4 ,10,2 ,8 ,4 },
		{2 ,5 ,0 ,6 ,2 ,5 ,0 ,6 },
		{8 ,4 ,9 ,3 ,8 ,4 ,9 ,3 },
	},{	// ディザ８（ハーフトーン）
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
	}
};

// MSX階調 → Win階調 ( 0～255 ) 変換テーブル
const int init_convert7to255_r[]={
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert7to255_g[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert7to255_b[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

// MSX階調 SCREEN8 → Win階調 ( 0～255 ) 変換テーブル
const int init_convert7to255_s8r[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert7to255_s8g[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert3to255_s8b[]={
	0, 73, 146, 255
};

// MSX階調 SCREEN12 → Win階調 ( 0～255 ) 変換テーブル
const int init_convert31to255_s12r[] = {
	 0 * 255 / 31,  1 * 255 / 31,  2 * 255 / 31,  3 * 255 / 31,
	 4 * 255 / 31,  5 * 255 / 31,  6 * 255 / 31,  7 * 255 / 31,
	 8 * 255 / 31,  9 * 255 / 31, 10 * 255 / 31, 11 * 255 / 31,
	12 * 255 / 31, 13 * 255 / 31, 14 * 255 / 31, 15 * 255 / 31,
	16 * 255 / 31, 17 * 255 / 31, 18 * 255 / 31, 19 * 255 / 31,
	20 * 255 / 31, 21 * 255 / 31, 22 * 255 / 31, 23 * 255 / 31,
	24 * 255 / 31, 25 * 255 / 31, 26 * 255 / 31, 27 * 255 / 31,
	28 * 255 / 31, 29 * 255 / 31, 30 * 255 / 31, 31 * 255 / 31,
};

const int init_convert31to255_s12g[] = {
	 0 * 255 / 31,  1 * 255 / 31,  2 * 255 / 31,  3 * 255 / 31,
	 4 * 255 / 31,  5 * 255 / 31,  6 * 255 / 31,  7 * 255 / 31,
	 8 * 255 / 31,  9 * 255 / 31, 10 * 255 / 31, 11 * 255 / 31,
	12 * 255 / 31, 13 * 255 / 31, 14 * 255 / 31, 15 * 255 / 31,
	16 * 255 / 31, 17 * 255 / 31, 18 * 255 / 31, 19 * 255 / 31,
	20 * 255 / 31, 21 * 255 / 31, 22 * 255 / 31, 23 * 255 / 31,
	24 * 255 / 31, 25 * 255 / 31, 26 * 255 / 31, 27 * 255 / 31,
	28 * 255 / 31, 29 * 255 / 31, 30 * 255 / 31, 31 * 255 / 31,
};

const int init_convert31to255_s12b[] = {
	 0 * 255 / 31,  1 * 255 / 31,  2 * 255 / 31,  3 * 255 / 31,
	 4 * 255 / 31,  5 * 255 / 31,  6 * 255 / 31,  7 * 255 / 31,
	 8 * 255 / 31,  9 * 255 / 31, 10 * 255 / 31, 11 * 255 / 31,
	12 * 255 / 31, 13 * 255 / 31, 14 * 255 / 31, 15 * 255 / 31,
	16 * 255 / 31, 17 * 255 / 31, 18 * 255 / 31, 19 * 255 / 31,
	20 * 255 / 31, 21 * 255 / 31, 22 * 255 / 31, 23 * 255 / 31,
	24 * 255 / 31, 25 * 255 / 31, 26 * 255 / 31, 27 * 255 / 31,
	28 * 255 / 31, 29 * 255 / 31, 30 * 255 / 31, 31 * 255 / 31,
};

// MSX階調 → Win階調 ( 0～255 ) 変換テーブル
int convert7to255_r[ 8 ];
int convert7to255_g[ 8 ];
int convert7to255_b[ 8 ];

// MSX階調 SCREEN8 → Win階調 ( 0～255 ) 変換テーブル
int convert7to255_s8r[ 8 ];
int convert7to255_s8g[ 8 ];
int convert3to255_s8b[ 4 ];

// MSX階調 SCREEN12 → Win階調 ( 0～255 ) 変換テーブル
int convert31to255_s12r[ 32 ];
int convert31to255_s12g[ 32 ];
int convert31to255_s12b[ 32 ];

//	SCREEN2/3 変換用作業域
static unsigned char vram[ 0x4000 ];

// -------------------------------------------------------------
// プロトタイプ
// -------------------------------------------------------------
static inline C_COLOR _get_pixel( C_COLOR *in, int width, int height, int x, int y );
static inline void _set_pixel( C_COLOR *out, int width, int height, int x, int y, C_COLOR c );
static inline int _abs( int a );
static inline void _put_dither_pattern( int *r, int *g, int *b, int screen_mode, int ErrAdd, int x, int y, C_COLOR c );
static inline int _distance( int r1, int g1, int b1, int r2, int g2, int b2 );
static inline int _distance_itp( int r1, int g1, int b1, int r2, int g2, int b2 );

static int cnvCreateHistgram( C_COLOR *in,int size,C_COLOR_TABLE **tbl,C_COLOR *pal,int pp,
							  bool FourceZero,C_COLOR FZColor );
static bool cnvCompare( C_PALETTE* Pal1, C_PALETTE* Pal2 );

static bool cnvRecolor8( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );
static bool cnvRecolor5( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );

static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, C_COLOR *pal );
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog );

// -----------------------------------------------------
//	1.	日本語名
//		最適化パレットからパレット値を求める
//	2.	引数
//		p_convert_table . (I)	テーブル
//		n ............... (I)	テーブルのサイズ
//		v ............... (I)	最適化パレット (0～255)
//	3.	返値
//		パレット値
//	4.	備考
//		なし
// -----------------------------------------------------
int convert_rgb_to_palette( const int *p_convert_table, int n, int v ){
	int minimum_diff = 999, i, diff;
	int index = 0;

	for( i = 0; i < n; i++ ){
		diff = p_convert_table[ i ] - v;
		if( diff < 0 ){
			diff = -diff;
		}
		if( diff < minimum_diff ){
			index = i;
			minimum_diff = diff;
		}
	}
	return index;
}

// -------------------------------------------------------------
//	1.	日本語名
//		複製ルーチン
//	2.	引数
//		in			...	(I)	入力画像格納メモリのアドレス
//		inwidth		...	(I)	入力画像の幅
//		inheight	...	(I)	入力画像の高さ
//		out			...	(I)	出力画像格納先メモリのアドレス
//		outwidth	...	(I)	出力画像の幅
//		outheight	...	(I)	出力画像の高さ
//		prog		...	(I)	経過表示関数
//		seido		...	(I)	切り捨てるビット数
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool cnvCopy( C_COLOR *in ,int inwidth ,int inheight ,
			  C_COLOR *out,int outwidth,int outheight, PROGRESS prog, int seido )
{
	int	x,y,w;
	int	hh;
	C_COLOR mask;

	// 精度制限用マスク作成
	mask = ~((1<<seido)-1);
	mask = mask & ((mask<<8) | 0xFF) & ((mask<<16) | 0xFFFF);
	// 出力先全ピクセルを舐める
	w = outwidth;
	if( w>inwidth ) w=inwidth;
	if( outheight < inheight ) {
		hh = outheight; 
	} else {
		hh = inheight;
	}
	for( y= 0; y < hh; ++y ){
		if( !prog( y * 100 / outheight ) ) return false;	// 経過を表示
		for( x = 0; x < w; ++x ){
			out[ x ] = in[ x ] & mask;
		}
		out += outwidth;
		in  += inwidth;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		インターレースモード用複製ルーチン
//	2.	引数
//		in			...	(I)	入力画像格納メモリのアドレス
//		inwidth		...	(I)	入力画像の幅
//		inheight	...	(I)	入力画像の高さ
//		out			...	(I)	出力画像格納先メモリのアドレス
//		outwidth	...	(I)	出力画像の幅
//		outheight	...	(I)	出力画像の高さ
//		prog		...	(I)	経過表示関数
//		seido		...	(I)	切り捨てるビット数
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool cnvCopyInter( C_COLOR *in ,int inwidth ,int inheight ,
			  C_COLOR *out,int outwidth,int outheight,PROGRESS prog,int seido )
{
	int	x,y;
	int	ww, hh;
	C_COLOR c1,c2;
	C_COLOR mask;

	if( outheight < inheight ) hh = outheight; else hh = inheight;
	if( outwidth  < inwidth  ) ww = outwidth;  else ww = inwidth / 2;

	// 精度制限用マスク作成
	mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );
	
	// 出力先全ピクセルを舐める
	for( y = 0; y < hh; ++y ){
		if( !prog( y * 100 / outheight ) ) return false;	// 経過を表示
		for( x = 0; x < ww; ++x ){	
			c1=in[ x * 2 + 0];
			c2=in[ x * 2 + 1];
			out[ x ]=GET_RGB(	( GET_RED(   c1 ) + GET_RED(   c2 ) ) / 2,
								( GET_GREEN( c1 ) + GET_GREEN( c2 ) ) / 2,
								( GET_BLUE(  c1 ) + GET_BLUE(  c2 ) ) / 2 ) & mask;
		}
		out += outwidth;
		in  += inwidth;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ノーマル拡大縮小ルーチン
//	2.	引数
//		in			...	(I)	入力画像格納メモリのアドレス
//		inwidth		...	(I)	入力画像の幅
//		inheight	...	(I)	入力画像の高さ
//		out			...	(I)	出力画像格納先メモリのアドレス
//		outwidth	...	(I)	出力画像メモリの幅
//		outheight	...	(I)	出力画像メモリの高さ
//		wwidth		...	(I) 出力する幅
//		wheight		...	(I)	出力する高さ
//		prog		...	(I)	経過表示関数
//		seido		...	(I)	切り捨てるビット数
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		途中経過表示関数が失敗すると中断して失敗を返す
// -------------------------------------------------------------
bool cnvNormResize( C_COLOR *in , int inwidth , int inheight ,
			    C_COLOR *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy;
	C_COLOR	c;
	C_COLOR	mask;

	// 精度制限用マスク作成
	mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	// 入力と出力が同じサイズならそのまま複製
	if( (inwidth == outwidth) && (inheight == outheight) &&
		(inwidth == wwidth  ) && (inheight == wheight  ) ){
		for( x = 0; x < inwidth * inheight; ++x ){
			out[ x ] = in[ x ] & mask;
		}
		return true;
	}

	// 出力先全ピクセルを舐める
	for( y = 0; y < wheight; ++y ){
		//	経過を表示
		if( !prog( y * 100 / wheight ) ) return false;
		//	入力画像の座標へ変換
		yy   = y * inheight / wheight;
		for( x = 0; x < wwidth; ++x ){
			// 入力画像の座標に変換
			xx  = x * inwidth / wwidth;
			c   = _get_pixel( in, inwidth, inheight, xx, yy );
			_set_pixel( out, outwidth, outheight, x, y, GET_RGB(GET_RED( c ), GET_GREEN( c ), GET_BLUE( c )) & mask );
		}	// x
	}	// y
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		アンチエイリアスつき拡大縮小ルーチン
//	2.	引数
//		in			...	(I)	入力画像格納メモリのアドレス
//		inwidth		...	(I)	入力画像の幅
//		inheight	...	(I)	入力画像の高さ
//		out			...	(I)	出力画像格納先メモリのアドレス
//		outwidth	...	(I)	出力画像メモリの幅
//		outheight	...	(I)	出力画像メモリの高さ
//		wwidth		...	(I) 出力する幅
//		wheight		...	(I)	出力する高さ
//		prog		...	(I)	経過表示関数
//		seido		...	(I)	切り捨てるビット数
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		途中経過表示関数が失敗すると中断して失敗を返す
// -------------------------------------------------------------
bool cnvAntiResize( C_COLOR *in , int inwidth , int inheight ,
			    C_COLOR *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy, ix, iy, ixm1, ixm2, iym1, iym2, ixw, iyw;
	float		ax, ay, prx, pgx, pbx, pry, pgy, pby, am;
	C_COLOR	c;
	C_COLOR	mask;
	bool		bWZoom;	//	幅拡大
	bool		bHZoom;	//	高さ拡大

	ax = 0;

	// 精度制限用マスク作成
	mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	// 入力と出力が同じサイズならそのまま複製
	if( (inwidth == outwidth) && (inheight == outheight) &&
		(inwidth == wwidth  ) && (inheight == wheight  ) ){
		for( x = 0; x < inwidth * inheight; ++x ){
			out[ x ] = in[ x ] & mask;
		}
		return true;
	}

	// 拡大か縮小か
	bWZoom = ( inwidth < wwidth );
	bHZoom = ( inheight< wheight);

	// 出力先全ピクセルを舐める
	for( y = 0; y < wheight; ++y ){
		//	経過を表示
		if( !prog( y * 100 / wheight ) ) return false;
		//	入力画像の座標へ変換
		iym1 = int(( float( y )        ) * inheight / wheight );	//	上側
		iym2 = int(( float( y ) + 1.0f ) * inheight / wheight );	//	下側
		iyw  = iym2 - iym1;
		yy   = y * inheight / wheight;
		for( x = 0; x < wwidth; ++x ){
			// 入力画像の座標に変換
			ixm1 = int(( float( x )        ) * inwidth / wwidth );	//	左側
			ixm2 = int(( float( x ) + 1.0f ) * inwidth / wwidth );	//	右側
			ixw  = ixm2 - ixm1;
			// 出力先１ピクセルに対応する入力画像のピクセル集合平均を求める
			ay = 0;						// 重みの合計値（Ｙ軸方向）
			pry = pgy = pby = 0;		// ピクセル値合計値（Ｙ軸方向）
			if( bHZoom ) {
				//	垂直拡大の場合
				if( bWZoom ) {
					//	水平拡大の場合
					xx  = x * inwidth / wwidth;
					ax  = (float(x) * inwidth / wwidth ) - float(xx);	//	位置 xx+1 への一致度
					ay  = (float(y) * inheight/ wheight) - float(yy);	//	位置 xx+1 への一致度
					c   = _get_pixel( in, inwidth, inheight, xx, yy );
					prx = GET_RED( c ) * (1-ax) * (1-ay);
					pgx = GET_GREEN( c ) * (1-ax) * (1-ay);
					pbx = GET_BLUE( c ) * (1-ax) * (1-ay);
					c	= _get_pixel( in, inwidth, inheight, xx+1, yy );
					prx += GET_RED( c ) * ax * (1-ay);
					pgx += GET_GREEN( c ) * ax * (1-ay);
					pbx += GET_BLUE( c ) * ax * (1-ay);
					c   = _get_pixel( in, inwidth, inheight, xx, yy+1 );
					prx += GET_RED( c ) * (1-ax) * ay;
					pgx += GET_GREEN( c ) * (1-ax) * ay;
					pbx += GET_BLUE( c ) * (1-ax) * ay;
					c	= _get_pixel( in, inwidth, inheight, xx+1, yy+1 );
					prx += GET_RED( c ) * ax * ay;
					pgx += GET_GREEN( c ) * ax * ay;
					pbx += GET_BLUE( c ) * ax * ay;
					ax = 1.0f;
				} else {
					//	水平縮小の場合
					if( ixw == 0 ){
						c	= _get_pixel( in, inwidth, inheight, ixm1, yy );
						prx = GET_RED( c ) * (1-ay);
						pgx = GET_GREEN( c ) * (1-ay);
						pbx = GET_BLUE( c ) * (1-ay);
						c	= _get_pixel( in, inwidth, inheight, ixm1, yy+1 );
						prx += GET_RED( c ) * ay;
						pgx += GET_GREEN( c ) * ay;
						pbx += GET_BLUE( c ) * ay;
						ax  = am = 1.0f;
					}else{
						ax = 0;					// 重みの合計値（Ｘ軸方向）
						prx = pgx = pbx = 0;	// ピクセル値合計値（Ｘ軸方向）
						for( ix = ixm1; ix <= ixm2; ++ix ){
							am = float( ix - ixm1 ) / ixw;
							if( am < 0 ) am = -am;
							am = 1.0f - am;
							c = _get_pixel( in, inwidth, inheight, ix, yy );
							prx += GET_RED( c ) * am * (1-ay);
							pgx += GET_GREEN( c ) * am * (1-ay);
							pbx += GET_BLUE( c ) * am * (1-ay);
							c = _get_pixel( in, inwidth, inheight, ix, yy+1 );
							prx += GET_RED( c ) * am * ay;
							pgx += GET_GREEN( c ) * am * ay;
							pbx += GET_BLUE( c ) * am * ay;
							ax  += am;
						}	// ix
					}
				}
				_set_pixel( out, outwidth, outheight, x, y,
						GET_RGB( int( prx/ax ), int( pgx/ax ), int( pbx/ax ) ) & mask );
			} else {
				//	垂直縮小の場合
				for( iy = iym1; iy <= iym2; ++iy ){
					if( bWZoom ) {
						//	水平拡大の場合
						xx  = x * inwidth / wwidth;
						ax  = (float(x) * inwidth / wwidth) - float(xx);	//	位置 xx+1 への一致度
						c   = _get_pixel( in, inwidth, inheight, xx, iy );
						prx = GET_RED( c ) * (1-ax);
						pgx = GET_GREEN( c ) * (1-ax);
						pbx = GET_BLUE( c ) * (1-ax);
						c = _get_pixel( in, inwidth, inheight, xx+1, iy );
						prx += GET_RED( c ) * ax;
						pgx += GET_GREEN( c ) * ax;
						pbx += GET_BLUE( c ) * ax;
						ax = 1.0f;
					} else {
						//	水平縮小の場合
						if( ixw == 0 ){
							c = _get_pixel( in, inwidth, inheight, ixm1, iy );
							prx = GET_RED( c );
							pgx = GET_GREEN( c );
							pbx = GET_BLUE( c );
							ax  = am = 1.0f;
						}else{
							ax = 0;					// 重みの合計値（Ｘ軸方向）
							prx = pgx = pbx = 0;	// ピクセル値合計値（Ｘ軸方向）
							for( ix = ixm1; ix <= ixm2; ++ix ){
								am = float( ix - ixm1 ) / ixw;
								if( am < 0 ) am = -am;
								am = 1.0f - am;
								c = _get_pixel( in, inwidth, inheight, ix, iy );
								prx += GET_RED( c ) * am;
								pgx += GET_GREEN( c ) * am;
								pbx += GET_BLUE( c ) * am;
								ax  += am;
							}	// ix
						}
					}

					if( iyw == 0 ){
						am = 1.0f;
					}else{
						am = float( iy - iym1 ) / iyw;
						if( am < 0 ) am = -am;
						am = 1.0f - am;
					}
					pry += prx * am;
					pgy += pgx * am;
					pby += pbx * am;
					ay  += am;
				}	// iy
				ay *= ax;
				_set_pixel( out, outwidth, outheight, x, y,
						GET_RGB( int( pry/ay ), int( pgy/ay ), int( pby/ay ) ) & mask );
			}
		}	// x
	}	// y
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ディザ処理
//	2.	引数
//		r		...	赤の出力先
//		g		...	緑の出力先
//		b		...	青の出力先
//		mode	...	ディザの種類
//		ErrAdd	...	ディザの適用方法
//		x		...	Ｘ座標
//		y		...	Ｙ座標
//		c		...	色
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static inline void _put_dither_pattern( int *r, int *g, int *b, int screen_mode, int ErrAdd, int x, int y, C_COLOR c )
{
	if( screen_mode >= EALGO_DITH1 ){
		switch( ErrAdd ){
		case EADD_ROTATE:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][       x & 7  ][ y & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][       y & 7  ][ x & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ 7 - ( x & 7 )][ y & 7 ];
			break;
		case EADD_NONE:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ];
			break;
		case EADD_X:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ ( x + 0 ) & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ ( x + 1 ) & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ ( x + 2 ) & 7 ];
			break;
		case EADD_Y:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][ ( y + 0 ) & 7 ][ x & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][ ( y + 1 ) & 7 ][ x & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ ( y + 2 ) & 7 ][ x & 7 ];
			break;
		}
	}else{
		*r = GET_RED(   c );
		*g = GET_GREEN( c );
		*b = GET_BLUE(  c );
	}
}

// -------------------------------------------------------------
//	減色処理

// -------------------------------------------------------------
//	1.	日本語名
//		減色ルーチン（sc2(4)/3/5/6/7/8用）
//	2.	引数
//		in		...	画像入力メモリ（内容は破壊されます）
//		width	...	画像幅
//		height	...	画像高さ
//		out		...	画像出力メモリ
//		CnvMode	...	現在の設定
//		prog	...	経過表示関数
//		pal		...	パレット
//	3.	返値
//		false	...	失敗
//		true	...	成功
//	4.	備考
//		今のところメモリ不足以外で失敗することはない	
// -------------------------------------------------------------
bool cnvRecolor( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt )
{
	bool	bRet;

	if( CnvMode->screen_mode == MD_SC8 || CnvMode->screen_mode == MD_SC8_256L ) {
		//	256色ビットマップへ変換する
		return cnvRecolor8( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );
	} else {
		//	16色ビットマップへ変換する
		bRet = cnvRecolor5( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );

		//	特殊な画面（SC2/3/4) にあわせて変換する		
		switch( CnvMode->screen_mode ) {
		case MD_SC2:
			bRet = bRet && cnvSC5toSC2( out, prog, pal );
			break;
		case MD_SC3:
			bRet = bRet && cnvSC5toSC3( out, prog );
			break;
		}
	}
	return bRet;
}

// -------------------------------------------------------------
//	1.	日本語名
//		減色ルーチン（sc8用）
//	2.	引数
//		in		...	画像入力メモリ（内容は破壊されます）
//		width	...	画像幅
//		height	...	画像高さ
//		out		...	画像出力メモリ
//		CnvMode	...	現在の設定
//		prog	...	経過表示関数
//		pal		...	パレット
//	3.	返値
//		false	...	失敗
//		true	...	成功
//	4.	備考
//		同じことを数カ所に書いているのは高速化のため。
// -------------------------------------------------------------
static bool cnvRecolor8( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS p_progress_cbr,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt ) {
	int				x, y, ptr, buffer_size;
	int				cr, cg, cb;		// 元画素のＲＧＢ
	int				er, eg, eb;		// 誤差（正数へ丸める）
	int				n;
	C_COLOR c,cc,*pin;
	C_COLOR fource_zero_color = -1;
	signed short *p_diffusion_error[2] = { nullptr, nullptr };
	signed short *p_x_diffusion_error, *p_y_diffusion_error;
	int diffusion_error_coef = 0;
	int x_diffusion_error_coef = 0;
	int y_diffusion_error_coef = 0;
	int ee = CnvMode->err;
	C_COLOR bit_depth_mask;
	bool ret = false;
	int convert14to255_s8r[ 15 ] = { 0 };
	int convert14to255_s8g[ 15 ] = { 0 };
	int convert6to255_s8b[ 7 ] = { 0 };

	// 誤差拡散係数
	if( CnvMode->diffusion_error_enable ) {
		diffusion_error_coef = int( CnvMode->diffusion_error_coef * 1024 );
		x_diffusion_error_coef = int( CnvMode->diffusion_error_x_weight * 256. );
		y_diffusion_error_coef = 256 - x_diffusion_error_coef;
	}

	// 誤差蓄積用バッファの作成
	buffer_size = ( width + 5 ) * 4 * sizeof( signed short );
	for( x = 0; x < 2; x++ ) {
		p_diffusion_error[ x ] = (signed short*) malloc( buffer_size );
		if( p_diffusion_error[ x ]==NULL ) goto l_exit;
		memset( p_diffusion_error[ x ], 0, buffer_size );
	}

	// 2倍サイズ変換テーブル作成
	for( x = 0; x < 15; x++ ) {
		if( (x & 1) == 0 ) {
			convert14to255_s8r[ x ] = convert7to255_s8r[ x >> 1 ];
			convert14to255_s8g[ x ] = convert7to255_s8g[ x >> 1 ];
		}
		else {
			convert14to255_s8r[ x ] = ( convert7to255_s8r[ x >> 1 ] + convert7to255_s8r[ ( x >> 1 ) + 1 ] ) >> 1;
			convert14to255_s8g[ x ] = ( convert7to255_s8g[ x >> 1 ] + convert7to255_s8g[ ( x >> 1 ) + 1 ] ) >> 1;
		}
		if( x < 7 ) {
			if( ( x & 1 ) == 0 ){
				convert6to255_s8b[ x ] = convert3to255_s8b[ x >> 1 ];
			}
			else{
				convert6to255_s8b[ x ] = ( convert3to255_s8b[ x >> 1 ] + convert3to255_s8b[ ( x >> 1 ) + 1 ] ) >> 1;
			}
		}
	}

	// 精度制限用マスク作成
	bit_depth_mask = ( ~(( 1 << CnvMode->Seido ) - 1 ) ) & 0xFF;	//	8bit mask
	bit_depth_mask = ( bit_depth_mask << 16 ) | ( bit_depth_mask << 8 ) | bit_depth_mask;

	// 強制 0化 の色
	if( CnvMode->FourceZero ) {
		fource_zero_color = CnvMode->FZColor & bit_depth_mask;
	}

	// 全ピクセルを舐める
	ptr = 0;
	pin = in;

	for( y = 0; y < height; ++y ){
		if( !p_progress_cbr( y * 100 / height ) ) goto l_exit;
		p_x_diffusion_error = p_diffusion_error[ y & 1 ];
		p_y_diffusion_error = p_diffusion_error[ 1 - ( y & 1 ) ];

		for( x = 0; x < width; ++x ){
			cc = c = *pin;
			pin++;
			
			// ディザ処理
			_put_dither_pattern( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
			
			if( diffusion_error_coef ) {
				// 誤差を計算に入れる
				cr += p_x_diffusion_error[ x * 4 + 0 ];
				cg += p_x_diffusion_error[ x * 4 + 1 ];
				cb += p_x_diffusion_error[ x * 4 + 2 ];
			}

			// 最も近い色を見つける
			if( CnvMode->Tile ) {
				//	タイルモード
				er = convert_rgb_to_palette( convert14to255_s8r, 15, cr );
				eg = convert_rgb_to_palette( convert14to255_s8g, 15, cg );
				eb = convert_rgb_to_palette( convert6to255_s8b ,  7, cb );
				if( ( x ^ y ) & 1 ){
					er = ( er + 1 ) >> 1;
					eg = ( eg + 1 ) >> 1;
					eb = ( eb + 1 ) >> 1;
					er = range_limiter( er, 0, 7 );
					eg = range_limiter( eg, 0, 7 );
					eb = range_limiter( eb, 0, 3 );
				}
				else {
					er >>= 1;
					eg >>= 1;
					eb >>= 1;
				}
			}
			else {
				//	非タイルモード
				er = convert_rgb_to_palette( convert7to255_s8r, 8, cr );
				eg = convert_rgb_to_palette( convert7to255_s8g, 8, cg );
				eb = convert_rgb_to_palette( convert3to255_s8b, 4, cb );
			}
			if( er == 0 && eg == 0 && eb == 0 && CnvMode->NonZero ) {
				er = 0;
				eg = 1;
				eb = 0;
			}
			n = ( er << 2 ) | ( eg << 5 ) | eb;
			cc = GET_RGB( convert7to255_s8r[ er ], convert7to255_s8g[ eg ], convert3to255_s8b[ eb ] );

			if( diffusion_error_coef ) {
				// 誤差を周囲のピクセルへ拡散させる
				er = range_limiter( ( cr - GET_RED( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eg = range_limiter( ( cg - GET_GREEN( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eb = range_limiter( ( cb - GET_BLUE( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				// 微細な誤差は消滅させる
				if( _abs( er ) < (signed)CnvMode->err ) er = 0;
				if( _abs( eg ) < (signed)CnvMode->err ) eg = 0;
				if( _abs( eb ) < (signed)CnvMode->err ) eb = 0;
				// 右に拡散
				p_x_diffusion_error[ ( x + 1 ) * 4 + 0 ] += (signed short)( ( er * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 1 ] += (signed short)( ( eg * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 2 ] += (signed short)( ( eb * x_diffusion_error_coef ) >> 8 );
				// 下に拡散
				p_y_diffusion_error[ x * 4 + 0 ] = (signed short)( ( er * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 1 ] = (signed short)( ( eg * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 2 ] = (signed short)( ( eb * y_diffusion_error_coef ) >> 8 );
			}

			// 結果を出力する
			if( CnvMode->FourceZero && cc == fource_zero_color ) n = 0;	// 強制ゼロ化
			out[ ptr ] = n;
			// 次の出力先
			++ptr;
		}	// x
	}	// y
	ret = true;
l_exit:
	if( p_diffusion_error[0] != nullptr ) free( p_diffusion_error[0] );
	if( p_diffusion_error[1] != nullptr ) free( p_diffusion_error[1] );
	return ret;
}

// -------------------------------------------------------------
//	1.	日本語名
//		減色ルーチン（sc2(4)/3/5/6/7用）
//	2.	引数
//		in		...	画像入力メモリ（内容は破壊されます）
//		width	...	画像幅
//		height	...	画像高さ
//		out		...	画像出力メモリ
//		CnvMode	...	現在の設定
//		prog	...	経過表示関数
//		pal		...	パレット
//	3.	返値
//		false	...	失敗
//		true	...	成功
//	4.	備考
//		今のところメモリ不足以外で失敗することはない	
// -------------------------------------------------------------
static bool cnvRecolor5( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt ) {
	int x, y, z, d, ptr;
	int cr, cg, cb;		// 元画素のＲＧＢ
	int pr, pg, pb;		// 画素値
	int er, eg, eb;		// 誤差（正数へ丸める）
	int r, n, nr;
	C_COLOR c, cc, *pin;
	C_COLOR fource_zero_color = -1;
	signed short *p_diffusion_error[2] = { nullptr, nullptr };
	signed short *p_x_diffusion_error, *p_y_diffusion_error;
	int palnum;
	int diffusion_error_coef = 0;
	int x_diffusion_error_coef = 0;
	int y_diffusion_error_coef = 0;
	int ee = CnvMode->err;
	C_COLOR bit_depth_mask;
	bool ret = false;
	int color_num;

	// 誤差拡散係数
	if( CnvMode->diffusion_error_enable ){
		diffusion_error_coef = int( CnvMode->diffusion_error_coef * 1024 );
		x_diffusion_error_coef = int( CnvMode->diffusion_error_x_weight * 256. );
		y_diffusion_error_coef = 256 - x_diffusion_error_coef;
	}

	// 誤差蓄積用バッファの作成
	memset( p_diffusion_error, 0, sizeof( p_diffusion_error ) );
	for( x = 0; x < 2; ++x ) {
		p_diffusion_error[ x ] = (signed short*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 4 * sizeof( signed short ) );
		if( p_diffusion_error[ x ]==NULL ) goto l_exit;
	}

	// 精度制限用マスク作成
	bit_depth_mask = ~(( 1 << CnvMode->Seido ) - 1 );
	bit_depth_mask = bit_depth_mask & (( bit_depth_mask << 8 ) | 0xFF ) & (( bit_depth_mask << 16 ) | 0xFFFF );

	// パレット数
	switch( CnvMode->screen_mode ){
	case MD_SC6:
	case MD_SC6_256L:
		palnum = 4;		break;
	default:
		palnum = 16;	break;
	}
	fource_zero_color = CnvMode->FourceZero ? CnvMode->FZColor & bit_depth_mask : -1;

	if( CnvMode->Tile ) {
		color_num = tailcnt;
	}
	else {
		color_num = palnum;
	}

	// 全ピクセルを舐める
	ptr = 0;
	pin = in;
	n = 0;

	for( y = 0; y < height; ++y ){
		if( !prog( y * 100 / height ) ) goto l_exit;
		p_x_diffusion_error = p_diffusion_error[ y & 1 ];
		p_y_diffusion_error = p_diffusion_error[ 1 - ( y & 1 ) ];

		for( x = 0; x < width; ++x ){
			cc = c = *pin;
			pin++;
			
			// ディザ処理
			_put_dither_pattern( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
			
			if( diffusion_error_coef ) {
				// 誤差を計算に入れる
				cr += p_x_diffusion_error[ x * 4 + 0 ];
				cg += p_x_diffusion_error[ x * 4 + 1 ];
				cb += p_x_diffusion_error[ x * 4 + 2 ];
			}

			// 最も近い色を見つける
			nr = 0x7FFFFFFFL;

			if( CnvMode->NonZero ) {
				z = 1; 
			}
			else {
				z = 0;
			}
			// 比較する色は実際に表示できる色で無ければならないため、範囲外をクリップする
			pr = range_limiter( cr, 0, 255 );
			pg = range_limiter( cg, 0, 255 );
			pb = range_limiter( cb, 0, 255 );

			for( ; z < color_num; ++z ){
				// 近いか？
				if( CnvMode->Tile ) {
					// タイルモード
					c = tail[ z ].c;
				}
				else {
					// 非タイルモード
					if( CnvMode->PalEn[ z ] == PALEN_NONE ) {
						//	このパレットは使用禁止
						continue;
					}
					c = pal[ z ];
				}
				r = _distance_itp( pr, pg, pb, GET_RED(c), GET_GREEN(c), GET_BLUE(c) );
				if( r < nr ){
					nr = r;
					if( CnvMode->Tile ) {
						// タイルモード
						n  = tail[z].p[ ( x ^ y ) & 1 ];
					}
					else {
						// 非タイルモード
						n  = z;
					}
					cc = c;
					if( r == 0 ) break;	// 完全一致なら抜ける
				}
			}	// z

			if( diffusion_error_coef ) {
				// 誤差を周囲のピクセルへ拡散させる
				er = range_limiter( ( cr - GET_RED(   cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eg = range_limiter( ( cg - GET_GREEN( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eb = range_limiter( ( cb - GET_BLUE(  cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				// 微細な誤差は消滅させる
				if( _abs( er ) < (signed)CnvMode->err ) er = 0;
				if( _abs( eg ) < (signed)CnvMode->err ) eg = 0;
				if( _abs( eb ) < (signed)CnvMode->err ) eb = 0;
				// 右に拡散
				p_x_diffusion_error[ ( x + 1 ) * 4 + 0 ] += (signed short)( ( er * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 1 ] += (signed short)( ( eg * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 2 ] += (signed short)( ( eb * x_diffusion_error_coef ) >> 8 );
				// 下に拡散
				p_y_diffusion_error[ x * 4 + 0 ] = (signed short)( ( er * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 1 ] = (signed short)( ( eg * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 2 ] = (signed short)( ( eb * y_diffusion_error_coef ) >> 8 );
			}

			// 結果を出力する
			if( CnvMode->FourceZero && cc == fource_zero_color ) n = 0;	// 強制ゼロ化
			if( palnum == 4 ) {
				// 4色
				if( x & 3 ){
					d=out[ ptr >> 2 ];
					d= d | ( n & 0x03 ) << ( ( 3 - ( x & 3 ) ) * 2 );
				}else{
					d= n << 6;
				}
				out[ ptr >> 2 ] = d;
			} else {
				// 16色
				if( x & 1 ){
					d=out[ptr>>1];
					d=( d & 0xF0 ) | n;
				}else{
					d= n<<4;
				}
				out[ptr>>1]=d;
			}
			// 次の出力先
			++ptr;
		}	// x
	}	// y
	ret = true;
l_exit:
	if( p_diffusion_error[0] != NULL ) LocalFree( p_diffusion_error[0] );
	if( p_diffusion_error[1] != NULL ) LocalFree( p_diffusion_error[1] );
	return ret;
}

// -------------------------------------------------------------
//	1.	日本語名
//		カラーパレットの明るさを取得する
//	2.	引数
//		p_pal	...	パレットに対応する色
//		c ......... パレット番号
//	3.	返値
//		輝度レベル 0-65535
//	4.	備考
//		なし
// -------------------------------------------------------------
static int _get_palette_luminance( C_COLOR *p_pal, int c ) {
	int r, g, b;

	r = GET_RED( p_pal[ c ] );
	g = GET_GREEN( p_pal[ c ] );
	b = GET_BLUE( p_pal[ c ] );
	return (77 * r + 150 * g + 29 * b);
}

// -------------------------------------------------------------
//	1.	日本語名
//		SCREEN5 イメージを SCREEN2/4 イメージへ変換する
//	2.	引数
//		p_image	...	VRAMイメージ
//		prog	...	経過表示関数
//		p_pal	...	パレットに対応する色
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		インターレースモードは使えない
// -------------------------------------------------------------
static bool cnvSC5toSC2( unsigned char *p_image, PROGRESS prog, C_COLOR *p_pal )
{
	int	x, y, i, j, adr, dadr;
	int	cc, c, p, lum1, lum2;
	int	cnt, idx1, idx2, len, l;
	int	ccnt[ 8 ] = { 0 };
	int	ccol[ 8 ] = { 0 };
	C_COLOR	c1, c2;

	memset( vram, 0, sizeof( vram ) );

	adr  = 0;
	for( y = 0; y < 192; y++ ) {
		for( x = 0; x < 32; x++ ) {
			//	８ドット組の中で使われている色を調べる： cnt = 色数
			cnt  = 0;
			idx1 = 0;							//	最も多い色のインデックス
			for( j = 0; j < 8; j++ ) {			//	8ドットなめる
				cc = p_image[ adr + j / 2 ];
				if( j & 1 ) {
					cc = cc & 0x0F;
				} else {
					cc = (cc >> 4) & 0x0F;
				}
				for( i = 0; i < cnt; ++i ) {	//	同じ色を検索
					if( cc == ccol[ i ] ) {
						ccnt[ i ]++;			//		見つけた色の数を増加
						if( ccnt[ i ] > ccnt[ idx1 ] ) {	//	最も多い
							idx1 = i;
						}
						break;
					}
				}
				if( i == cnt ) {				//	新しい色
					ccol[ cnt ] = cc;			//		見つけた色
					ccnt[ cnt ] = 1;			//		見つけた色の数
					cnt++;
				}
			}

			if( cnt < 2 ) {
				//	１色しかない場合
				c = ccol[ 0 ];					//	カラーテーブルに格納する値
				p = 0x00;						//	パターンジェネレータテーブルに格納する値
			} else {
				//	２色以上の場合

				//	１色目と最も異なる２色目を選択する（コントラスト重視）
				len  = 0;
				c1= p_pal[ ccol[ idx1 ] ];
				for( i = 0; i < cnt; ++i ) {
					if( i == idx1 ) continue;
					cc = ccol[i];
					l = _distance_itp( GET_RED( p_pal[cc] ), GET_GREEN( p_pal[cc] ), GET_BLUE( p_pal[cc] ), GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ));
					if( l >= len ) {
						len = l;
						idx2 = i;
					}
				}

				c = ( ccol[ idx1 ] << 4 ) | ccol[ idx2 ];
				p = 0;
				c2= p_pal[ ccol[ idx2 ] ];
				for( j = 0; j < 8; j++ ) {
					cc = p_image[ adr + j / 2 ];
					if( j & 1 ) {
						cc = cc & 0x0F;
					} else {
						cc = (cc >> 4) & 0x0F;
					}
					//	選ばれた２色との色距離を比較
					p = p << 1;
					if( _distance_itp( GET_RED( p_pal[cc] ), GET_GREEN( p_pal[cc] ), GET_BLUE( p_pal[cc] ), GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 )) 
					  < _distance_itp( GET_RED( p_pal[cc] ), GET_GREEN( p_pal[cc] ), GET_BLUE( p_pal[cc] ), GET_RED( c2 ), GET_GREEN( c2 ), GET_BLUE( c2 )) ) {
						p = p | 1;
					}
				}

			}

			//	明るい色の方を パターンジェネレータのbit値 = 1 の方に割り当てる
			if( cnt >= 2 ) {
				lum1 = _get_palette_luminance( p_pal, c & 0x0F );
				lum2 = _get_palette_luminance( p_pal, ( c >> 4 ) & 0x0F );
				if( ( lum1 == lum2 && (c & 0x0F) > ((c >> 4) & 0x0F) ) || (lum1 > lum2) ) {
					p = p ^ 255;
					c = ((c << 4) & 0xF0) | ((c >> 4) & 0x0F);
				}
			}
			else {
				//	1色しか無い場合でも、明るい色なら前景色 (パターンジェネレータの bit値 = 1) に変更する
				lum1 = _get_palette_luminance( p_pal, c & 0x0F );
				if( lum1 >= (128 << 8) ) {
					p = p ^ 255;
					c = ( ( c << 4 ) & 0xF0 ) | ( ( c >> 4 ) & 0x0F );
				}
			}

			//	格納
			dadr = ( y & 7 ) + ( x * 8 ) + ( y >> 3 )*256;
			vram[ dadr			  ] = p;
			vram[ dadr + SC2COLOR ] = c;
			//	8ドット移動
			adr += 4;
		}
	}

	memcpy( p_image, vram, sizeof( vram ) );
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		SCREEN5 イメージを SCREEN3 イメージへ変換する
//	2.	引数
//		out		...	VRAMイメージ
//		prog	...	経過表示関数
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		インターレースモードは使えない
// -------------------------------------------------------------
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog )
{
	int	x, y, yy, adr;

	memset( vram, 0, sizeof( vram ) );

	adr = 0;
	for( y = 0; y < 6; ++y ) {
		for( yy = 0; yy < 8; ++yy ) {
			for( x = 0; x < 32; ++x ) {
				vram[ x*8 + yy + y*256 ] = out[ adr ];
				++adr;
			}
		}
	}

	memcpy( out, vram, sizeof( vram ) );
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		減色ルーチン（自然画専用）
//	2.	引数
//		in		...	画像入力メモリ
//		width	...	画像幅 ( 256 )
//		height	...	画像高さ ( 212 )
//		out		...	画像出力メモリ
//		CnvMode	...	現在の設定
//		prog	...	経過表示関数
//	3.	返値
//		false	...	失敗
//		true	...	成功
//	4.	備考
//		今のところメモリ不足以外で失敗することはない
// -------------------------------------------------------------
bool cnvNtcolor( C_COLOR *in ,int width ,int height ,unsigned char *out ,
				 SETTING *CnvMode ,PROGRESS prog )
{
	int				x, y, z, xx;
	int				convert31to255_s12y[ 32 ];
	C_COLOR		c;
	int				r[ 5 ] = { 0 }, g[ 5 ] = { 0 }, b[ 5 ] = { 0 };
	int				vy, min, tyy, txx;
	int				rr, gg, bb, er, eg, eb;
	int				jj, kk, yy[ 4 ] = { 0 }, ii[ 4 ] = { 0 }, s;
	int				mask;
	unsigned char	*pout = out;
	short			*ptmp,*tmp = NULL;
	int				*errbuf[ 2 ], *errbuf0, *errbuf1;

	int		k		= CnvMode->diffusion_error_enable ? int( CnvMode->diffusion_error_coef * 1024 ) : 0;
	int		kx		= int( CnvMode->diffusion_error_x_weight * 256. );
	int		ky		= 256 - kx;
	int		algo	= CnvMode->AlgoMode;
	bool	rc		= CnvMode->JKrc;
	int		ealgo	= CnvMode->ErrAlgo;
	bool	sc10	= ( CnvMode->screen_mode == MD_SC10 || CnvMode->screen_mode == MD_SC10_256L );
	int		zero	= CnvMode->FourceZero ? (sc10 ? 0x10 : 0x08) : 0;
	bool	ret		= false;

	tyy = 0;

	for( y = 0; y < 32; y++ ){
		convert31to255_s12y[ y ] = (convert31to255_s12r[ y ] * 4 + convert31to255_s12g[ y ] * 8  + convert31to255_s12b[ y ] * 2 + 7) / 14;
		convert31to255_s12y[ y ] = range_limiter( convert31to255_s12y[ y ], 0, 255 );
	}
	// 画面モードに合わせたマスクを設定
	if( sc10 ){
		mask=0xF0;
	}else{
		mask=0xF8;
	}
	// 誤差蓄積用ﾊﾞｯﾌｧの作成
	memset( errbuf, 0, sizeof( errbuf ) );
	for( x = 0; x < 2; ++x ) {
		errbuf[ x ] = (signed int*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 3 * sizeof( int ) );
		if( errbuf[ x ]==NULL ) goto l_exit;
	}
	// 誤差拡散の場合は、いったん Ｙをすべて生成してから拡散する
	if( k ){
		ptmp = tmp = (short*)LocalAlloc( LMEM_FIXED, sizeof(short) * width * ( height + 1 ) );
		if( tmp == NULL ) goto l_exit;
	}
	for( y = 0; y < height; ++y ){
		if( !prog( y * 100 / height ) ) goto l_exit;	// 経過を表示

		errbuf0 = errbuf[ y & 1 ];
		errbuf1 = errbuf[ 1 - ( y & 1 ) ];
		for( x = xx = 0; x < width; x += 4, xx += 12 ){
			//	標準的な変換により各画素の J,K の値を確定する
			jj = kk = 0;
			txx = xx;
			for( z = 0; z < 4; ++z ){
				c = _get_pixel( in, width, height, x + z, y );
				r[ z ] = convert31to255_s12r[ convert_rgb_to_palette( convert31to255_s12r, 32, GET_RED( c ) + errbuf0[ txx + 0 ] ) ];
				g[ z ] = convert31to255_s12g[ convert_rgb_to_palette( convert31to255_s12g, 32, GET_GREEN( c ) + errbuf0[ txx + 1 ] ) ];
				b[ z ] = convert31to255_s12b[ convert_rgb_to_palette( convert31to255_s12b, 32, GET_BLUE( c ) + errbuf0[ txx + 2 ] ) ];
				vy   = range_limiter( b[ z ] / 2 + r[ z ] / 4 + g[ z ] / 8, 0, 255 );
				jj += r[ z ] - vy;
				kk += g[ z ] - vy;
				txx += 3;
			}
			// J,K の値を平均して４画素ペアの J,K 値を確定する
			jj = range_limiter( jj / 4, -256, 255 );
			kk = range_limiter( kk / 4, -256, 255 );

			// Y の算出と範囲外切りつめ
			txx = xx;
			for( z = 0; z < 4; ++z ){
				// アルゴリズムの適用
				switch( algo ){
				case ALGO_NONE:		// 無し
					tyy = (b[ z ] + ( r[ z ] + g[ z ] / 2 ) / 2 ) / 2;
					break;
				case ALGO_RECALC:	// 輝度再計算による誤差訂正アルゴリズム（画質・輝度再現優先）
					tyy = 0;
					min = 0x7FFFFFFF;	//	十分大きな値（∞の意味）
					rr = range_limiter( r[z], 0, 255 );
					gg = range_limiter( g[z], 0, 255 );
					bb = range_limiter( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = _distance_itp( rr, gg, bb,
							range_limiter( vy + jj, 0, 255 ),  
							range_limiter( vy + kk, 0, 255 ), 
							range_limiter( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
						if( min <= s ) break;	//	誤差が広がりはじめたらうち切る
						min = s; tyy = vy;
					}
					break;
				case ALGO_POWERFUL:		// パワフル活用法の計算式
					ii[ z ]= b[ z ] + r[ z ] * 2 + g[ z ] * 4;
					tyy = ( ( ii[ z ] * 4 ) - 6 * jj - 15 * kk ) / 29;
					break;
				case ALGO_NTSC2:		// 微調整版
					tyy = ( r[ z ] * 27 + g[ z ] * 28 + b[ z ] * 50 ) / 115;
					break;
				case ALGO_RECALC2:	// 輝度再計算による誤差訂正アルゴリズム（色合い再現優先）
					tyy = 0;
					min = 0x7FFFFFFF;	//	十分大きな値（∞の意味）
					rr = range_limiter( r[z], 0, 255 );
					gg = range_limiter( g[z], 0, 255 );
					bb = range_limiter( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = _distance_itp( rr, gg, bb,
							range_limiter( vy + jj, 0, 255 ),  
							range_limiter( vy + kk, 0, 255 ), 
							range_limiter( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
						if( min <= s ) break;	//	誤差が広がりはじめたらうち切る
						min = s; tyy = vy * 61 / 64;
					}
					break;
				}	// switch
				// ディザパターン
				if( ealgo >= EALGO_DITH1 ){			// ディザリング
					tyy += dith[ ealgo-EALGO_DITH1 ][ y & 7 ][ ( x + z ) & 7 ] / 2;
				}
				// 範囲外除外
				yy[ z ] = range_limiter( tyy, 0, 255 );
				if( k ){	// 誤差拡散
					// 算出した 仮Y,J,K から R,G,B 値（ MSXで表示される色 ）を計算
					rr = convert31to255_s12r[ convert_rgb_to_palette( convert31to255_s12r, 32, range_limiter( jj + yy[ z ], 0, 255 ) ) ];
					gg = convert31to255_s12g[ convert_rgb_to_palette( convert31to255_s12g, 32, range_limiter( kk + yy[ z ], 0, 255 ) ) ];
					bb = convert31to255_s12b[ convert_rgb_to_palette( convert31to255_s12b, 32, range_limiter( 5 * yy[ z ] / 4 - jj / 2 - kk / 4, 0, 255 ) ) ];
					// 誤差を周囲のピクセルへ拡散させる
					er = range_limiter( ( r[ z ] - rr ) * k / 1024 , -32768, 32767 );
					eg = range_limiter( ( g[ z ] - gg ) * k / 1024 , -32768, 32767 );
					eb = range_limiter( ( b[ z ] - bb ) * k / 1024 , -32768, 32767 );

					// 微細な誤差は消滅させる
					if( _abs( er ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 3 ] += (er * kx) >> 8;
						errbuf1[ txx + 0 ] = (er * ky) >> 8;
					} else {
						errbuf1[ txx + 0 ] = 0;
					}
					if( _abs( eg ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 4 ] += (eg * kx) >> 8;
						errbuf1[ txx + 1 ] = (eg * ky) >> 8;
					} else {
						errbuf1[ txx + 1 ] = 0;
					}
					if( _abs( eb ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 5 ] += (eb * kx) >> 8;
						errbuf1[ txx + 2 ] = (eb * ky) >> 8;
					} else {
						errbuf1[ txx + 2 ] = 0;
					}
					txx += 3;
				}
			}	// for z

			// 色差情報の再計算
			if( rc ){
				jj = kk = 0;
				for( z = 0; z < 4; ++z ){
					jj += r[ z ] - yy[ z ];
					kk += g[ z ] - yy[ z ];
				}
				jj = ( jj + 2 ) / 4;	// 四捨五入和平均
				kk = ( kk + 2 ) / 4;	// 四捨五入和平均
			}
			// J,K 範囲外切りつめ
			jj = range_limiter( jj * 31 / 255, -32, 31 );	// 小数点以下切り捨て
			kk = range_limiter( kk * 31 / 255, -32, 31 );	// 小数点以下切り捨て
			
			// 出力
			if( k ){			// Ｙ値誤差拡散
				*(ptmp++)=yy[0];
				*(ptmp++)=yy[1];
				*(ptmp++)=yy[2];
				*(ptmp++)=yy[3];
				*(out++ )= kk     & 7;
				*(out++ )=(kk>>3) & 7;
				*(out++ )= jj     & 7;
				*(out++ )=(jj>>3) & 7;
			}else{
				*out=( kk     & 7) | ( range_limiter( yy[0], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((kk>>3) & 7) | ( range_limiter( yy[1], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=( jj     & 7) | ( range_limiter( yy[2], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((jj>>3) & 7) | ( range_limiter( yy[3], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
			}
		}	// for x
	}	// for y
	if( !k ) goto l_done;

	// 誤差拡散
	ptmp=tmp;
	for( y = 0; y < height; ++y ){
		if( !prog( y * 100 / height ) ) goto l_exit;	// 経過を表示
		for( x = 0; x < width; ++x ){
			// 誤差の算出
			vy = ((*ptmp) >> 3 ) * 255 / 31;					//	採用したいＹ値
			z  = range_limiter( vy, 0, 255 ) & mask;				//	実際に採用するＹ値
			jj = ( vy - convert31to255_s12y[ convert_rgb_to_palette( convert31to255_s12y, 32, z ) ] ) * k / 1024;			//	誤差
			if( jj >= (signed)CnvMode->err ) {
				// 誤差の拡散
				if( x != width - 1 ) {
					*(ptmp + 1) = (short)range_limiter( (int)*(ptmp +     1) + jj, -32768, 32767 );
				}
				*(ptmp + width) = (short)range_limiter( (int)*(ptmp + width) + jj, -32768, 32767 );
			}
			++ptmp;
			// データの出力
			*pout |= z;
			if( *pout == 0 ) *pout = zero;
			++pout;
		}
	}
l_done:
	ret = true;

l_exit:
	if( tmp		  != NULL ) LocalFree( tmp );
	if( errbuf[0] != NULL ) LocalFree( errbuf[0] );
	if( errbuf[1] != NULL ) LocalFree( errbuf[1] );
	return ret;
}

// -------------------------------------------------------------
// ソート用色テーブル比較関数(降順ソート用)
//
// 引数：	tbl1		色１アドレス
//			tbl2		色２アドレス
// 返値：	tbl1<tbl2	正数
//			tbl1=tbl2	0
//			tbl1>tbl2	負数
// -------------------------------------------------------------
int cnvColorTblCompare( const void *tbl1, const void *tbl2 ){
	//	ヒストグラムのカウント値で大小比較する
	return( (long)( ( (C_COLOR_TABLE *)tbl2 )->n ) - (long)( ( (C_COLOR_TABLE *)tbl1 )->n ) );
}

// -------------------------------------------------------------
// 減色用パレット（２色～１６色）を求める
//
// 引数：	in			画像（内容はソートされます）
//			width		幅
//			height		高さ
//			pal			パレット（16個）のアドレス
//			mode		0 通常 / 1 多分布選択
//			cnt			使用するパレットの数
//			pp			決定済みパレットの数
//			FourceZero	強制ゼロ化の色がある true / ない false
//			FZColor		強制ゼロ化の色
// 返値：	成功 true / 失敗 false
// -------------------------------------------------------------
bool cnvGetPalette( C_COLOR *in,int width,int height,C_COLOR *pal,int screen_mode,int cnt,int fixed_color_num,
				    bool FourceZero,C_COLOR FZColor )
{
	C_COLOR	c;
	C_COLOR_TABLE *p_color_table, swap_buffer;
	int			n, base_color, compare_color, compare_count_distance;
	int			r, g, b, compare_color_distance, delete_color_distance, delete_base_color, delete_color, delete_count_distance;

	// ２～１６色しか処理できない
	if( cnt < 2 || cnt > 16 || cnt <= fixed_color_num ) return false;
	
	// ヒストグラムの作成
	n = cnvCreateHistgram( in, width * height, &p_color_table, pal, fixed_color_num, FourceZero, FZColor );
	if( p_color_table == NULL ) {
		return false;
	}
	memset( pal + fixed_color_num, 0, sizeof( C_COLOR ) * ( cnt - fixed_color_num ) );	

	// 色をソートする (固定色は、ヒストカウントが∞なので、上位に集まる)
	qsort( p_color_table, n, sizeof( C_COLOR_TABLE ), cnvColorTblCompare );

	if( screen_mode == 0 ){
		// 目的の色数になるまで似た色を削除し続ける
		while( n > cnt ){
			delete_base_color = -1;					// 基準となる色のインデックス
			delete_color = -1;						// 削除対象として選択した色のインデックス
			delete_color_distance = 0x7FFFFFFF;		// 削除対象として選択した色の距離
			delete_count_distance = 0;				// ヒストカウントの差
			for( base_color = 0; base_color < (n - 1); base_color++ ) {
				// 着目色
				c = p_color_table[ base_color ].c;
				r = GET_RED(   c );
				g = GET_GREEN( c );
				b = GET_BLUE(  c );
				for( compare_color = base_color + 1; compare_color < n; compare_color++ ){	// 着目色(base_color)に最も近くて使用数が少ない色(y)を検索する
					if( compare_color < fixed_color_num ) {
						//	0 ～ pp-1 は、変更してはいけない固定色なので対象外
						continue;
					}
					c = p_color_table[ compare_color ].c;
					compare_color_distance = _distance_itp( r, g, b, GET_RED( c ), GET_GREEN( c ), GET_BLUE( c ) );
					compare_count_distance = (p_color_table[ base_color ].n - p_color_table[ compare_color ].n);
					if( delete_count_distance > compare_count_distance ) {
						//	前回のヒットよりも ヒストグラムのカウント数の差が小さい ので対象外
						//continue;
					}
					if( delete_color_distance < compare_color_distance ) {
						//	前回のヒットよりも 似てない ので対象外
						continue;
					}
					delete_base_color = base_color;
					delete_color = compare_color;
					delete_color_distance = compare_color_distance;
					delete_count_distance = compare_count_distance;
				}
			}
			if( delete_color > 0 ){			// 検索して見つかった色を着目色に吸収する
				if( delete_base_color >= fixed_color_num ) {
					p_color_table[ delete_base_color ].n += p_color_table[ delete_color ].n;
				}
				if( delete_color < (n - 1) ) {
					//	削除対象が最後の1個でなければ、削除して詰める
					memcpy( &p_color_table[ delete_color ], &p_color_table[ delete_color + 1 ], sizeof( C_COLOR_TABLE ) * ( n - 1 - delete_color ) );
				}
				n--;
				//	多い順になるようにソートし直し
				for( compare_color = delete_color; compare_color > 0; compare_color-- ) {
					if( p_color_table[ compare_color ].n <= p_color_table[ compare_color - 1 ].n ) {
						break;
					}
					swap_buffer                        = p_color_table[ compare_color     ];
					p_color_table[ compare_color     ] = p_color_table[ compare_color - 1 ];
					p_color_table[ compare_color - 1 ] = swap_buffer;
				}
			}
		}
	} else {
		//	多分布選色
	}

	// テーブル内のパレット先頭 cnt 個を採用する
	for( base_color = 0; base_color < n && base_color < cnt; ++base_color ){
		pal[ base_color ] = p_color_table[ base_color ].c;
	}

	// メモリを解放する
	LocalFree( p_color_table );
	return true;
}

// -------------------------------------------------------------
// 減色用パレット（256色）を求める
//
// 引数：	pal			パレット（16個）のアドレス
// 返値：	成功 true / 失敗 false
// 備考：	SCREEN8 色番号のビットマップ
//			(MSB) [G][G][G][R][R][R][B][B] (LSB)
// -------------------------------------------------------------
bool cnvGetPaletteS8( C_COLOR *pal ){
	int			i, r, g, b;

	// ヒストグラムの作成
	ZeroMemory( pal, sizeof( C_COLOR ) * 256 );

	for( i = 0; i < 256; ++i ){
		// 着目色（選択色を使用の色）を取得
		r = convert7to255_r[ ( i >> 2 ) & 7 ];
		g = convert7to255_g[ ( i >> 5 ) & 7 ];
		b = convert7to255_b[ i & 3 ];
		pal[ i ] = GET_RGB( r, g, b );
	}
	return true;
}

// -------------------------------------------------------------
// ヒストグラムを作成する
//
// 引数：	in			画像メモリ
//			size		画像メモリのサイズ
//			tbl			カラーテーブルを格納するポインタのアドレス
//			Pal			無条件で追加するパレット
//			pp			Pal の数
//			FourceZero	除外する色がある true / ない false
//			FZColor		除外する色
// 返値：	tbl のサイズ
// -------------------------------------------------------------
static int cnvCreateHistgram( C_COLOR *in, int size, C_COLOR_TABLE **tbl, C_COLOR *pal, int pp,
		bool FourceZero, C_COLOR FZColor ) {

	C_COLOR_TABLE hash[ 512 ];
	int i, n, using_color_num, t, r, g, b;
	C_COLOR	c;

	// NULL クリア
	*tbl = NULL;
	using_color_num = 0;		// 色の使用数

	memset( hash, 0, sizeof( hash ) );
	for( i = 0; i < 512; i++ ) {
		r = convert7to255_r[ (i >> 6) & 7 ];
		g = convert7to255_g[ (i >> 3) & 7 ];
		b = convert7to255_b[ (i >> 0) & 7 ];
		hash[ i ].c = GET_RGB( r, g, b );
	}

	// 画素の数を数える
	for( i = 0; i < size; i++ ) {
		c = in[ i ];
		if( c == FZColor && FourceZero ) continue;
		r = convert_rgb_to_palette( convert7to255_r, 8, GET_RED(   c ) );
		g = convert_rgb_to_palette( convert7to255_g, 8, GET_GREEN( c ) );
		b = convert_rgb_to_palette( convert7to255_b, 8, GET_BLUE(  c ) );
		n = r * 64 + g * 8 + b;		//	ハッシュ関数
		hash[ n ].n++;
		if( hash[ n ].n == 1 ) {
			using_color_num++;
		}
	}	// for

	// カラーテーブルを確保する
	*tbl=(C_COLOR_TABLE*) LocalAlloc( LMEM_FIXED, sizeof( C_COLOR_TABLE ) * ( using_color_num + pp ) );
	if( *tbl == NULL ) return 0;

	// ハッシュ表から tbl へ変換
	t=0;
	for( i = 0; i < pp ; ++i ){
		//	確実に採用する色は、ヒストグラムのカウント値を ∞ (0x7FFFFFFF) にして入れておく
		(*tbl)[ t ].c = pal[ i ];
		(*tbl)[ t ].n = 0x7FFFFFFF;
		//	hash の中の同一色を採用しないために、ヒストグラムのカウント値を 0 にしておく
		r = convert_rgb_to_palette( convert7to255_r, 8, GET_RED(   pal[ i ] ) );
		g = convert_rgb_to_palette( convert7to255_g, 8, GET_GREEN( pal[ i ] ) );
		b = convert_rgb_to_palette( convert7to255_b, 8, GET_BLUE(  pal[ i ] ) );
		n = r * 64 + g * 8 + b;		//	ハッシュ関数
		hash[ n ].n = 0;
		//	次へ
		t++;
	}
	for( i = 0; i < 512; ++i ){
		if( hash[ i ].n == 0 ) {
			continue;
		}
		(*tbl)[ t ] = hash[ i ];
		t++;
	}
	return t;
}

// -------------------------------------------------------------
//	1.	日本語名
//		タイルパターンを求める(SC5/SC7専用)
//	2.	引数
//		pal		...	(I)	決定済みのパレット
//		palen	...	(I)	使用可能パレット
//		zeroen	...	(I)	ゼロを使用可能か否か
//		tail	...	(I)	タイルパターン格納先
//		mode	...	(I)	スクリーンモード
//	3.	返値
//		作成したタイルパターンの数
//	4.	備考
//		なし
// -------------------------------------------------------------
int cnvCreateTail4( C_PALETTE *pal,uint8_t *palen,bool zeroen,C_TILE_PATTERN *tail, int screen_mode )
{
	int i, j, k, l;
	int r,g,b;
	int rr,gg,bb;
	int n=0,m;
	bool ok;
	int	pnum;
	// すべての色の組み合わせを処理する
	l = zeroen ? 0 : 1;
	pnum = ( screen_mode == MD_SC6 || screen_mode == MD_SC6_256L ) ? 4 : 16;
	for( i = l; i < pnum; ++i ){
		if( palen[ i ] == 1 ) continue;
		r = pal[ i ].red;
		g = pal[ i ].green;
		b = pal[ i ].blue;
		for( j = l; j < pnum; ++j ){
			if( palen[ j ] == 1 ) continue;
			rr = pal[ j ].red;
			gg = pal[ j ].green;
			bb = pal[ j ].blue;
			ok=true;
			m=n;
			if( i != j ){
				for( k = 0; k < n; ++k ){
					if( ( r + rr ) / 2 != ( pal[ tail[ k ].p[ 0 ] ].red   + pal[ tail[ k ].p[ 1 ] ].red   ) / 2 ||
						( g + gg ) / 2 != ( pal[ tail[ k ].p[ 0 ] ].green + pal[ tail[ k ].p[ 1 ] ].green ) / 2 ||
						( b + bb ) / 2 != ( pal[ tail[ k ].p[ 0 ] ].blue  + pal[ tail[ k ].p[ 1 ] ].blue  ) / 2 ) continue;
					if( _distance_itp( r, g, b, rr, gg, bb ) <
						_distance_itp( pal[ tail[ k ].p[ 0 ] ].red,pal[ tail[ k ].p[ 0 ] ].green,pal[ tail[ k ].p[ 0 ] ].blue,
								  pal[ tail[ k ].p[ 1 ] ].red,pal[ tail[ k ].p[ 1 ] ].green,pal[ tail[ k ].p[ 1 ] ].blue ) ){
						m = k;
					}else{
						m = -1;
						break;
					}
				}
			}
			if( m != -1 ){
				tail[ m ].c = GET_RGB( ( convert7to255_r[ r ] + convert7to255_r[ rr ] ) / 2, ( convert7to255_g[ g ] + convert7to255_g[ gg ] ) / 2, ( convert7to255_b[ b ] + convert7to255_b[ bb ] ) / 2 );
				tail[ m ].p[ 0 ] = i;
				tail[ m ].p[ 1 ] = j;
				if( m == n ) ++n;
			}
		}
	}
	return n;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットの大小比較
//	2.	引数
//		Pal1	...	(I)	パレット１のアドレス
//		Pal2	...	(I)	パレット２のアドレス
//	3.	返値
//		true	...	Pal2 の方が小さい
//		false	...	Pal2 の方が大きい
//	4.	備考
//		なし
// -------------------------------------------------------------
static bool cnvCompare( C_PALETTE* Pal1, C_PALETTE* Pal2 )
{
	int	i;
	int	mask;

	mask = 4;
	for( i = 0; i < 3; ++i ) {
		if( ( Pal1->red   & mask ) > ( Pal2->red   & mask ) ) return true;
		if( ( Pal1->red   & mask ) < ( Pal2->red   & mask ) ) return false;
		if( ( Pal1->green & mask ) > ( Pal2->green & mask ) ) return true;
		if( ( Pal1->green & mask ) < ( Pal2->green & mask ) ) return false;
		if( ( Pal1->blue  & mask ) > ( Pal2->blue  & mask ) ) return true;
		if( ( Pal1->blue  & mask ) < ( Pal2->blue  & mask ) ) return false;
		mask = mask >> 1;
	}
	//	同じ
	return false;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットのソート
//	2.	引数
//		Mode	...	(I)	設定内容
//		Pal		...	(I)	パレットの値
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void cnvSortPalette( SETTING* Mode, C_COLOR* Pal )
{
	C_PALETTE			color_palette[16] = { 0 }, tBakCol;	// SC5/SC7 におけるＭＳＸ側パレット指定
	uint8_t		PalEn[16] = { 0 }, tBakEn;
	C_COLOR	pal[16] = { 0 }, tBakPal;
	int			cnt, z, x, pnum;

	//	パレットをコピー
	cnt  = 0;
	pnum = ( Mode->screen_mode == MD_SC6 || Mode->screen_mode == MD_SC6_256L ) ? 4 : 16;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			color_palette[ cnt ]   = Mode->color_palette[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			color_palette[ cnt ]   = Mode->color_palette[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		}
	}
	if( cnt <= 1 ) return;

	//	ソート
	switch( Mode->SortMode ) {
	case SM_INCAUTO:
	case SM_INC:
		for( z = cnt - 1; z >= 0; --z ) {
			for( x = 0; x < z; ++x ) {
				if( cnvCompare( &color_palette[ x ], &color_palette[ x + 1 ] ) ) {
					tBakCol        = color_palette[ x ];
					color_palette[ x ]       = color_palette[ x + 1 ];
					color_palette[ x + 1 ]   = tBakCol;
					tBakEn         = PalEn[ x ];
					PalEn[ x ]     = PalEn[ x + 1 ];
					PalEn[ x + 1 ] = tBakEn;
					tBakPal        = pal[ x ];
					pal[ x ]       = pal[ x + 1 ];
					pal[ x + 1 ]   = tBakPal;
				}
			}
		}
		break;
	case SM_DECAUTO:
	case SM_DEC:
		for( z = cnt - 1; z >= 0; --z ) {
			for( x = 0; x < z; ++x ) {
				if( !cnvCompare( &color_palette[ x ], &color_palette[ x + 1 ] ) ) {
					tBakCol        = color_palette[ x ];
					color_palette[ x ]       = color_palette[ x + 1 ];
					color_palette[ x + 1 ]   = tBakCol;
					tBakEn         = PalEn[ x ];
					PalEn[ x ]     = PalEn[ x + 1 ];
					PalEn[ x + 1 ] = tBakEn;
					tBakPal        = pal[ x ];
					pal[ x ]       = pal[ x + 1 ];
					pal[ x + 1 ]   = tBakPal;
				}
			}
		}
		break;
	}

	//	格納
	cnt = 0;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			Mode->color_palette[ z ]   = color_palette[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Mode->color_palette[ z ]   = color_palette[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		}
	}
}

// -------------------------------------------------------------
//	ユーティリティ

// -------------------------------------------------------------
//	1.	日本語名
//		ピクセル値取得
//	2.	引数
//		in		...	(I)	画像メモリ
//		width	...	(I)	幅
//		height	...	(I)	高さ
//		x		...	(I)	座標
//		y		...	(I)	座標
//	3.	返値
//		色
//	4.	備考
//		なし
// -------------------------------------------------------------
static inline C_COLOR _get_pixel( C_COLOR *in,int width,int height,int x,int y )
{
	// 範囲外修正
	x = range_limiter( x,0,width-1  );
	y = range_limiter( y,0,height-1 );
	// 指定の位置の色を返す
	return( *( in + x + y*width ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		ピクセル配置
//	2.	引数
//		out		...	(I)	画像メモリ
//		width	...	(I)	幅
//		height	...	(I)	高さ
//		x		...	(I)	座標
//		y		...	(I)	座標
//		c		...	(I)	色
// 3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static inline void _set_pixel( C_COLOR *out,int width,int height,int x,int y,C_COLOR c )
{
	// 範囲外修正
	x = range_limiter( x,0,width-1  );
	y = range_limiter( y,0,height-1 );
	// 指定の位置へ色を格納
	*( out + x + y*width )=c;
}

// -------------------------------------------------------------
//	1.	日本語名
//		絶対値を求める
//	2.	引数
//		a	...	(I)	対象
//	3.	返値
//		a の絶対値
//	4.	備考
//		なし
// -------------------------------------------------------------
static inline int _abs( int a )
{
	return( ( a < 0 ) ? -a : a );
}

// -------------------------------------------------------------
//	1.	日本語名
//		２色の距離を求める（遠さ）
//	2.	引数
//		r1	...	(I)	色１赤
//		g1	...	(I)	色１緑
//		b1	...	(I)	色１青
//		r2	...	(I)	色２赤
//		g2	...	(I)	色２緑
//		b2	...	(I)	色２青
//	3.	返値
//		遠さ
//	4.	備考
//		”遠さ”を示す比較用の値であり、スケールに意味はない
// -------------------------------------------------------------
static inline int _distance( int r1, int g1, int b1, int r2, int g2, int b2 )
{
	int r, g, b;
	r = r2 - r1;
	g = g2 - g1;
	b = b2 - b1;
	return( r*r + g*g + b*b );
}

// -------------------------------------------------------------
//	1.	日本語名
//		RGB → ICtCp変換
//	2.	引数
//		p_i		...	(O)	強度
//		p_ct	...	(O)	青黄
//		p_cp	...	(O)	赤緑
//		r		...	(I)	赤 (0-255)
//		g		...	(I)	緑 (0-255)
//		b		...	(I)	青 (0-255)
//	3.	返値
//		遠さ
//	4.	備考
//		”遠さ”を示す比較用の値であり、スケールに意味はない
// -------------------------------------------------------------
static void _rgb_to_itp( double *p_i, double *p_ct, double *p_cp, int r, int g, int b ){
	double l, m, s;

	l = ( 1688. * r + 2146. * g +  262. * b ) / ( 4096. * 255. );
	m = (  683. * r + 2951. * g +  462. * b ) / ( 4096. * 255. );
	s = (   99. * r +  309. * g + 3688. * b ) / ( 4096. * 255. );

	l = 0.5 * sqrt( l );
	m = 0.5 * sqrt( m );
	s = 0.5 * sqrt( s );

	*p_i  = ( 2048. * l + 2048. * m             ) / 4096.;
	*p_ct = ( 3625. * l - 7465. * m + 3840. * s ) / 4096.;
	*p_cp = ( 9500. * l - 9212. * m -  288. * s ) / 4096.;
}

// -------------------------------------------------------------
//	1.	日本語名
//		２色の距離を求める（遠さ） ICtCp (HLG) Version
//	2.	引数
//		r1	...	(I)	色１赤
//		g1	...	(I)	色１緑
//		b1	...	(I)	色１青
//		r2	...	(I)	色２赤
//		g2	...	(I)	色２緑
//		b2	...	(I)	色２青
//	3.	返値
//		遠さ
//	4.	備考
//		”遠さ”を示す比較用の値であり、スケールに意味はない
// -------------------------------------------------------------
static inline int _distance_itp( int r1, int g1, int b1, int r2, int g2, int b2 ){
	double i1, t1, p1, i2, t2, p2, i, t, p;

	_rgb_to_itp( &i1, &t1, &p1, r1, g1, b1 );
	_rgb_to_itp( &i2, &t2, &p2, r2, g2, b2 );
	i = i1 - i2;
	t = t1 - t2;
	p = p1 - p2;
	return( (int)(sqrt(i * i + t * t + p * p) * 65536.) );
}

// -------------------------------------------------------------
//	プレビュー生成

// -------------------------------------------------------------
//	1.	日本語名
//		範囲外数値の補正
//	2.	引数
//		n	...	(I)	数値
//		min	...	(I)	最小値
//		max	...	(I)	最大値
//	3.	返値
//		補正後の n
//	4.	備考
//		なし
// -------------------------------------------------------------
int range_limiter( int n, int min, int max )
{
	if( n < min ) return min;
	if( n > max ) return max;
	return n;
}

// -------------------------------------------------------------
//	1.	日本語名
//		BSAVE 形式で画像を保存する
//	2.	引数
//		bmp		...	(I)	画像データのアドレス
//		width	...	(I)	幅
//		height	...	(I)	高さ
//		inter	...	(I)	インターレース
//		paltbl	...	(I)	パレットテーブルのアドレス( 0 なら付加しない )
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
bool cnvSaveBmpFile( const char *szInFileName, LPBYTE bmp, int width, int height, SETTING *Mode )
{
	HDC					hDC   = NULL, hSDC;
	BITMAPINFOHEADER	bi;
	BITMAPFILEHEADER	bf;
	int					nSize;
	int					w;
	FU_FILE				*hFile = FU_INVALID_HANDLE;
	HBITMAP				hBMP  = NULL;
	char				*pMem = NULL;
	bool				bRet;

	bRet = false;
	//	ﾍｯﾀﾞの作成
	memset( &bf, 0, sizeof( bf ) );
	memset( &bi, 0, sizeof( bi ) );

	w		= ( width * 3 + 3 ) & 0xFFFC;
	nSize	= w * height;

	bf.bfType			= 'MB';
	bf.bfOffBits		= sizeof( bf ) + sizeof( bi );
	bf.bfSize			= sizeof( bf ) + sizeof( bi ) + w * height;

	bi.biBitCount		= 24;
	bi.biCompression	= BI_RGB;
	bi.biHeight			= height;
	bi.biPlanes			= 1;
	bi.biSize			= sizeof( bi );
	bi.biSizeImage		= nSize;
	bi.biWidth			= width;
	bi.biXPelsPerMeter	= 1000;
	bi.biYPelsPerMeter	= 1000;

	//	仮想画面を作成する
	hSDC = GetDC( NULL );
	hDC  = CreateCompatibleDC( hSDC );
	ReleaseDC( NULL, hSDC );
	if( hDC == NULL ) goto exit;
	hBMP = CreateDIBSection( hDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&pMem, NULL, NULL );
	if( hBMP == NULL || pMem == NULL ) goto exit;
	SelectObject( hDC, hBMP );
	GdiFlush();

	draw_screen( bmp, hDC, Mode );
	GdiFlush();

	//	ファイル出力
	hFile = fu_open( szInFileName, "wb" );
	if( hFile == FU_INVALID_HANDLE ) goto exit;

	fu_write( hFile, &bf, sizeof( bf ) );
	fu_write( hFile, &bi, sizeof( bi ) );
	fu_write( hFile, pMem, nSize       );

	bRet = true;
exit:
	//	リソースの解放
	if( hFile!= FU_INVALID_HANDLE ) fu_close( hFile );
	if( hDC  != NULL ) DeleteDC( hDC );
	if( hBMP != NULL ) DeleteObject( hBMP );

	return bRet;
}
