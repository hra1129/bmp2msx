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

const int screen_mode[] = { 2, 3, 5, 6, 7, 8, 10, 12, 5, 6, 7, 8, 10, 12, 8, 8, 2, 3, 5, 6, 7, 5, 6, 7 };	// 画面モード番号

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
	{ 256, 212 },	// screen 8 (V9968 EPAL palette)
	{ 256, 256 },	// screen 8 (V9968 EPAL palette) (256 lines)
	{ 256, 192 },	// screen 2/4 (V9968 EPAL palette)
	{ 64, 48 },	// screen 3 (V9968 EPAL palette)
	{ 256, 212 },	// screen 5 (V9968 EPAL palette)
	{ 512, 212 },	// screen 6 (V9968 EPAL palette)
	{ 512, 212 },	// screen 7 (V9968 EPAL palette)
	{ 256, 256 },	// screen 5 (V9968 EPAL palette) (256 lines)
	{ 512, 256 },	// screen 6 (V9968 EPAL palette) (256 lines)
	{ 512, 256 },	// screen 7 (V9968 EPAL palette) (256 lines)
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
static void _rgb_to_itp( double *p_i, double *p_ct, double *p_cp, int r, int g, int b );

static int cnvCreateHistgram( C_COLOR *in,int size,C_COLOR_TABLE **tbl,C_COLOR *pal,int pp,
							  bool FourceZero,C_COLOR FZColor );
static bool cnvCompare( C_PALETTE* Pal1, C_PALETTE* Pal2 );

static bool cnvRecolor8( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );
static bool cnvRecolor8P( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );
static bool cnvRecolor5( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );

static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, C_COLOR *pal );
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog );
static bool cnvRecolorSc2Pair( C_COLOR *in, int width, int height, unsigned char *out,
							   SETTING *CnvMode, PROGRESS prog, C_COLOR *pal );

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
static inline C_COLOR _create_bit_depth_mask( int seido )
{
	C_COLOR mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );
	return mask;
}

bool cnvCopy( C_COLOR *in ,int inwidth ,int inheight ,
			  C_COLOR *out,int outwidth,int outheight, PROGRESS prog, int seido )
{
	int	x,y,w;
	int	hh;
	C_COLOR mask;

	// 精度制限用マスク作成
	mask = _create_bit_depth_mask( seido );
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
	mask = _create_bit_depth_mask( seido );
	
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
	mask = _create_bit_depth_mask( seido );

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
	mask = _create_bit_depth_mask( seido );

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

	if( CnvMode->screen_mode == MD_SC8P || CnvMode->screen_mode == MD_SC8P_256L ) {
		//	256色パレットビットマップへ変換する (V9968 EPAL)
		return cnvRecolor8P( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );
	} else if( CnvMode->screen_mode == MD_SC8 || CnvMode->screen_mode == MD_SC8_256L ) {
		//	256色ビットマップへ変換する
		return cnvRecolor8( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );
	} else if( ( CnvMode->screen_mode == MD_SC2 || CnvMode->screen_mode == MD_SC2P ) &&
				CnvMode->Sc2Pair && !CnvMode->Tile && width == 256 && height == 192 ) {
		//	SCREEN2/4 ラインペア量子化 (平坦部を2ラインで混合)
		//	タイル使用時・インターレース時 (高さ384) は従来変換へ
		return cnvRecolorSc2Pair( in, width, height, out, CnvMode, prog, pal );
	} else {
		//	16色ビットマップへ変換する
		bRet = cnvRecolor5( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );

		//	特殊な画面（SC2/3/4) にあわせて変換する		
		switch( CnvMode->screen_mode ) {
		case MD_SC2:
		case MD_SC2P:
			bRet = bRet && cnvSC5toSC2( out, prog, pal );
			break;
		case MD_SC3:
		case MD_SC3P:
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
//		減色ルーチン（SCREEN8 V9968 EPAL パレット用）
//	2.	引数
//		in		...	画像入力メモリ（内容は破壊されます）
//		width	...	画像幅
//		height	...	画像高さ
//		out		...	画像出力メモリ（1ドット1バイトのパレット番号）
//		CnvMode	...	現在の設定
//		prog	...	経過表示関数
//		pal		...	パレット（256色）
//	3.	返値
//		false	...	失敗
//		true	...	成功
//	4.	備考
//		cnvRecolor5 の256色版。タイルモードには対応しない。
// -------------------------------------------------------------
static bool cnvRecolor8P( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt ) {
	int x, y, z, ptr;
	int cr, cg, cb;		// 元画素のＲＧＢ
	int pr, pg, pb;		// 画素値
	int er, eg, eb;		// 誤差（正数へ丸める）
	int n;
	double nr, r;
	double pi, pt, pp;	// 画素の ICtCp 値
	double pal_i[ 256 ], pal_t[ 256 ], pal_p[ 256 ];	// パレットの ICtCp 値
	C_COLOR c, cc, *pin;
	C_COLOR fource_zero_color = -1;
	signed short *p_diffusion_error[2] = { nullptr, nullptr };
	signed short *p_x_diffusion_error, *p_y_diffusion_error;
	int diffusion_error_coef = 0;
	int x_diffusion_error_coef = 0;
	int y_diffusion_error_coef = 0;
	C_COLOR bit_depth_mask;
	bool ret = false;

	(void)tail;		// タイルモード未対応
	(void)tailcnt;

	// 誤差拡散係数
	if( CnvMode->diffusion_error_enable ){
		diffusion_error_coef = int( CnvMode->diffusion_error_coef * 1024 );
		x_diffusion_error_coef = int( CnvMode->diffusion_error_x_weight * 256. );
		y_diffusion_error_coef = 256 - x_diffusion_error_coef;
	}

	// 誤差蓄積用バッファの作成
	for( x = 0; x < 2; ++x ) {
		p_diffusion_error[ x ] = (signed short*) malloc( ( width + 5 ) * 4 * sizeof( signed short ) );
		if( p_diffusion_error[ x ]==NULL ) goto l_exit;
		memset( p_diffusion_error[ x ], 0, ( width + 5 ) * 4 * sizeof( signed short ) );
	}

	// 精度制限用マスク作成
	bit_depth_mask = _create_bit_depth_mask( CnvMode->Seido );
	fource_zero_color = CnvMode->FourceZero ? CnvMode->FZColor & bit_depth_mask : -1;

	// パレットの ICtCp 値を先に求めておく（距離計算の高速化）
	for( z = 0; z < 256; ++z ){
		_rgb_to_itp( &pal_i[ z ], &pal_t[ z ], &pal_p[ z ], GET_RED( pal[ z ] ), GET_GREEN( pal[ z ] ), GET_BLUE( pal[ z ] ) );
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

			// 比較する色は実際に表示できる色で無ければならないため、範囲外をクリップする
			pr = range_limiter( cr, 0, 255 );
			pg = range_limiter( cg, 0, 255 );
			pb = range_limiter( cb, 0, 255 );

			// 最も近い色を見つける（_distance_itp の二乗に相当。大小関係は変わらない）
			_rgb_to_itp( &pi, &pt, &pp, pr, pg, pb );
			nr = 1.0e30;
			if( CnvMode->NonZero ) {
				z = 1;
			}
			else {
				z = 0;
			}
			for( ; z < 256; ++z ){
				double di = pi - pal_i[ z ];
				double dt = pt - pal_t[ z ];
				double dp = pp - pal_p[ z ];
				r = di * di + dt * dt + dp * dp;
				if( r < nr ){
					nr = r;
					n  = z;
					cc = pal[ z ];
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
			out[ ptr ] = (unsigned char)n;
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
	bit_depth_mask = _create_bit_depth_mask( CnvMode->Seido );

	// パレット数
	switch( CnvMode->screen_mode ){
	case MD_SC6:
	case MD_SC6_256L:
	case MD_SC6P:
	case MD_SC6P_256L:
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
//	SCREEN2/4 ラインペア量子化 (Sc2Pair)
//
//	平坦・緩勾配領域では、上下 2 ラインを 1 組として量子化し、
//	2 ラインの平均色が目標に近づく色の組を選ぶことで、1 ライン
//	2 色の制約のまま中間色を表現する。平坦領域の判定は 5x5 の
//	ロバストレンジ (P90-P10、全チャンネル最大、閾値 32) と
//	5x5 多数決 (13/25) で行う (固定値、設定項目にはしない)。
//
//	構成はプロトタイプ (sc2_pair.cpp の v3f_bl) と同一である。
//	従来ストリームは cnvRecolor5 の出力そのものを使うため、
//	誤差拡散・ディザパターンはマスク外では従来通り効く。
//	(マスク内のドットにはディザパターンは加算されない。)
//	マスク内はペア量子化＋ペア対応セルフィット、マスク外は
//	従来量子化＋従来セルフィット、境界 (混在) ブロックは
//	従来フィット固定とする。単一走査ループへの統合は将来課題
//	(notes/sc2_pair_impl.md §5 参照)。
// -------------------------------------------------------------
#define SC2P_FLAT_RANGE	32		// 平坦判定の閾値 (5x5 ロバストレンジ)

// ペア量子化の候補色 (cnvRecolor5 の探索集合と同じ)
struct SC2P_CAND {
	int idx;
	int r, g, b;
};

static int _sc2p_build_candidates( SETTING *CnvMode, C_COLOR *pal, SC2P_CAND *cand )
{
	int n = 0;
	int z0 = CnvMode->NonZero ? 1 : 0;
	for( int z = z0; z < 16; ++z ){
		if( CnvMode->PalEn[ z ] == PALEN_NONE ) continue;
		cand[ n ].idx = z;
		cand[ n ].r = GET_RED( pal[ z ] );
		cand[ n ].g = GET_GREEN( pal[ z ] );
		cand[ n ].b = GET_BLUE( pal[ z ] );
		++n;
	}
	return n;
}

// 製品の整数距離 _distance_itp の二乗。
// プロトタイプの dist2_product と同一値である。
static inline double _sc2p_dist2( int r1, int g1, int b1, int r2, int g2, int b2 )
{
	int d = _distance_itp( r1, g1, b1, r2, g2, b2 );
	return (double) d * (double) d;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ペア量子化 (2 ライン同時量子化)
//	2.	引数
//		in		...	画像入力メモリ (256x192)
//		width	...	画像幅
//		height	...	画像高さ
//		CnvMode	...	現在の設定
//		pal		...	パレット
//		weight	...	平均色の重み (SC2=2, SC2P=1)
//		idx		...	出力するパレット番号 (width*height)
//	3.	返値
//		なし
//	4.	備考
//		各ドット列について (a,b) の全組合せを評価し、
//		d(t1,a)^2 + d(t2,b)^2 + w*d(t1+t2,a+b)^2 を最小化する。
//		同値は先勝ち (プロトタイプと同一のタイブレーク)。
// -------------------------------------------------------------
static void _sc2p_pair_quantize( C_COLOR *in, int width, int height, SETTING *CnvMode, C_COLOR *pal,
								 double weight, unsigned char *idx )
{
	SC2P_CAND cand[ 16 ];
	int nc = _sc2p_build_candidates( CnvMode, pal, cand );
	int psum[ 16 ][ 16 ][ 3 ];
	int x, y, a, b;
	C_COLOR fz = CnvMode->FourceZero ? ( CnvMode->FZColor & _create_bit_depth_mask( CnvMode->Seido ) ) : (C_COLOR) -1;

	if( nc <= 0 ) {
		memset( idx, 0, (size_t) width * height );
		return;
	}
	for( a = 0; a < nc; ++a ){
		for( b = 0; b < nc; ++b ){
			psum[ a ][ b ][ 0 ] = cand[ a ].r + cand[ b ].r;
			psum[ a ][ b ][ 1 ] = cand[ a ].g + cand[ b ].g;
			psum[ a ][ b ][ 2 ] = cand[ a ].b + cand[ b ].b;
		}
	}

	for( y = 0; y < height; y += 2 ){
		bool have2 = ( y + 1 < height );
		for( x = 0; x < width; ++x ){
			C_COLOR c1px = in[ (size_t) y * width + x ];
			C_COLOR c2px = have2 ? in[ (size_t) ( y + 1 ) * width + x ] : c1px;
			int t1r = GET_RED( c1px ), t1g = GET_GREEN( c1px ), t1b = GET_BLUE( c1px );
			int t2r = GET_RED( c2px ), t2g = GET_GREEN( c2px ), t2b = GET_BLUE( c2px );
			int tsr = t1r + t2r, tsg = t1g + t2g, tsb = t1b + t2b;
			double e1[ 16 ], e2[ 16 ];
			double best = 0;
			int ba = 0, bb = 0;
			bool first = true;
			int i1, i2;

			for( a = 0; a < nc; ++a ) e1[ a ] = _sc2p_dist2( t1r, t1g, t1b, cand[ a ].r, cand[ a ].g, cand[ a ].b );
			for( b = 0; b < nc; ++b ) e2[ b ] = _sc2p_dist2( t2r, t2g, t2b, cand[ b ].r, cand[ b ].g, cand[ b ].b );
			for( a = 0; a < nc; ++a ){
				for( b = 0; b < nc; ++b ){
					double e = e1[ a ] + e2[ b ];
					if( weight != 0.0 ) e += weight * _sc2p_dist2( tsr, tsg, tsb, psum[ a ][ b ][ 0 ], psum[ a ][ b ][ 1 ], psum[ a ][ b ][ 2 ] );
					if( first || e < best ){
						best = e; ba = a; bb = b; first = false;
					}
				}
			}
			i1 = cand[ ba ].idx;
			i2 = cand[ bb ].idx;
			if( CnvMode->FourceZero ){
				if( pal[ i1 ] == fz ) i1 = 0;
				if( pal[ i2 ] == fz ) i2 = 0;
			}
			idx[ (size_t) y * width + x ] = (unsigned char) i1;
			if( have2 ) idx[ (size_t) ( y + 1 ) * width + x ] = (unsigned char) i2;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		平坦領域マスクの作成
//	2.	引数
//		in		...	画像入力メモリ (256x192)
//		width	...	画像幅
//		height	...	画像高さ
//		mask	...	出力マスク (width*height、1=平坦)
//	3.	返値
//		なし
//	4.	備考
//		チャンネル毎に 5x5 の 25 サンプルをソートし
//		P90-P10 (添字 [21]-[2]) の最大を全チャンネルで取る。
//		閾値以下を平坦とし、5x5 多数決 (>=13/25) で整える。
// -------------------------------------------------------------
static void _sc2p_flat_mask( C_COLOR *in, int width, int height, unsigned char *mask )
{
	unsigned char *raw = (unsigned char*) malloc( (size_t) width * height );
	int x, y, c, dx, dy;
	if( !raw ){
		memset( mask, 0, (size_t) width * height );
		return;
	}
	for( y = 0; y < height; ++y ){
		for( x = 0; x < width; ++x ){
			int worst = 0;
			for( c = 0; c < 3; ++c ){
				int v[ 25 ];
				int n = 0;
				int a, b2, t;
				for( dy = -2; dy <= 2; ++dy ){
					int yy = y + dy; if( yy < 0 ) yy = 0; if( yy >= height ) yy = height - 1;
					for( dx = -2; dx <= 2; ++dx ){
						int xx = x + dx; if( xx < 0 ) xx = 0; if( xx >= width ) xx = width - 1;
						C_COLOR px = in[ (size_t) yy * width + xx ];
						v[ n++ ] = c == 0 ? GET_RED( px ) : c == 1 ? GET_GREEN( px ) : GET_BLUE( px );
					}
				}
				// 挿入ソート (25 要素)
				for( a = 1; a < 25; ++a ){
					t = v[ a ];
					b2 = a - 1;
					while( b2 >= 0 && v[ b2 ] > t ){ v[ b2 + 1 ] = v[ b2 ]; --b2; }
					v[ b2 + 1 ] = t;
				}
				if( v[ 21 ] - v[ 2 ] > worst ) worst = v[ 21 ] - v[ 2 ];
			}
			raw[ (size_t) y * width + x ] = worst <= SC2P_FLAT_RANGE ? 1 : 0;
		}
	}
	for( y = 0; y < height; ++y ){
		for( x = 0; x < width; ++x ){
			int cnt = 0;
			for( dy = -2; dy <= 2; ++dy ){
				int yy = y + dy; if( yy < 0 ) yy = 0; if( yy >= height ) yy = height - 1;
				for( dx = -2; dx <= 2; ++dx ){
					int xx = x + dx; if( xx < 0 ) xx = 0; if( xx >= width ) xx = width - 1;
					cnt += raw[ (size_t) yy * width + xx ];
				}
			}
			mask[ (size_t) y * width + x ] = cnt >= 13 ? 1 : 0;
		}
	}
	free( raw );
}

// -------------------------------------------------------------
//	1.	日本語名
//		セル出力 (cnvSC5toSC2 の末尾と同一の規約)
//	2.	引数
//		dadr	...	VRAM 格納アドレス
//		pal		...	パレット
//		first	...	前景にしたい色 (bits=1)
//		second	...	背景の色 (bits=0)
//		bits	...	8 ドットのビット列 (bit(7-j)=1 で first)
//	3.	返値
//		なし
//	4.	備考
//		明るい色を前景 (パターン bit=1) へ正規化する。
// -------------------------------------------------------------
static void _sc2p_emit_cell( int dadr, C_COLOR *pal, int first, int second, int bits )
{
	int c, p;
	if( first == second ){
		c = first;
		p = 0x00;
		if( _get_palette_luminance( pal, c & 0x0F ) >= ( 128 << 8 ) ){
			p = p ^ 255;
			c = ( ( c << 4 ) & 0xF0 ) | ( ( c >> 4 ) & 0x0F );
		}
	} else {
		c = ( first << 4 ) | second;
		p = bits;
		int lum1 = _get_palette_luminance( pal, c & 0x0F );
		int lum2 = _get_palette_luminance( pal, ( c >> 4 ) & 0x0F );
		if( ( lum1 == lum2 && ( c & 0x0F ) > ( ( c >> 4 ) & 0x0F ) ) || ( lum1 > lum2 ) ){
			p = p ^ 255;
			c = ( ( c << 4 ) & 0xF0 ) | ( ( c >> 4 ) & 0x0F );
		}
	}
	vram[ dadr ] = (unsigned char) p;
	vram[ dadr + SC2COLOR ] = (unsigned char) c;
}

// -------------------------------------------------------------
//	1.	日本語名
//		従来セルフィット (1 ライン分、cnvSC5toSC2 と同一)
//	2.	引数
//		idx		...	パレット番号画像
//		width	...	画像幅
//		y		...	ライン
//		cx		...	セル (8 ドット単位)
//		pal		...	パレット
//		first	...	(出) 最も多い色
//		second	...	(出) 最も異なる色
//		assign	...	(出) 各ドットの割当て色
//	3.	返値
//		なし
// -------------------------------------------------------------
static void _sc2p_fit_line_cell( unsigned char *idx, int width, int y, int cx, C_COLOR *pal,
								 int *first, int *second, int assign[ 8 ] )
{
	int cnt = 0, idx1 = 0, idx2 = 0;
	int ccnt[ 8 ] = { 0 };
	int ccol[ 8 ] = { 0 };
	int i, j;
	for( j = 0; j < 8; ++j ){
		int cc = idx[ (size_t) y * width + cx * 8 + j ];
		for( i = 0; i < cnt; ++i ){
			if( cc == ccol[ i ] ){
				ccnt[ i ]++;
				if( ccnt[ i ] > ccnt[ idx1 ] ) idx1 = i;
				break;
			}
		}
		if( i == cnt ){
			ccol[ cnt ] = cc;
			ccnt[ cnt ] = 1;
			cnt++;
		}
	}
	if( cnt < 2 ){
		*first = *second = ccol[ 0 ];
		for( j = 0; j < 8; ++j ) assign[ j ] = ccol[ 0 ];
		return;
	}
	{
		C_COLOR c1 = pal[ ccol[ idx1 ] ];
		int len = 0;
		for( i = 0; i < cnt; ++i ){
			if( i == idx1 ) continue;
			C_COLOR pc = pal[ ccol[ i ] ];
			int l = _distance_itp( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
								   GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ) );
			if( l >= len ){
				len = l;
				idx2 = i;
			}
		}
		{
			C_COLOR c2 = pal[ ccol[ idx2 ] ];
			*first = ccol[ idx1 ];
			*second = ccol[ idx2 ];
			for( j = 0; j < 8; ++j ){
				int cc = idx[ (size_t) y * width + cx * 8 + j ];
				C_COLOR pc = pal[ cc ];
				assign[ j ] = ( _distance_itp( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
											  GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ) )
							  < _distance_itp( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
											  GET_RED( c2 ), GET_GREEN( c2 ), GET_BLUE( c2 ) ) )
							? ccol[ idx1 ] : ccol[ idx2 ];
			}
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		ペア対応セルフィット (8x2 ブロック)
//	2.	引数
//		in		...	画像入力メモリ
//		idx		...	パレット番号画像 (合成済み)
//		width	...	画像幅
//		y		...	ブロック先頭ライン (偶数)
//		cx		...	セル (8 ドット単位)
//		CnvMode	...	現在の設定
//		pal		...	パレット
//		colw	...	列毎のペア重み (マスク内=weight、外=0)
//		best_pair	...	(出) ライン毎の選出 2 色
//		best_assign	...	(出) 各ドットの割当て色
//	3.	返値
//		true	...	フィット成功
//		false	...	候補が無くフィット不能 (呼び出し側は従来フィットへ)
//	4.	備考
//		ライン毎に頻度上位 5 色までの短縮リストを作り、
//		ペア (e1,e2)x(o1,o2) の全組合せを評価して、列毎の
//		4 色同時割当てでブロック合計誤差を最小化する。
//		候補に無い番号 (FourceZero の 0 など) だけの列は
//		組合せが成立しないため false を返す。
// -------------------------------------------------------------
static bool _sc2p_fit_pair_block( C_COLOR *in, unsigned char *idx, int width, int y, int cx,
								  SETTING *CnvMode, C_COLOR *pal, const double colw[ 8 ],
								  int best_pair[ 2 ][ 2 ], int best_assign[ 2 ][ 8 ] )
{
	SC2P_CAND cand[ 16 ];
	int nc = _sc2p_build_candidates( CnvMode, pal, cand );
	int list[ 2 ][ 5 ];
	int nlist[ 2 ] = { 0, 0 };
	int ln, j, e1, e2, o1, o2;
	double best_total = 0;
	bool first = true;
	int assign[ 2 ][ 8 ];

	if( nc <= 0 ) return false;
	for( ln = 0; ln < 2; ++ln ){
		int yy = y + ln;
		int order[ 16 ], freq[ 16 ], no = 0;
		int a, b2, t;
		for( j = 0; j < 8; ++j ){
			int cc = idx[ (size_t) yy * width + cx * 8 + j ];
			int i;
			for( i = 0; i < no; ++i ) if( order[ i ] == cc ){ freq[ i ]++; break; }
			if( i == no ){ order[ no ] = cc; freq[ no ] = 1; no++; }
		}
		// 頻度降順の安定ソート
		for( a = 0; a < no; ++a ){
			for( b2 = a + 1; b2 < no; ++b2 ){
				if( freq[ b2 ] > freq[ a ] ){
					t = order[ a ]; order[ a ] = order[ b2 ]; order[ b2 ] = t;
					t = freq[ a ]; freq[ a ] = freq[ b2 ]; freq[ b2 ] = t;
				}
			}
		}
		for( int i = 0; i < no && nlist[ ln ] < 5; ++i ) list[ ln ][ nlist[ ln ]++ ] = order[ i ];
		// 各ドットの最近傍色を補う
		for( j = 0; j < 8 && nlist[ ln ] < 5; ++j ){
			C_COLOR px = in[ (size_t) yy * width + cx * 8 + j ];
			int besti = -1;
			double bestd = 0;
			for( int ci = 0; ci < nc; ++ci ){
				double d = _sc2p_dist2( GET_RED( px ), GET_GREEN( px ), GET_BLUE( px ), cand[ ci ].r, cand[ ci ].g, cand[ ci ].b );
				if( besti < 0 || d < bestd ){ bestd = d; besti = cand[ ci ].idx; }
			}
			bool have = false;
			for( int i = 0; i < nlist[ ln ]; ++i ) if( list[ ln ][ i ] == besti ) have = true;
			if( !have && besti >= 0 ) list[ ln ][ nlist[ ln ]++ ] = besti;
		}
		if( nlist[ ln ] == 0 ) list[ ln ][ nlist[ ln ]++ ] = cand[ 0 ].idx;
	}

	for( e1 = 0; e1 < nlist[ 0 ]; ++e1 )
	for( e2 = e1; e2 < nlist[ 0 ]; ++e2 )
	for( o1 = 0; o1 < nlist[ 1 ]; ++o1 )
	for( o2 = o1; o2 < nlist[ 1 ]; ++o2 ){
		int ep[ 2 ] = { list[ 0 ][ e1 ], list[ 0 ][ e2 ] };
		int op[ 2 ] = { list[ 1 ][ o1 ], list[ 1 ][ o2 ] };
		SC2P_CAND *epc[ 2 ] = { nullptr, nullptr };
		SC2P_CAND *opc[ 2 ] = { nullptr, nullptr };
		double total = 0;
		for( int k = 0; k < nc; ++k ){
			if( cand[ k ].idx == ep[ 0 ] ) epc[ 0 ] = &cand[ k ];
			if( cand[ k ].idx == ep[ 1 ] ) epc[ 1 ] = &cand[ k ];
			if( cand[ k ].idx == op[ 0 ] ) opc[ 0 ] = &cand[ k ];
			if( cand[ k ].idx == op[ 1 ] ) opc[ 1 ] = &cand[ k ];
		}
		if( !epc[ 0 ] || !epc[ 1 ] || !opc[ 0 ] || !opc[ 1 ] ) continue;
		for( j = 0; j < 8; ++j ){
			C_COLOR ca = in[ (size_t) y * width + cx * 8 + j ];
			C_COLOR cb = in[ (size_t) ( y + 1 ) * width + cx * 8 + j ];
			int t1r = GET_RED( ca ), t1g = GET_GREEN( ca ), t1b = GET_BLUE( ca );
			int t2r = GET_RED( cb ), t2g = GET_GREEN( cb ), t2b = GET_BLUE( cb );
			int tsr = t1r + t2r, tsg = t1g + t2g, tsb = t1b + t2b;
			double weff = colw[ j ];
			double bj = 0;
			int ba = 0, bb = 0;
			bool fj = true;
			for( int a = 0; a < 2; ++a ){
				for( int b = 0; b < 2; ++b ){
					double e = _sc2p_dist2( t1r, t1g, t1b, epc[ a ]->r, epc[ a ]->g, epc[ a ]->b )
							 + _sc2p_dist2( t2r, t2g, t2b, opc[ b ]->r, opc[ b ]->g, opc[ b ]->b );
					if( weff != 0.0 ) e += weff * _sc2p_dist2( tsr, tsg, tsb, epc[ a ]->r + opc[ b ]->r, epc[ a ]->g + opc[ b ]->g, epc[ a ]->b + opc[ b ]->b );
					if( fj || e < bj ){ bj = e; ba = a; bb = b; fj = false; }
				}
			}
			total += bj;
			assign[ 0 ][ j ] = ep[ ba ];
			assign[ 1 ][ j ] = op[ bb ];
		}
		if( first || total < best_total ){
			best_total = total;
			first = false;
			memcpy( best_assign, assign, sizeof( assign ) );
			best_pair[ 0 ][ 0 ] = ep[ 0 ];
			best_pair[ 0 ][ 1 ] = ep[ 1 ];
			best_pair[ 1 ][ 0 ] = op[ 0 ];
			best_pair[ 1 ][ 1 ] = op[ 1 ];
		}
	}
	return !first;
}

// -------------------------------------------------------------
//	1.	日本語名
//		SCREEN2/4 ラインペア量子化の全体処理
//	2.	引数
//		in		...	画像入力メモリ (256x192)
//		width	...	画像幅 (256)
//		height	...	画像高さ (192)
//		out		...	画像出力メモリ (VRAM 0x4000)
//		CnvMode	...	現在の設定
//		prog	...	経過表示関数
//		pal		...	パレット
//	3.	返値
//		false	...	失敗
//		true	...	成功
//	4.	備考
//		従来ストリームは cnvRecolor5 の出力をニブル展開して
//		得る (誤差拡散・ディザパターンを含む)。ペアストリーム
//		は _sc2p_pair_quantize で得る。マスクで両者を合成し、
//		8x2 ブロック毎にフィットして vram へ出力する。
//		境界ブロックは従来フィット固定 (v3f_bl)。
// -------------------------------------------------------------
static bool cnvRecolorSc2Pair( C_COLOR *in, int width, int height, unsigned char *out,
							   SETTING *CnvMode, PROGRESS prog, C_COLOR *pal )
{
	double weight = ( CnvMode->screen_mode == MD_SC2P ) ? 1.0 : 2.0;
	unsigned char *tmp = nullptr;
	unsigned char *idx_pair = nullptr;
	unsigned char *idx = nullptr;
	unsigned char *mask = nullptr;
	unsigned char *pairmode = nullptr;
	bool ret = false;
	int x, y;

	{
		SC2P_CAND cand[ 16 ];
		if( _sc2p_build_candidates( CnvMode, pal, cand ) <= 0 ){
			// 候補色が無い場合は従来変換へ
			if( !cnvRecolor5( in, width, height, out, CnvMode, prog, pal, nullptr, 0 ) ) return false;
			return cnvSC5toSC2( out, prog, pal );
		}
	}

	tmp = (unsigned char*) malloc( (size_t) width * height / 2 );
	idx_pair = (unsigned char*) malloc( (size_t) width * height );
	idx = (unsigned char*) malloc( (size_t) width * height );
	mask = (unsigned char*) malloc( (size_t) width * height );
	pairmode = (unsigned char*) malloc( (size_t) ( height / 2 ) * width );
	if( !tmp || !idx_pair || !idx || !mask || !pairmode ) goto l_exit;

	// 従来ストリーム (誤差拡散・ディザパターンを含む)
	if( !cnvRecolor5( in, width, height, tmp, CnvMode, prog, pal, nullptr, 0 ) ) goto l_exit;
	// ニブル展開
	for( int i = 0; i < width * height; ++i ){
		unsigned char b = tmp[ i >> 1 ];
		idx[ i ] = ( i & 1 ) ? ( b & 0x0F ) : ( ( b >> 4 ) & 0x0F );
	}

	// ペアストリーム
	_sc2p_pair_quantize( in, width, height, CnvMode, pal, weight, idx_pair );
	// 平坦マスク
	_sc2p_flat_mask( in, width, height, mask );
	// 合成: マスク内のドット列はペアストリームで置換
	for( y = 0; y + 1 < height; y += 2 ){
		for( x = 0; x < width; ++x ){
			bool pm = mask[ (size_t) y * width + x ] && mask[ (size_t) ( y + 1 ) * width + x ];
			pairmode[ (size_t) ( y / 2 ) * width + x ] = pm ? 1 : 0;
			if( pm ){
				idx[ (size_t) y * width + x ] = idx_pair[ (size_t) y * width + x ];
				idx[ (size_t) ( y + 1 ) * width + x ] = idx_pair[ (size_t) ( y + 1 ) * width + x ];
			}
		}
	}

	// セルフィット＋出力
	memset( vram, 0, sizeof( vram ) );
	for( y = 0; y + 1 < height; y += 2 ){
		for( int cx = 0; cx < 32; ++cx ){
			int nmask = 0;
			int ln, j;
			for( ln = 0; ln < 2; ++ln ){
				for( j = 0; j < 8; ++j ){
					if( mask[ (size_t) ( y + ln ) * width + cx * 8 + j ] ) nmask++;
				}
			}
			if( nmask == 16 ){
				double colw[ 8 ];
				int bp[ 2 ][ 2 ];
				int ba[ 2 ][ 8 ];
				for( j = 0; j < 8; ++j ) colw[ j ] = pairmode[ (size_t) ( y / 2 ) * width + cx * 8 + j ] ? weight : 0.0;
				if( _sc2p_fit_pair_block( in, idx, width, y, cx, CnvMode, pal, colw, bp, ba ) ){
					for( ln = 0; ln < 2; ++ln ){
						int yy = y + ln;
						int bits = 0;
						for( j = 0; j < 8; ++j ){
							bits <<= 1;
							if( ba[ ln ][ j ] == bp[ ln ][ 0 ] ) bits |= 1;
						}
						int dadr = ( yy & 7 ) + ( cx * 8 ) + ( yy >> 3 ) * 256;
						_sc2p_emit_cell( dadr, pal, bp[ ln ][ 0 ], bp[ ln ][ 1 ], bits );
					}
					continue;
				}
				// ペアフィット不能の場合は従来フィットへ落とす
			}
			for( ln = 0; ln < 2; ++ln ){
				int yy = y + ln;
				int first_c, second_c;
				int assign[ 8 ];
				int bits = 0;
				_sc2p_fit_line_cell( idx, width, yy, cx, pal, &first_c, &second_c, assign );
				for( j = 0; j < 8; ++j ){
					bits <<= 1;
					if( assign[ j ] == first_c ) bits |= 1;
				}
				int dadr = ( yy & 7 ) + ( cx * 8 ) + ( yy >> 3 ) * 256;
				_sc2p_emit_cell( dadr, pal, first_c, second_c, bits );
			}
		}
	}
	memcpy( out, vram, sizeof( vram ) );
	ret = true;
l_exit:
	if( tmp ) free( tmp );
	if( idx_pair ) free( idx_pair );
	if( idx ) free( idx );
	if( mask ) free( mask );
	if( pairmode ) free( pairmode );
	return ret;
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
		b = convert3to255_s8b[ i & 3 ];
		pal[ i ] = GET_RGB( r, g, b );
	}
	return true;
}

// -------------------------------------------------------------
//	RGB各5bit 色空間のユーティリティ (SCREEN8 EPAL用)

// 5bit値 → 8bit値 変換テーブル ( v * 255 / 31 )
static const int epal_convert31to255[ 32 ] = {
	 0,  8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98,106,115,123,
	131,139,148,156,164,172,180,189,197,205,213,222,230,238,246,255
};

// 8bit値 → 最も近い 5bit値
static int epal_quantize31( int v )
{
	return convert_rgb_to_palette( epal_convert31to255, 32, v );
}

// メディアンカット用の色ビン
struct C_EPAL_BIN {
	int		r, g, b;		// RGB各5bit
	int		n;				// その数
};

// -------------------------------------------------------------
//	EPAL 自動選択パレットの重複排除 (後処理)
//	自動選択された色の中に同値の色が複数あると、パレット枠が
//	無駄になる。重複の判定は、選択結果が最終的に記録される
//	形式で行う。
//	- cnvGetPaletteEpalN (EpalHGuard OFF): CreatePal の各コピーは
//	  AUTO 項目を 3bit へ引き戻してから 5bit へ再導出して記録
//	  するため、5bit では異なる色同士でも記録後に同値になり
//	  うる。記録変換 (引き戻し→再導出) を通した値で判定する。
//	  記録変換は 5bit 値の関数なので、この重複を無くせば
//	  5bit そのものの重複も無くなる。
//	- cnvGetPaletteEpalWu / cnvGetPalette256: 記録は 5bit の
//	  ままなので、5bit 値そのもので判定する。
//	重複したスロットは、現在のパレット (自動選択分＋固定色)
//	から最も遠いヒストグラムビン (画素数 × 距離^2 が最大) で
//	置き換える。同点の場合はビン番号の小さい方を採る決定的
//	動作。置換候補が尽きた場合はそのスロットをそのまま残す。
// -------------------------------------------------------------
static int _epal_recorded_channel( const int *tbl, int v5 )
{
	int idx = convert_rgb_to_palette( tbl, 8, epal_convert31to255[ v5 & 31 ] );
	return ( tbl[ idx ] * 31 + 127 ) / 255;
}

static void _epal_recorded5( int r5, int g5, int b5, int *out )
{
	out[ 0 ] = _epal_recorded_channel( convert7to255_r, r5 );
	out[ 1 ] = _epal_recorded_channel( convert7to255_g, g5 );
	out[ 2 ] = _epal_recorded_channel( convert7to255_b, b5 );
}

static void _epal_dedup_palette( C_EPAL_BIN *bins, int nbin, C_COLOR *pal, int count,
								 const C_COLOR *fixed, int fixed_num, bool key_recorded )
{
	int (*pal5)[ 3 ] = nullptr, (*rec5)[ 3 ] = nullptr;
	int (*f5)[ 3 ] = nullptr, (*frec)[ 3 ] = nullptr;
	int (*key)[ 3 ], (*fkey)[ 3 ];
	int i, j, k, b;

	if( count <= 0 ) return;
	pal5 = (int (*)[ 3 ]) malloc( sizeof( int ) * 3 * count );
	rec5 = (int (*)[ 3 ]) malloc( sizeof( int ) * 3 * count );
	if( fixed_num > 0 ){
		f5 = (int (*)[ 3 ]) malloc( sizeof( int ) * 3 * fixed_num );
		frec = (int (*)[ 3 ]) malloc( sizeof( int ) * 3 * fixed_num );
	}
	if( pal5 == NULL || rec5 == NULL || ( fixed_num > 0 && ( f5 == NULL || frec == NULL ) ) ){
		free( pal5 ); free( rec5 ); free( f5 ); free( frec );
		return;		// 失敗時は選択結果をそのまま使う
	}
	for( i = 0; i < count; ++i ){
		pal5[ i ][ 0 ] = epal_quantize31( GET_RED( pal[ i ] ) );
		pal5[ i ][ 1 ] = epal_quantize31( GET_GREEN( pal[ i ] ) );
		pal5[ i ][ 2 ] = epal_quantize31( GET_BLUE( pal[ i ] ) );
		_epal_recorded5( pal5[ i ][ 0 ], pal5[ i ][ 1 ], pal5[ i ][ 2 ], rec5[ i ] );
	}
	for( k = 0; k < fixed_num; ++k ){
		f5[ k ][ 0 ] = epal_quantize31( GET_RED( fixed[ k ] ) );
		f5[ k ][ 1 ] = epal_quantize31( GET_GREEN( fixed[ k ] ) );
		f5[ k ][ 2 ] = epal_quantize31( GET_BLUE( fixed[ k ] ) );
		_epal_recorded5( f5[ k ][ 0 ], f5[ k ][ 1 ], f5[ k ][ 2 ], frec[ k ] );
	}
	key = key_recorded ? rec5 : pal5;
	fkey = key_recorded ? frec : f5;

	for( i = 0; i < count; ++i ){
		bool dup = false;
		for( j = 0; j < i && !dup; ++j ){
			if( key[ j ][ 0 ] == key[ i ][ 0 ] && key[ j ][ 1 ] == key[ i ][ 1 ] && key[ j ][ 2 ] == key[ i ][ 2 ] ) dup = true;
		}
		for( k = 0; k < fixed_num && !dup; ++k ){
			if( fkey[ k ][ 0 ] == key[ i ][ 0 ] && fkey[ k ][ 1 ] == key[ i ][ 1 ] && fkey[ k ][ 2 ] == key[ i ][ 2 ] ) dup = true;
		}
		if( !dup ) continue;

		// 置換候補: キーが現在のどのキーとも重ならず、サービスの
		// 悪さ (画素数 × 最寄りキーまでの距離^2) が最大のビン
		int best_bin = -1;
		long best_score = 0;
		int best_key[ 3 ];
		for( b = 0; b < nbin; ++b ){
			int bkey[ 3 ];
			long dmin = -1;
			bool clash = false;
			if( key_recorded ) _epal_recorded5( bins[ b ].r, bins[ b ].g, bins[ b ].b, bkey );
			else { bkey[ 0 ] = bins[ b ].r; bkey[ 1 ] = bins[ b ].g; bkey[ 2 ] = bins[ b ].b; }
			for( j = 0; j < count && !clash; ++j ){
				long dr, dg, db, d2;
				if( j == i ) continue;
				if( key[ j ][ 0 ] == bkey[ 0 ] && key[ j ][ 1 ] == bkey[ 1 ] && key[ j ][ 2 ] == bkey[ 2 ] ) clash = true;
				dr = (long)bkey[ 0 ] - key[ j ][ 0 ];
				dg = (long)bkey[ 1 ] - key[ j ][ 1 ];
				db = (long)bkey[ 2 ] - key[ j ][ 2 ];
				d2 = dr * dr + dg * dg + db * db;
				if( dmin < 0 || d2 < dmin ) dmin = d2;
			}
			for( k = 0; k < fixed_num && !clash; ++k ){
				long dr, dg, db, d2;
				if( fkey[ k ][ 0 ] == bkey[ 0 ] && fkey[ k ][ 1 ] == bkey[ 1 ] && fkey[ k ][ 2 ] == bkey[ 2 ] ) clash = true;
				dr = (long)bkey[ 0 ] - fkey[ k ][ 0 ];
				dg = (long)bkey[ 1 ] - fkey[ k ][ 1 ];
				db = (long)bkey[ 2 ] - fkey[ k ][ 2 ];
				d2 = dr * dr + dg * dg + db * db;
				if( dmin < 0 || d2 < dmin ) dmin = d2;
			}
			if( clash || dmin <= 0 ) continue;
			{
				long score = (long)bins[ b ].n * dmin;
				if( score > best_score ){
					best_score = score;
					best_bin = b;
					best_key[ 0 ] = bkey[ 0 ];
					best_key[ 1 ] = bkey[ 1 ];
					best_key[ 2 ] = bkey[ 2 ];
				}
			}
		}
		if( best_bin >= 0 ){
			pal[ i ] = GET_RGB( epal_convert31to255[ bins[ best_bin ].r ],
								epal_convert31to255[ bins[ best_bin ].g ],
								epal_convert31to255[ bins[ best_bin ].b ] );
			pal5[ i ][ 0 ] = bins[ best_bin ].r;
			pal5[ i ][ 1 ] = bins[ best_bin ].g;
			pal5[ i ][ 2 ] = bins[ best_bin ].b;
			_epal_recorded5( pal5[ i ][ 0 ], pal5[ i ][ 1 ], pal5[ i ][ 2 ], rec5[ i ] );
		}
	}
	free( pal5 ); free( rec5 ); free( f5 ); free( frec );
}

// メディアンカット用のボックス
struct C_EPAL_BOX {
	int		lo;				// ビン配列の開始位置
	int		hi;				// ビン配列の終了位置（この値の手前まで）
	long	pop;			// 総画素数
	int		rmin, rmax, gmin, gmax, bmin, bmax;
};

static void _epal_box_update( C_EPAL_BOX *box, C_EPAL_BIN *bins )
{
	int i;

	box->pop = 0;
	box->rmin = box->gmin = box->bmin = 31;
	box->rmax = box->gmax = box->bmax = 0;
	for( i = box->lo; i < box->hi; ++i ){
		box->pop += bins[ i ].n;
		if( bins[ i ].r < box->rmin ) box->rmin = bins[ i ].r;
		if( bins[ i ].r > box->rmax ) box->rmax = bins[ i ].r;
		if( bins[ i ].g < box->gmin ) box->gmin = bins[ i ].g;
		if( bins[ i ].g > box->gmax ) box->gmax = bins[ i ].g;
		if( bins[ i ].b < box->bmin ) box->bmin = bins[ i ].b;
		if( bins[ i ].b > box->bmax ) box->bmax = bins[ i ].b;
	}
}

static int _epal_bin_key( const C_EPAL_BIN *bin, int channel )
{
	switch( channel ){
	case 0:		return bin->r;
	case 1:		return bin->g;
	default:	return bin->b;
	}
}

static int epal_sort_channel;	// ソート中のチャンネル（_epal_bin_compare 用）

static int _epal_bin_compare( const void *a, const void *b )
{
	return _epal_bin_key( (const C_EPAL_BIN*)a, epal_sort_channel ) - _epal_bin_key( (const C_EPAL_BIN*)b, epal_sort_channel );
}

// -------------------------------------------------------------
//	1.	日本語名
//		減色用パレット（256色・RGB各5bit）を求める (SCREEN8 EPAL用)
//	2.	引数
//		in			...	(I)	画像
//		width		...	(I)	幅
//		height		...	(I)	高さ
//		pal			...	(O)	パレット（256個）のアドレス
//		p_pal_count	...	(O)	採用したパレット数を格納するアドレス
//		FourceZero	...	(I)	強制ゼロ化の色がある true / ない false
//		FZColor		...	(I)	強制ゼロ化の色
//	3.	返値
//		成功 true / 失敗 false
//	4.	備考
//		色空間は RGB各5bit (32768色)。使われている色が256色
//		以下の場合は多い順にそのまま採用し、多い場合はメディ
//		アンカット法で256色へ減色する。
// -------------------------------------------------------------
bool cnvGetPalette256( C_COLOR *in, int width, int height, C_COLOR *pal, int *p_pal_count,
					   bool FourceZero, C_COLOR FZColor )
{
	uint32_t *hist = nullptr;
	C_EPAL_BIN *bins = nullptr;
	C_EPAL_BOX boxes[ 256 ];
	int i, j, nbin, nbox;
	C_COLOR c;

	*p_pal_count = 0;
	ZeroMemory( pal, sizeof( C_COLOR ) * 256 );

	// ヒストグラムの作成 (RGB各5bit)
	hist = (uint32_t*) malloc( sizeof( uint32_t ) * 32768 );
	if( hist == NULL ) return false;
	memset( hist, 0, sizeof( uint32_t ) * 32768 );
	for( i = 0; i < width * height; ++i ){
		c = in[ i ];
		if( FourceZero && c == FZColor ) continue;
		hist[ ( epal_quantize31( GET_RED( c ) ) << 10 ) | ( epal_quantize31( GET_GREEN( c ) ) << 5 ) | epal_quantize31( GET_BLUE( c ) ) ]++;
	}
	nbin = 0;
	for( i = 0; i < 32768; ++i ){
		if( hist[ i ] ) ++nbin;
	}
	if( nbin == 0 ){
		free( hist );
		return false;
	}
	bins = (C_EPAL_BIN*) malloc( sizeof( C_EPAL_BIN ) * nbin );
	if( bins == NULL ){
		free( hist );
		return false;
	}
	nbin = 0;
	for( i = 0; i < 32768; ++i ){
		if( hist[ i ] == 0 ) continue;
		bins[ nbin ].r = ( i >> 10 ) & 31;
		bins[ nbin ].g = ( i >>  5 ) & 31;
		bins[ nbin ].b =   i        & 31;
		bins[ nbin ].n = (int)hist[ i ];
		++nbin;
	}
	free( hist );
	hist = nullptr;

	if( nbin <= 256 ){
		// 使われている色が256色以下なら、多い順にそのまま採用する
		C_COLOR_TABLE *tbl = (C_COLOR_TABLE*) malloc( sizeof( C_COLOR_TABLE ) * nbin );
		if( tbl == NULL ){
			free( bins );
			return false;
		}
		for( i = 0; i < nbin; ++i ){
			tbl[ i ].c = GET_RGB( epal_convert31to255[ bins[ i ].r ], epal_convert31to255[ bins[ i ].g ], epal_convert31to255[ bins[ i ].b ] );
			tbl[ i ].n = bins[ i ].n;
		}
		qsort( tbl, nbin, sizeof( C_COLOR_TABLE ), cnvColorTblCompare );
		for( i = 0; i < nbin; ++i ){
			pal[ i ] = tbl[ i ].c;
		}
		*p_pal_count = nbin;
		free( tbl );
		free( bins );
		return true;
	}

	// メディアンカット法で256個のボックスへ分割する
	boxes[ 0 ].lo = 0;
	boxes[ 0 ].hi = nbin;
	_epal_box_update( &boxes[ 0 ], bins );
	nbox = 1;
	while( nbox < 256 ){
		// 分割するボックスを選ぶ（画素数が最大で、色の広がりがあるもの）
		int target = -1;
		long target_pop = -1;
		int range, channel, split;
		long acc, half;
		C_EPAL_BOX *box;

		for( i = 0; i < nbox; ++i ){
			if( boxes[ i ].hi - boxes[ i ].lo < 2 ) continue;
			range = boxes[ i ].rmax - boxes[ i ].rmin;
			if( boxes[ i ].gmax - boxes[ i ].gmin > range ) range = boxes[ i ].gmax - boxes[ i ].gmin;
			if( boxes[ i ].bmax - boxes[ i ].bmin > range ) range = boxes[ i ].bmax - boxes[ i ].bmin;
			if( range == 0 ) continue;
			if( boxes[ i ].pop > target_pop ){
				target_pop = boxes[ i ].pop;
				target = i;
			}
		}
		if( target < 0 ) break;		// これ以上分割できない

		// 最も広がりの大きいチャンネルで、画素数の中央値のところで分割する
		box = &boxes[ target ];
		channel = 0;
		range = box->rmax - box->rmin;
		if( box->gmax - box->gmin > range ){ channel = 1; range = box->gmax - box->gmin; }
		if( box->bmax - box->bmin > range ){ channel = 2; range = box->bmax - box->bmin; }
		epal_sort_channel = channel;
		qsort( bins + box->lo, box->hi - box->lo, sizeof( C_EPAL_BIN ), _epal_bin_compare );
		half = box->pop / 2;
		acc = 0;
		split = box->hi - 1;
		for( i = box->lo; i < box->hi - 1; ++i ){
			acc += bins[ i ].n;
			if( acc >= half ){
				split = i + 1;
				break;
			}
		}
		boxes[ nbox ].lo = split;
		boxes[ nbox ].hi = box->hi;
		box->hi = split;
		_epal_box_update( box, bins );
		_epal_box_update( &boxes[ nbox ], bins );
		++nbox;
	}

	// 各ボックスの平均色をパレットにする
	for( i = 0; i < nbox; ++i ){
		long rsum = 0, gsum = 0, bsum = 0;
		int r5, g5, b5;
		for( j = boxes[ i ].lo; j < boxes[ i ].hi; ++j ){
			rsum += (long)bins[ j ].r * bins[ j ].n;
			gsum += (long)bins[ j ].g * bins[ j ].n;
			bsum += (long)bins[ j ].b * bins[ j ].n;
		}
		r5 = (int)( ( rsum + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		g5 = (int)( ( gsum + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		b5 = (int)( ( bsum + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		pal[ i ] = GET_RGB( epal_convert31to255[ r5 ], epal_convert31to255[ g5 ], epal_convert31to255[ b5 ] );
	}
	_epal_dedup_palette( bins, nbin, pal, nbox, NULL, 0, false );
	*p_pal_count = nbox;
	free( bins );
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		EPALモード判定 (SCREEN8 EPALを除く)
//	2.	引数
//		screen_mode	...	(I)	画面モード
//	3.	返値
//		SCREEN2/3/5/6/7 の EPALモードなら true
// -------------------------------------------------------------
bool cnvIsEpalMode( uint32_t screen_mode )
{
	return screen_mode >= MD_SC2P && screen_mode <= MD_SC7P_256L;
}

// -------------------------------------------------------------
//	1.	日本語名
//		EPALモードに対応するベースモード
//	2.	引数
//		screen_mode	...	(I)	画面モード
//	3.	返値
//		ベースモード (非EPALモードはそのまま)
// -------------------------------------------------------------
uint32_t cnvEpalBaseMode( uint32_t screen_mode )
{
	switch( screen_mode ) {
	case MD_SC2P:		return MD_SC2;
	case MD_SC3P:		return MD_SC3;
	case MD_SC5P:		return MD_SC5;
	case MD_SC6P:		return MD_SC6;
	case MD_SC7P:		return MD_SC7;
	case MD_SC5P_256L:	return MD_SC5_256L;
	case MD_SC6P_256L:	return MD_SC6_256L;
	case MD_SC7P_256L:	return MD_SC7_256L;
	}
	return screen_mode;
}

// -------------------------------------------------------------
//	1.	日本語名
//		EPALモードの色数
//	2.	引数
//		screen_mode	...	(I)	画面モード
//	3.	返値
//		色数 (SCREEN6系=4, SCREEN8 EPAL=256, その他EPAL=16, 非EPAL時は0)
// -------------------------------------------------------------
int cnvEpalColorCount( uint32_t screen_mode )
{
	if( screen_mode == MD_SC8P || screen_mode == MD_SC8P_256L ) return 256;
	if( !cnvIsEpalMode( screen_mode ) ) return 0;
	switch( cnvEpalBaseMode( screen_mode ) ) {
	case MD_SC6:
	case MD_SC6_256L:
		return 4;
	}
	return 16;
}

// -------------------------------------------------------------
//	1.	日本語名
//		EPAL 5bit値を8bitへ展開する
//	2.	引数
//		v5	...	(I)	5bit値 (0～31)
//	3.	返値
//		8bit値 (floor(v*255/31))
// -------------------------------------------------------------
int cnvEpalTo255( int v5 )
{
	return epal_convert31to255[ v5 & 31 ];
}

// -------------------------------------------------------------
//	1.	日本語名
//		減色用パレット（最大16色・RGB各5bit）を求める
//	2.	引数
//		in			...	(I)	画像メモリ（読み取り専用）
//		width		...	(I)	幅
//		height		...	(I)	高さ
//		pal			...	(O)	パレット（自動選択分のみ）のアドレス
//		p_pal_count	...	(O)	採用したパレット数を格納するアドレス
//		max_colors	...	(I)	パレット総数の上限 (固定色を含む)
//		fixed		...	(I)	固定色の配列 (選択対象から除外する)
//		fixed_num	...	(I)	fixed の数
//		FourceZero	...	(I)	強制ゼロ化の色がある true / ない false
//		FZColor		...	(I)	強制ゼロ化の色
//	3.	返値
//		成功 true / 失敗 false
//	4.	備考
//		cnvGetPalette256 (SCREEN8 EPAL用) の少数色版。色空間は
//		RGB各5bit (32768色)。使われている色が上限以下なら多い順に
//		そのまま採用し、多い場合はメディアンカット法で減色する。
//		固定色と量子化後に重なる色は選択対象から除外される。
// -------------------------------------------------------------
bool cnvGetPaletteEpalN( C_COLOR *in, int width, int height, C_COLOR *pal, int *p_pal_count,
						 int max_colors, const C_COLOR *fixed, int fixed_num,
						 bool FourceZero, C_COLOR FZColor )
{
	uint32_t *hist = nullptr;
	C_EPAL_BIN *bins = nullptr;
	C_EPAL_BOX boxes[ 16 ];
	int i, j, nbin, nbox;
	int target;
	C_COLOR c;

	*p_pal_count = 0;
	ZeroMemory( pal, sizeof( C_COLOR ) * max_colors );
	target = max_colors - fixed_num;
	if( target <= 0 ) return true;

	// ヒストグラムの作成 (RGB各5bit)
	hist = (uint32_t*) malloc( sizeof( uint32_t ) * 32768 );
	if( hist == NULL ) return false;
	memset( hist, 0, sizeof( uint32_t ) * 32768 );
	for( i = 0; i < width * height; ++i ){
		c = in[ i ];
		if( FourceZero && c == FZColor ) continue;
		hist[ ( epal_quantize31( GET_RED( c ) ) << 10 ) | ( epal_quantize31( GET_GREEN( c ) ) << 5 ) | epal_quantize31( GET_BLUE( c ) ) ]++;
	}
	// 固定色は選択対象から除外する
	for( i = 0; i < fixed_num; ++i ){
		c = fixed[ i ];
		hist[ ( epal_quantize31( GET_RED( c ) ) << 10 ) | ( epal_quantize31( GET_GREEN( c ) ) << 5 ) | epal_quantize31( GET_BLUE( c ) ) ] = 0;
	}
	nbin = 0;
	for( i = 0; i < 32768; ++i ){
		if( hist[ i ] ) ++nbin;
	}
	if( nbin == 0 ){
		free( hist );
		return false;
	}
	bins = (C_EPAL_BIN*) malloc( sizeof( C_EPAL_BIN ) * nbin );
	if( bins == NULL ){
		free( hist );
		return false;
	}
	nbin = 0;
	for( i = 0; i < 32768; ++i ){
		if( hist[ i ] == 0 ) continue;
		bins[ nbin ].r = ( i >> 10 ) & 31;
		bins[ nbin ].g = ( i >>  5 ) & 31;
		bins[ nbin ].b =   i        & 31;
		bins[ nbin ].n = (int)hist[ i ];
		++nbin;
	}
	free( hist );
	hist = nullptr;

	if( nbin <= target ){
		// 使われている色が上限以下なら、多い順にそのまま採用する
		C_COLOR_TABLE *tbl = (C_COLOR_TABLE*) malloc( sizeof( C_COLOR_TABLE ) * nbin );
		if( tbl == NULL ){
			free( bins );
			return false;
		}
		for( i = 0; i < nbin; ++i ){
			tbl[ i ].c = GET_RGB( epal_convert31to255[ bins[ i ].r ], epal_convert31to255[ bins[ i ].g ], epal_convert31to255[ bins[ i ].b ] );
			tbl[ i ].n = bins[ i ].n;
		}
		qsort( tbl, nbin, sizeof( C_COLOR_TABLE ), cnvColorTblCompare );
		for( i = 0; i < nbin; ++i ){
			pal[ i ] = tbl[ i ].c;
		}
		*p_pal_count = nbin;
		free( tbl );
		free( bins );
		return true;
	}

	// メディアンカット法で target 個のボックスへ分割する
	boxes[ 0 ].lo = 0;
	boxes[ 0 ].hi = nbin;
	_epal_box_update( &boxes[ 0 ], bins );
	nbox = 1;
	while( nbox < target ){
		// 分割するボックスを選ぶ（画素数が最大で、色の広がりがあるもの）
		int target_box = -1;
		long target_pop = -1;
		int range, channel, split;
		long acc, half;
		C_EPAL_BOX *box;

		for( i = 0; i < nbox; ++i ){
			if( boxes[ i ].hi - boxes[ i ].lo < 2 ) continue;
			range = boxes[ i ].rmax - boxes[ i ].rmin;
			if( boxes[ i ].gmax - boxes[ i ].gmin > range ) range = boxes[ i ].gmax - boxes[ i ].gmin;
			if( boxes[ i ].bmax - boxes[ i ].bmin > range ) range = boxes[ i ].bmax - boxes[ i ].bmin;
			if( range == 0 ) continue;
			if( boxes[ i ].pop > target_pop ){
				target_pop = boxes[ i ].pop;
				target_box = i;
			}
		}
		if( target_box < 0 ) break;		// これ以上分割できない

		// 最も広がりの大きいチャンネルで、画素数の中央値のところで分割する
		box = &boxes[ target_box ];
		channel = 0;
		range = box->rmax - box->rmin;
		if( box->gmax - box->gmin > range ){ channel = 1; range = box->gmax - box->gmin; }
		if( box->bmax - box->bmin > range ){ channel = 2; range = box->bmax - box->bmin; }
		epal_sort_channel = channel;
		qsort( bins + box->lo, box->hi - box->lo, sizeof( C_EPAL_BIN ), _epal_bin_compare );
		half = box->pop / 2;
		acc = 0;
		split = box->hi - 1;
		for( i = box->lo; i < box->hi - 1; ++i ){
			acc += bins[ i ].n;
			if( acc >= half ){
				split = i + 1;
				break;
			}
		}
		boxes[ nbox ].lo = split;
		boxes[ nbox ].hi = box->hi;
		box->hi = split;
		_epal_box_update( box, bins );
		_epal_box_update( &boxes[ nbox ], bins );
		++nbox;
	}

	// 各ボックスの平均色をパレットにする
	for( i = 0; i < nbox; ++i ){
		long rsum = 0, gsum = 0, bsum = 0;
		int r5, g5, b5;
		for( j = boxes[ i ].lo; j < boxes[ i ].hi; ++j ){
			rsum += (long)bins[ j ].r * bins[ j ].n;
			gsum += (long)bins[ j ].g * bins[ j ].n;
			bsum += (long)bins[ j ].b * bins[ j ].n;
		}
		r5 = (int)( ( rsum + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		g5 = (int)( ( gsum + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		b5 = (int)( ( bsum + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		pal[ i ] = GET_RGB( epal_convert31to255[ r5 ], epal_convert31to255[ g5 ], epal_convert31to255[ b5 ] );
	}
	_epal_dedup_palette( bins, nbin, pal, nbox, fixed, fixed_num, true );
	*p_pal_count = nbox;
	free( bins );
	return true;
}

// -------------------------------------------------------------
//	Wu + k-means 法によるパレット選択用 (EpalHGuard の土台選択)
// -------------------------------------------------------------

struct C_WU_BOX {
	int		lo, hi;			// ビン配列の範囲
	long	pop;			// 総画素数
	double	sr, sg, sb;		// Σ n*c
	double	s2;				// Σ n*(r^2+g^2+b^2)
};

static void _wu_box_update( C_WU_BOX *box, C_EPAL_BIN *bins )
{
	int i;
	box->pop = 0;
	box->sr = box->sg = box->sb = box->s2 = 0.0;
	for( i = box->lo; i < box->hi; ++i ){
		double n = (double)bins[ i ].n;
		box->pop += bins[ i ].n;
		box->sr += n * bins[ i ].r;
		box->sg += n * bins[ i ].g;
		box->sb += n * bins[ i ].b;
		box->s2 += n * ( bins[ i ].r * bins[ i ].r + bins[ i ].g * bins[ i ].g + bins[ i ].b * bins[ i ].b );
	}
}

// ボックス内二乗誤差 (SSD)。Wu 法で最大化/最小化の対象になる量
static double _wu_box_ssd( const C_WU_BOX *box )
{
	double v;
	if( box->pop <= 0 ) return 0.0;
	v = box->s2 - ( box->sr * box->sr + box->sg * box->sg + box->sb * box->sb ) / (double)box->pop;
	return v > 0.0 ? v : 0.0;
}

static int _wu_bin_key( const C_EPAL_BIN *bin, int channel )
{
	switch( channel ){
	case 0:		return bin->r;
	case 1:		return bin->g;
	default:	return bin->b;
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		減色用パレット (RGB各5bit) を Wu + k-means 法で求める
//	2.	引数
//		cnvGetPaletteEpalN と同じ
//	3.	返値
//		true: 成功 / false: 失敗
//	4.	備考
//		EpalHGuard 有効時の土台選択。メディアンカット (生カウント) の
//		代わりに、Wu の分散最適ボックス分割 (モーメント法) で初期
//		パレットを作り、k-means (LBG) で生カウント SSE を極小まで
//		磨く。どちらも 555 ヒストグラム上で決定的に動作する。
//		固定色のビンは選択対象から除外される (EpalN と同じ規則で、
//		固定色が中心になることはない)。
// -------------------------------------------------------------
bool cnvGetPaletteEpalWu( C_COLOR *in, int width, int height, C_COLOR *pal, int *p_pal_count,
						  int max_colors, const C_COLOR *fixed, int fixed_num,
						  bool FourceZero, C_COLOR FZColor )
{
	uint32_t *hist = nullptr;
	C_EPAL_BIN *bins = nullptr;
	C_EPAL_BIN *tmpbins = nullptr;
	C_WU_BOX *boxes = nullptr;
	double *ctr = nullptr;		// 中心 (nbox*3)。Wu の平均で初期化し k-means で磨く
	int *owner = nullptr;
	double *acc = nullptr;
	int i, j, nbin, nbox;
	int target;
	C_COLOR c;

	*p_pal_count = 0;
	ZeroMemory( pal, sizeof( C_COLOR ) * max_colors );
	target = max_colors - fixed_num;
	if( target <= 0 ) return true;

	// ヒストグラムの作成 (RGB各5bit、EpalN と同じ)
	hist = (uint32_t*) malloc( sizeof( uint32_t ) * 32768 );
	if( hist == NULL ) return false;
	memset( hist, 0, sizeof( uint32_t ) * 32768 );
	for( i = 0; i < width * height; ++i ){
		c = in[ i ];
		if( FourceZero && c == FZColor ) continue;
		hist[ ( epal_quantize31( GET_RED( c ) ) << 10 ) | ( epal_quantize31( GET_GREEN( c ) ) << 5 ) | epal_quantize31( GET_BLUE( c ) ) ]++;
	}
	// 固定色は選択対象から除外する
	for( i = 0; i < fixed_num; ++i ){
		c = fixed[ i ];
		hist[ ( epal_quantize31( GET_RED( c ) ) << 10 ) | ( epal_quantize31( GET_GREEN( c ) ) << 5 ) | epal_quantize31( GET_BLUE( c ) ) ] = 0;
	}
	nbin = 0;
	for( i = 0; i < 32768; ++i ){
		if( hist[ i ] ) ++nbin;
	}
	if( nbin == 0 ){
		free( hist );
		return false;
	}
	bins = (C_EPAL_BIN*) malloc( sizeof( C_EPAL_BIN ) * nbin );
	if( bins == NULL ){
		free( hist );
		return false;
	}
	nbin = 0;
	for( i = 0; i < 32768; ++i ){
		if( hist[ i ] == 0 ) continue;
		bins[ nbin ].r = ( i >> 10 ) & 31;
		bins[ nbin ].g = ( i >>  5 ) & 31;
		bins[ nbin ].b =   i        & 31;
		bins[ nbin ].n = (int)hist[ i ];
		++nbin;
	}
	free( hist );
	hist = nullptr;

	if( nbin <= target ){
		// 使われている色が上限以下なら、多い順にそのまま採用する (EpalN と同じ)
		C_COLOR_TABLE *tbl = (C_COLOR_TABLE*) malloc( sizeof( C_COLOR_TABLE ) * nbin );
		if( tbl == NULL ){
			free( bins );
			return false;
		}
		for( i = 0; i < nbin; ++i ){
			tbl[ i ].c = GET_RGB( epal_convert31to255[ bins[ i ].r ], epal_convert31to255[ bins[ i ].g ], epal_convert31to255[ bins[ i ].b ] );
			tbl[ i ].n = bins[ i ].n;
		}
		qsort( tbl, nbin, sizeof( C_COLOR_TABLE ), cnvColorTblCompare );
		for( i = 0; i < nbin; ++i ){
			pal[ i ] = tbl[ i ].c;
		}
		*p_pal_count = nbin;
		free( tbl );
		free( bins );
		return true;
	}

	boxes = (C_WU_BOX*) malloc( sizeof( C_WU_BOX ) * target );
	tmpbins = (C_EPAL_BIN*) malloc( sizeof( C_EPAL_BIN ) * nbin );
	if( boxes == NULL || tmpbins == NULL ){
		free( boxes );
		free( tmpbins );
		free( bins );
		return false;
	}

	// ---- Wu 法: SSD 最大のボックスを、分割後の Q が最大になる面で割る ----
	//	Q(box) = ( (Σnr)^2 + (Σng)^2 + (Σnb)^2 ) / Σn で、2分割の Q の
	//	合計を最大化することは、合計 SSD の最小化と等しい。
	boxes[ 0 ].lo = 0;
	boxes[ 0 ].hi = nbin;
	_wu_box_update( &boxes[ 0 ], bins );
	nbox = 1;
	while( nbox < target ){
		int bi2, best_box = -1;
		double best_ssd = 0.0;
		for( bi2 = 0; bi2 < nbox; ++bi2 ){
			double v;
			if( boxes[ bi2 ].hi - boxes[ bi2 ].lo < 2 ) continue;
			v = _wu_box_ssd( &boxes[ bi2 ] );
			if( v > best_ssd ){
				best_ssd = v;
				best_box = bi2;
			}
		}
		if( best_box < 0 ) break;	// これ以上分割できない
		{
			C_WU_BOX *box = &boxes[ best_box ];
			double totalq = ( box->sr * box->sr + box->sg * box->sg + box->sb * box->sb ) / (double)box->pop;
			double bestq = totalq;
			int best_axis = -1, best_cut = -1, axis, coord;
			for( axis = 0; axis < 3; ++axis ){
				double bpop[ 32 ], bsr[ 32 ], bsg[ 32 ], bsb[ 32 ];
				double lpop = 0.0, lsr = 0.0, lsg = 0.0, lsb = 0.0;
				for( coord = 0; coord < 32; ++coord ) bpop[ coord ] = bsr[ coord ] = bsg[ coord ] = bsb[ coord ] = 0.0;
				for( i = box->lo; i < box->hi; ++i ){
					int k = _wu_bin_key( &bins[ i ], axis );
					bpop[ k ] += bins[ i ].n;
					bsr[ k ] += (double)bins[ i ].n * bins[ i ].r;
					bsg[ k ] += (double)bins[ i ].n * bins[ i ].g;
					bsb[ k ] += (double)bins[ i ].n * bins[ i ].b;
				}
				for( coord = 0; coord < 31; ++coord ){
					double rpop, q;
					lpop += bpop[ coord ];
					lsr += bsr[ coord ];
					lsg += bsg[ coord ];
					lsb += bsb[ coord ];
					rpop = (double)box->pop - lpop;
					if( lpop <= 0.0 || rpop <= 0.0 ) continue;
					q = ( lsr * lsr + lsg * lsg + lsb * lsb ) / lpop
					  + ( ( box->sr - lsr ) * ( box->sr - lsr )
						+ ( box->sg - lsg ) * ( box->sg - lsg )
						+ ( box->sb - lsb ) * ( box->sb - lsb ) ) / rpop;
					if( q > bestq ){
						bestq = q;
						best_axis = axis;
						best_cut = coord;
					}
				}
			}
			if( best_axis < 0 ) break;	// これ以上分割できない
			// 安定分配: キーが best_cut 以下のビンを前へ集める
			{
				int m = 0, split;
				for( i = box->lo; i < box->hi; ++i ){
					if( _wu_bin_key( &bins[ i ], best_axis ) <= best_cut ) tmpbins[ m++ ] = bins[ i ];
				}
				split = box->lo + m;
				for( i = box->lo; i < box->hi; ++i ){
					if( _wu_bin_key( &bins[ i ], best_axis ) > best_cut ) tmpbins[ m++ ] = bins[ i ];
				}
				for( i = 0; i < m; ++i ) bins[ box->lo + i ] = tmpbins[ i ];
				boxes[ nbox ].lo = split;
				boxes[ nbox ].hi = box->hi;
				box->hi = split;
				_wu_box_update( box, bins );
				_wu_box_update( &boxes[ nbox ], bins );
				++nbox;
			}
		}
	}

	// ---- k-means (LBG): Wu の中心を種に生カウント SSE を磨く ----
	ctr = (double*) malloc( sizeof( double ) * nbox * 3 );
	if( ctr != NULL ){
		for( i = 0; i < nbox; ++i ){
			ctr[ i * 3 + 0 ] = boxes[ i ].sr / (double)boxes[ i ].pop;
			ctr[ i * 3 + 1 ] = boxes[ i ].sg / (double)boxes[ i ].pop;
			ctr[ i * 3 + 2 ] = boxes[ i ].sb / (double)boxes[ i ].pop;
		}
		owner = (int*) malloc( sizeof( int ) * nbin );
		acc = (double*) malloc( sizeof( double ) * nbox * 4 );
		if( owner != NULL && acc != NULL ){
			int iter;
			for( i = 0; i < nbin; ++i ) owner[ i ] = -1;
			for( iter = 0; iter < 100; ++iter ){
				int changed = 0;
				// 割り当て: 最も近い中心へ (同距離なら小さい添字)
				for( i = 0; i < nbin; ++i ){
					int bj = 0;
					double bd = 1e30;
					for( j = 0; j < nbox; ++j ){
						double dr = bins[ i ].r - ctr[ j * 3 + 0 ];
						double dg = bins[ i ].g - ctr[ j * 3 + 1 ];
						double db = bins[ i ].b - ctr[ j * 3 + 2 ];
						double dd = dr * dr + dg * dg + db * db;
						if( dd < bd ){
							bd = dd;
							bj = j;
						}
					}
					if( owner[ i ] != bj ){
						owner[ i ] = bj;
						changed = 1;
					}
				}
				// 中心の再計算: 担当ビンの画素数重み付き平均 (空の中心は据え置き)
				for( j = 0; j < nbox * 4; ++j ) acc[ j ] = 0.0;
				for( i = 0; i < nbin; ++i ){
					double *a = &acc[ owner[ i ] * 4 ];
					a[ 0 ] += bins[ i ].n;
					a[ 1 ] += (double)bins[ i ].n * bins[ i ].r;
					a[ 2 ] += (double)bins[ i ].n * bins[ i ].g;
					a[ 3 ] += (double)bins[ i ].n * bins[ i ].b;
				}
				// 空クラスタの再シード: 中心が重なって潰れたクラスタを、
				// 最もサービスが悪いビン (画素数 × 距離^2 が最大) へ移す。
				// このまま据え置くと、丸め後に別の中心と重複する色に
				// なることがあるため。
				for( j = 0; j < nbox; ++j ){
					int bi2, best_bin = -1;
					double best_score = 0.0;
					if( acc[ j * 4 ] > 0.0 ) continue;
					for( bi2 = 0; bi2 < nbin; ++bi2 ){
						double dr = bins[ bi2 ].r - ctr[ owner[ bi2 ] * 3 + 0 ];
						double dg = bins[ bi2 ].g - ctr[ owner[ bi2 ] * 3 + 1 ];
						double db = bins[ bi2 ].b - ctr[ owner[ bi2 ] * 3 + 2 ];
						double score = bins[ bi2 ].n * ( dr * dr + dg * dg + db * db );
						int j3;
						bool clash = false;
						if( score <= best_score ) continue;
						for( j3 = 0; j3 < nbox; ++j3 ){
							if( ctr[ j3 * 3 + 0 ] == bins[ bi2 ].r && ctr[ j3 * 3 + 1 ] == bins[ bi2 ].g && ctr[ j3 * 3 + 2 ] == bins[ bi2 ].b ){
								clash = true;
								break;
							}
						}
						if( clash ) continue;
						best_score = score;
						best_bin = bi2;
					}
					if( best_bin >= 0 ){
						ctr[ j * 3 + 0 ] = bins[ best_bin ].r;
						ctr[ j * 3 + 1 ] = bins[ best_bin ].g;
						ctr[ j * 3 + 2 ] = bins[ best_bin ].b;
						changed = 1;
					}
				}
				for( j = 0; j < nbox; ++j ){
					double *a = &acc[ j * 4 ];
					if( a[ 0 ] > 0.0 ){
						ctr[ j * 3 + 0 ] = a[ 1 ] / a[ 0 ];
						ctr[ j * 3 + 1 ] = a[ 2 ] / a[ 0 ];
						ctr[ j * 3 + 2 ] = a[ 3 ] / a[ 0 ];
					}
				}
				if( !changed ) break;	// 割り当てが安定したら収束
			}
		}
	}

	// パレット化
	for( i = 0; i < nbox; ++i ){
		int r5, g5, b5;
		if( ctr != NULL ){
			r5 = (int)( ctr[ i * 3 + 0 ] + 0.5 );
			g5 = (int)( ctr[ i * 3 + 1 ] + 0.5 );
			b5 = (int)( ctr[ i * 3 + 2 ] + 0.5 );
		} else {
			r5 = (int)( ( boxes[ i ].sr + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
			g5 = (int)( ( boxes[ i ].sg + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
			b5 = (int)( ( boxes[ i ].sb + boxes[ i ].pop / 2 ) / boxes[ i ].pop );
		}
		if( r5 < 0 ) r5 = 0;
		if( r5 > 31 ) r5 = 31;
		if( g5 < 0 ) g5 = 0;
		if( g5 > 31 ) g5 = 31;
		if( b5 < 0 ) b5 = 0;
		if( b5 > 31 ) b5 = 31;
		pal[ i ] = GET_RGB( epal_convert31to255[ r5 ], epal_convert31to255[ g5 ], epal_convert31to255[ b5 ] );
	}
	_epal_dedup_palette( bins, nbin, pal, nbox, fixed, fixed_num, false );
	*p_pal_count = nbox;
	free( ctr );
	free( owner );
	free( acc );
	free( boxes );
	free( tmpbins );
	free( bins );
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		8bit色をRGB各5bitへ量子化する
//	2.	引数
//		c	...	(I)	8bit色
//		r5,g5,b5	...	(O)	RGB各5bit値
//	3.	返値
//		なし
// -------------------------------------------------------------
void cnvEpalColorTo5bit( C_COLOR c, int *r5, int *g5, int *b5 )
{
	*r5 = epal_quantize31( GET_RED( c ) );
	*g5 = epal_quantize31( GET_GREEN( c ) );
	*b5 = epal_quantize31( GET_BLUE( c ) );
}

// -------------------------------------------------------------
//	EPAL特徴色改善 (EpalHGuard) 用の小物
// -------------------------------------------------------------

// 分離型ボックスぼかし (端はクランプ)。img を書き換え、tmp を作業に使う
static void _epal_hg_blur( double *img, double *tmp, int w, int h, int radius )
{
	int x, y, k, cnt;
	double s;
	for( y = 0; y < h; ++y ){
		for( x = 0; x < w; ++x ){
			s = 0.0; cnt = 0;
			for( k = x - radius; k <= x + radius; ++k ){
				int cx = k < 0 ? 0 : ( k >= w ? w - 1 : k );
				s += img[ y * w + cx ];
				++cnt;
			}
			tmp[ y * w + x ] = s / cnt;
		}
	}
	for( y = 0; y < h; ++y ){
		for( x = 0; x < w; ++x ){
			s = 0.0; cnt = 0;
			for( k = y - radius; k <= y + radius; ++k ){
				int cy = k < 0 ? 0 : ( k >= h ? h - 1 : k );
				s += tmp[ cy * w + x ];
				++cnt;
			}
			img[ y * w + x ] = s / cnt;
		}
	}
}

static int _epal_hg_cmp_double( const void *a, const void *b )
{
	double d = *(const double *)a - *(const double *)b;
	return ( d > 0 ) - ( d < 0 );
}

// デッドゾーンを引いた残りを p99 で [0,1] に正規化する (配列を書き換える)
static void _epal_hg_normalize( double *a, int n, double deadzone )
{
	int i, m = 0;
	double *pos, p99;
	for( i = 0; i < n; ++i ) a[ i ] = a[ i ] > deadzone ? a[ i ] - deadzone : 0.0;
	pos = (double *)malloc( sizeof( double ) * ( n > 0 ? n : 1 ) );
	if( pos == NULL ) return;
	for( i = 0; i < n; ++i ) if( a[ i ] > 0 ) pos[ m++ ] = a[ i ];
	if( m == 0 ){ free( pos ); return; }
	qsort( pos, (size_t)m, sizeof( double ), _epal_hg_cmp_double );
	p99 = pos[ (int)( 0.99 * ( m - 1 ) ) ];
	free( pos );
	if( p99 <= 0 ) return;
	for( i = 0; i < n; ++i ){
		a[ i ] /= p99;
		if( a[ i ] > 1.0 ) a[ i ] = 1.0;
	}
}

// UVY応答マップ (画素ごと 0..1)。失敗時は NULL
//	色差マグニチュード M=|(U,V)| と輝度 Y のハイパス応答 (半径2ぼかしとの
//	絶対差) を、それぞれデッドゾーン (色差2.0 / 輝度6.0) + p99正規化し、
//	sqrt( nM^2 + nY^2 ) を 1 でクランプしたもの。
static double *_epal_hg_response( C_COLOR *in, int w, int h )
{
	int i, npix = w * h;
	double *Y = NULL, *M = NULL, *tmp = NULL, *blur = NULL;
	Y = (double *)malloc( sizeof( double ) * npix );
	M = (double *)malloc( sizeof( double ) * npix );
	tmp = (double *)malloc( sizeof( double ) * npix );
	blur = (double *)malloc( sizeof( double ) * npix );
	if( Y == NULL || M == NULL || tmp == NULL || blur == NULL ){
		free( Y ); free( M ); free( tmp ); free( blur );
		return NULL;
	}
	for( i = 0; i < npix; ++i ){
		double r = GET_RED( in[ i ] ), g = GET_GREEN( in[ i ] ), b = GET_BLUE( in[ i ] );
		double y = 0.299 * r + 0.587 * g + 0.114 * b;
		double u = 0.492111 * ( b - y );
		double v = 0.877283 * ( r - y );
		Y[ i ] = y;
		M[ i ] = sqrt( u * u + v * v );
	}
	memcpy( blur, M, sizeof( double ) * npix );
	_epal_hg_blur( blur, tmp, w, h, 2 );
	for( i = 0; i < npix; ++i ) M[ i ] = fabs( M[ i ] - blur[ i ] );
	_epal_hg_normalize( M, npix, 2.0 );
	memcpy( blur, Y, sizeof( double ) * npix );
	_epal_hg_blur( blur, tmp, w, h, 2 );
	for( i = 0; i < npix; ++i ) Y[ i ] = fabs( Y[ i ] - blur[ i ] );
	_epal_hg_normalize( Y, npix, 6.0 );
	for( i = 0; i < npix; ++i ){
		double v = sqrt( M[ i ] * M[ i ] + Y[ i ] * Y[ i ] );
		M[ i ] = v > 1.0 ? 1.0 : v;
	}
	free( Y ); free( tmp ); free( blur );
	return M;
}

static double *_epal_hg_sort_key;	// 添字ソート用のキー配列

static int _epal_hg_cmp_desc( const void *a, const void *b )
{
	double d = _epal_hg_sort_key[ *(const int *)b ] - _epal_hg_sort_key[ *(const int *)a ];
	return ( d > 0 ) - ( d < 0 );
}

static int _epal_hg_cmp_asc( const void *a, const void *b )
{
	double d = _epal_hg_sort_key[ *(const int *)a ] - _epal_hg_sort_key[ *(const int *)b ];
	return ( d > 0 ) - ( d < 0 );
}

// -------------------------------------------------------------
//	1.	日本語名
//		EPAL自動選択パレットの特徴色改善
//	2.	引数
//		in	...	(I)	画像メモリ (変換後のサイズ)
//		width, height	...	(I)	画像サイズ
//		pal5	...	(I/O)	自動選択されたパレット (RGB各5bit)
//		pal_count	...	(I)	pal5 の色数
//		fixed5	...	(I)	固定色 (RGB各5bit。置換対象にしない)
//		fixed_num	...	(I)	fixed5 の色数 (0 のとき fixed5 は NULL でよい)
//		FourceZero	...	(I)	除外する色がある true / ない false
//		FZColor	...	(I)	除外する色
//	3.	返値
//		受理した置換数
//	4.	備考
//		メディアンカット (生カウント) の結果を出発点に、小さく目立つ
//		特徴色を取りこぼさないための後処理。
//		提案とスコアは h(c) = sqrt( n(c) ) + b(c) のドメインで行う。
//		n(c) は 555 ヒストグラムの生カウント、b(c) は UVY応答の
//		ビン別合計で、全面応答のビンでは b(c) = n(c) になる画素等価の
//		正規化である。候補は単一ビンではなく、応答の強い画素の
//		色空間クラスタ (種ビンから半径5以内) の応答重み付き重心とする。
//		押し出しは h ドメインの増分が最も小さい順に試し、真の
//		(無重み) SSE が増えない置換だけを受理する (真SSEガード)。
//		固定色は決して押し出さず、受理された特徴色もその後の
//		置換では固定色と同様に保護される。
// -------------------------------------------------------------
int cnvEpalImprovePalette( C_COLOR *in, int width, int height,
						   int (*pal5)[ 3 ], int pal_count,
						   const int (*fixed5)[ 3 ], int fixed_num,
						   bool FourceZero, C_COLOR FZColor )
{
	const double RESPTH = 0.25;		// 強応答とみなす応答の下限
	const int SUPMIN = 8;			// クラスタの最小支持画素数
	const int MAXSWAP = 16;			// 置換回数の上限
	const int MAXSEED = 64;			// 1反復で走査する種ビンの上限
	int npix = width * height;
	int i, e, bi, accepted = 0;
	bool ok = true;
	double *resp = NULL;
	unsigned short *binidx = NULL;
	int *hn = NULL;
	double *hb = NULL;
	int nb = 0;
	int *br = NULL, *bg = NULL, *bb = NULL, *bown = NULL, *order = NULL, *eorder = NULL;
	double *bn = NULL, *bh = NULL, *bd1 = NULL, *bd2 = NULL, *bscore = NULL;
	double *csse = NULL, *cj = NULL, *ekey = NULL;
	int (*ent)[ 3 ] = NULL;
	bool *locked = NULL;		// 採用済みの特徴色 (以降の押し出し対象にしない)
	int nsp = 0;				// 強応答画素の数
	int *spidx = NULL;			// 強応答画素のビン番号
	double *spresp = NULL;		// 強応答画素の応答値
	int rej[ 64 ][ 3 ];
	int nrej = 0;

	if( pal_count <= 0 || npix <= 0 ) return 0;
	resp = _epal_hg_response( in, width, height );
	if( resp == NULL ) return 0;
	binidx = (unsigned short *)malloc( sizeof( unsigned short ) * npix );
	hn = (int *)calloc( 32768, sizeof( int ) );
	hb = (double *)calloc( 32768, sizeof( double ) );
	if( binidx == NULL || hn == NULL || hb == NULL ) ok = false;
	if( ok ){
		// 555 ヒストグラムと応答合計 (選択時と同じ除外規則)
		for( i = 0; i < npix; ++i ){
			int idx;
			if( FourceZero && in[ i ] == FZColor ){ binidx[ i ] = 0xFFFF; continue; }
			idx = ( epal_quantize31( GET_RED( in[ i ] ) ) << 10 )
				| ( epal_quantize31( GET_GREEN( in[ i ] ) ) << 5 )
				| epal_quantize31( GET_BLUE( in[ i ] ) );
			binidx[ i ] = (unsigned short)idx;
			hn[ idx ] += 1;
			hb[ idx ] += resp[ i ];
		}
		for( e = 0; e < fixed_num; ++e ){
			int idx = ( ( fixed5[ e ][ 0 ] & 31 ) << 10 )
					| ( ( fixed5[ e ][ 1 ] & 31 ) << 5 )
					| ( fixed5[ e ][ 2 ] & 31 );
			hn[ idx ] = 0;
			hb[ idx ] = 0.0;
		}
		for( i = 0; i < 32768; ++i ) if( hn[ i ] > 0 ) ++nb;
		if( nb == 0 ) ok = false;
		// 強応答画素のリスト (クラスタ抽出はこの画素だけを走査する)
		spidx = (int *)malloc( sizeof( int ) * npix );
		spresp = (double *)malloc( sizeof( double ) * npix );
		if( spidx == NULL || spresp == NULL ) ok = false;
		else {
			for( i = 0; i < npix; ++i ){
				if( resp[ i ] >= RESPTH && binidx[ i ] != 0xFFFF ){
					spidx[ nsp ] = binidx[ i ];
					spresp[ nsp ] = resp[ i ];
					++nsp;
				}
			}
		}
	}
	if( ok ){
		br = (int *)malloc( sizeof( int ) * nb );
		bg = (int *)malloc( sizeof( int ) * nb );
		bb = (int *)malloc( sizeof( int ) * nb );
		bown = (int *)malloc( sizeof( int ) * nb );
		order = (int *)malloc( sizeof( int ) * nb );
		bn = (double *)malloc( sizeof( double ) * nb );
		bh = (double *)malloc( sizeof( double ) * nb );
		bd1 = (double *)malloc( sizeof( double ) * nb );
		bd2 = (double *)malloc( sizeof( double ) * nb );
		bscore = (double *)malloc( sizeof( double ) * nb );
		eorder = (int *)malloc( sizeof( int ) * pal_count );
		locked = (bool *)calloc( pal_count, sizeof( bool ) );
		csse = (double *)malloc( sizeof( double ) * pal_count );
		cj = (double *)malloc( sizeof( double ) * pal_count );
		ekey = (double *)malloc( sizeof( double ) * pal_count );
		ent = (int (*)[ 3 ])malloc( sizeof( int ) * 3 * ( fixed_num + pal_count ) );
		if( br == NULL || bg == NULL || bb == NULL || bown == NULL || order == NULL
		 || bn == NULL || bh == NULL || bd1 == NULL || bd2 == NULL || bscore == NULL
		 || eorder == NULL || csse == NULL || cj == NULL || ekey == NULL || ent == NULL
		 || locked == NULL ) ok = false;
	}
	if( ok ){
		nb = 0;
		for( i = 0; i < 32768; ++i ) if( hn[ i ] > 0 ){
			br[ nb ] = ( i >> 10 ) & 31;
			bg[ nb ] = ( i >> 5 ) & 31;
			bb[ nb ] = i & 31;
			bn[ nb ] = (double)hn[ i ];
			bh[ nb ] = sqrt( bn[ nb ] ) + hb[ i ];	// h(c) = sqrt( n(c) ) + 1.0 * b(c)
			++nb;
		}
		while( accepted < MAXSWAP ){
			int nent = 0, nseed = 0, f;
			bool swapped = false;
			double total_j = 0.0;
			for( e = 0; e < fixed_num; ++e ){
				ent[ nent ][ 0 ] = fixed5[ e ][ 0 ] & 31;
				ent[ nent ][ 1 ] = fixed5[ e ][ 1 ] & 31;
				ent[ nent ][ 2 ] = fixed5[ e ][ 2 ] & 31;
				++nent;
			}
			for( e = 0; e < pal_count; ++e ){
				ent[ nent ][ 0 ] = pal5[ e ][ 0 ] & 31;
				ent[ nent ][ 1 ] = pal5[ e ][ 1 ] & 31;
				ent[ nent ][ 2 ] = pal5[ e ][ 2 ] & 31;
				++nent;
			}
			// 最近傍割り当て (d1 / d2 / 所有者) と種ビンのスコア
			for( bi = 0; bi < nb; ++bi ){
				double d1 = 1e30, d2 = 1e30;
				int ow = -1;
				for( e = 0; e < nent; ++e ){
					int dr = br[ bi ] - ent[ e ][ 0 ], dg = bg[ bi ] - ent[ e ][ 1 ], db = bb[ bi ] - ent[ e ][ 2 ];
					double dd = (double)( dr * dr + dg * dg + db * db );
					if( dd < d1 ){ d2 = d1; d1 = dd; ow = e; }
					else if( dd < d2 ){ d2 = dd; }
				}
				bd1[ bi ] = d1;
				bd2[ bi ] = d2;
				bown[ bi ] = ow;
				bscore[ bi ] = bh[ bi ] * d1;
				total_j += bh[ bi ] * d1;
				order[ bi ] = bi;
			}
			_epal_hg_sort_key = bscore;
			qsort( order, (size_t)nb, sizeof( int ), _epal_hg_cmp_desc );
			nrej = 0;
			for( f = 0; f < nb && nseed < MAXSEED; ++f ){
				int sbi = order[ f ];
				int sr, sg, sb, sup = 0, ci2;
				double wsum = 0.0, rsum = 0.0, gsum = 0.0, bsum = 0.0;
				int cand[ 3 ];
				bool dup = false;
				double add_sse = 0.0, add_j = 0.0, tol_j;
				if( bscore[ sbi ] <= 0.0 ) break;
				++nseed;
				sr = br[ sbi ]; sg = bg[ sbi ]; sb = bb[ sbi ];
				// 候補クラスタ: 強応答画素のうち色空間で種から半径5以内
				for( i = 0; i < nsp; ++i ){
					int idx = spidx[ i ], dr, dg, db;
					dr = ( ( idx >> 10 ) & 31 ) - sr;
					dg = ( ( idx >> 5 ) & 31 ) - sg;
					db = ( idx & 31 ) - sb;
					if( dr * dr + dg * dg + db * db > 25 ) continue;
					++sup;
					wsum += spresp[ i ];
					rsum += spresp[ i ] * ( ( idx >> 10 ) & 31 );
					gsum += spresp[ i ] * ( ( idx >> 5 ) & 31 );
					bsum += spresp[ i ] * ( idx & 31 );
				}
				if( sup < SUPMIN || wsum <= 0.0 ) continue;
				cand[ 0 ] = (int)( rsum / wsum + 0.5 );
				cand[ 1 ] = (int)( gsum / wsum + 0.5 );
				cand[ 2 ] = (int)( bsum / wsum + 0.5 );
				for( e = 0; e < nent; ++e ){
					if( ent[ e ][ 0 ] == cand[ 0 ] && ent[ e ][ 1 ] == cand[ 1 ] && ent[ e ][ 2 ] == cand[ 2 ] ){ dup = true; break; }
				}
				if( dup ) continue;
				for( i = 0; i < nrej; ++i ){
					if( rej[ i ][ 0 ] == cand[ 0 ] && rej[ i ][ 1 ] == cand[ 1 ] && rej[ i ][ 2 ] == cand[ 2 ] ){ dup = true; break; }
				}
				if( dup ) continue;
				// 候補追加の寄与と、押し出される色が所有するビンの取り戻し分
				for( e = 0; e < pal_count; ++e ){ csse[ e ] = 0.0; cj[ e ] = 0.0; }
				for( bi = 0; bi < nb; ++bi ){
					int dr = br[ bi ] - cand[ 0 ], dg = bg[ bi ] - cand[ 1 ], db = bb[ bi ] - cand[ 2 ];
					double dc = (double)( dr * dr + dg * dg + db * db );
					double keep = bd1[ bi ] < dc ? bd1[ bi ] : dc;
					add_sse += bn[ bi ] * ( keep - bd1[ bi ] );
					add_j += bh[ bi ] * ( keep - bd1[ bi ] );
					if( bown[ bi ] >= fixed_num ){
						double alt = bd2[ bi ] < dc ? bd2[ bi ] : dc;
						csse[ bown[ bi ] - fixed_num ] += bn[ bi ] * ( alt - keep );
						cj[ bown[ bi ] - fixed_num ] += bh[ bi ] * ( alt - keep );
					}
				}
				// 押し出し順: ΔJ (hドメイン) の小さい順
				// (採用済みの特徴色は固定色と同様に押し出し対象から外す)
				{
					int ne = 0;
					for( e = 0; e < pal_count; ++e ){
						if( locked[ e ] ) continue;
						eorder[ ne ] = e;
						ekey[ ne ] = add_j + cj[ e ];
						++ne;
					}
					_epal_hg_sort_key = ekey;
					qsort( eorder, (size_t)ne, sizeof( int ), _epal_hg_cmp_asc );
					tol_j = 1e-6 * ( total_j > 0.0 ? total_j : 1.0 );
					for( ci2 = 0; ci2 < ne; ++ci2 ){
					double dj, ds;
					e = eorder[ ci2 ];
					dj = add_j + cj[ e ];
					if( dj >= -tol_j ) break;
					ds = add_sse + csse[ e ];
					if( ds <= 0.0 ){	// 真SSEガード: 悪化する置換は受理しない
						pal5[ e ][ 0 ] = cand[ 0 ];
						pal5[ e ][ 1 ] = cand[ 1 ];
						pal5[ e ][ 2 ] = cand[ 2 ];
						locked[ e ] = true;	// 採用した特徴色は固定する
						++accepted;
						swapped = true;
						break;
					}
					}
				}
				if( swapped ) break;
				if( nrej < 64 ){
					rej[ nrej ][ 0 ] = cand[ 0 ];
					rej[ nrej ][ 1 ] = cand[ 1 ];
					rej[ nrej ][ 2 ] = cand[ 2 ];
					++nrej;
				}
			}
			if( !swapped ) break;
		}
	}
	free( resp ); free( binidx ); free( hn ); free( hb );
	free( br ); free( bg ); free( bb ); free( bown ); free( order ); free( eorder );
	free( bn ); free( bh ); free( bd1 ); free( bd2 ); free( bscore );
	free( csse ); free( cj ); free( ekey ); free( ent );
	free( spidx ); free( spresp );
	free( locked );
	return accepted;
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
	pnum = ( screen_mode == MD_SC6 || screen_mode == MD_SC6_256L || screen_mode == MD_SC6P || screen_mode == MD_SC6P_256L ) ? 4 : 16;
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
//		パレットの大小比較 (EPAL 5bit版)
//	2.	引数
//		Pal1	...	(I)	パレット１のアドレス
//		Pal2	...	(I)	パレット２のアドレス
//	3.	返値
//		true	...	Pal2 の方が小さい
//		false	...	Pal2 の方が大きい
//	4.	備考
//		cnvCompare の 5bit 版 (マスク 16,8,4,2,1)
// -------------------------------------------------------------
static bool cnvCompare5( C_PALETTE* Pal1, C_PALETTE* Pal2 )
{
	int	i;
	int	mask;

	mask = 16;
	for( i = 0; i < 5; ++i ) {
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
	C_PALETTE			epal_palette[16] = { 0 }, tBakEpal;	// EPAL用 5bitパレット (ソートに追随させる)
	uint8_t		PalEn[16] = { 0 }, tBakEn;
	C_COLOR	pal[16] = { 0 }, tBakPal;
	int			cnt, z, x, pnum;
	bool		epal = cnvIsEpalMode( Mode->screen_mode );

	//	パレットをコピー
	cnt  = 0;
	pnum = ( Mode->screen_mode == MD_SC6 || Mode->screen_mode == MD_SC6_256L || Mode->screen_mode == MD_SC6P || Mode->screen_mode == MD_SC6P_256L ) ? 4 : 16;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			color_palette[ cnt ]   = Mode->color_palette[ z ];
			epal_palette[ cnt ]  = Mode->color_palette_epal[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			color_palette[ cnt ]   = Mode->color_palette[ z ];
			epal_palette[ cnt ]  = Mode->color_palette_epal[ z ];
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
				if( epal ? cnvCompare5( &epal_palette[ x ], &epal_palette[ x + 1 ] ) : cnvCompare( &color_palette[ x ], &color_palette[ x + 1 ] ) ) {
					tBakCol        = color_palette[ x ];
					color_palette[ x ]       = color_palette[ x + 1 ];
					color_palette[ x + 1 ]   = tBakCol;
					tBakEn         = PalEn[ x ];
					PalEn[ x ]     = PalEn[ x + 1 ];
					PalEn[ x + 1 ] = tBakEn;
					tBakPal        = pal[ x ];
					pal[ x ]       = pal[ x + 1 ];
					pal[ x + 1 ]   = tBakPal;
					tBakEpal       = epal_palette[ x ];
					epal_palette[ x ]     = epal_palette[ x + 1 ];
					epal_palette[ x + 1 ] = tBakEpal;
				}
			}
		}
		break;
	case SM_DECAUTO:
	case SM_DEC:
		for( z = cnt - 1; z >= 0; --z ) {
			for( x = 0; x < z; ++x ) {
				if( !( epal ? cnvCompare5( &epal_palette[ x ], &epal_palette[ x + 1 ] ) : cnvCompare( &color_palette[ x ], &color_palette[ x + 1 ] ) ) ) {
					tBakCol        = color_palette[ x ];
					color_palette[ x ]       = color_palette[ x + 1 ];
					color_palette[ x + 1 ]   = tBakCol;
					tBakEn         = PalEn[ x ];
					PalEn[ x ]     = PalEn[ x + 1 ];
					PalEn[ x + 1 ] = tBakEn;
					tBakPal        = pal[ x ];
					pal[ x ]       = pal[ x + 1 ];
					pal[ x + 1 ]   = tBakPal;
					tBakEpal       = epal_palette[ x ];
					epal_palette[ x ]     = epal_palette[ x + 1 ];
					epal_palette[ x + 1 ] = tBakEpal;
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
			if( epal ) Mode->color_palette_epal[ z ] = epal_palette[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Mode->color_palette[ z ]   = color_palette[ cnt ];
			if( epal ) Mode->color_palette_epal[ z ] = epal_palette[ cnt ];
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
