// -------------------------------------------------------------
// BMP to MSX
// 変換処理
//										(C)2000/5/6 HRA!
// -------------------------------------------------------------

#include "Convert.h"
#include "math.h"
#include "fileuty.h"

// -------------------------------------------------------------
// 型
// -------------------------------------------------------------
typedef struct {
	COLORREF			c;		// 色
	int					n;		// その数
} COLORTBL;

// -------------------------------------------------------------
// 設定
// -------------------------------------------------------------
const int dith[][8][8]={	// ディザリングパターン
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
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
	}
};

// MSX階調（ 0～7 ) → Win階調 ( 0～255 ) 変換テーブル
const int cc[]={
	0, 36, 73, 109, 146, 182, 219, 255
};

// MSX階調（ 0～3 ) SCREEN8青 → Win階調 ( 0～255 ) 変換テーブル
const int cb[]={
/*	0, 85, 170, 255	*/
	0, 73, 146, 255
};

//	SCREEN2 のカラーテーブルアドレス
#define	SC2COLOR	0x2000

//	SCREEN2/3 変換用作業域
static unsigned char vram[ 0x4000 ];

// -------------------------------------------------------------
// プロトタイプ
// -------------------------------------------------------------
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y );
static inline void SetPix( COLORREF *out,int width,int height,int x,int y,COLORREF c );
static inline int Abs( int a );
static inline int AdjustNum( int n,int min,int max );
static inline void PutDither( int *r,int *g,int *b,int mode,int ErrAdd,int x,int y,COLORREF c );
static inline int Interval( int r1,int g1,int b1,int r2,int g2,int b2 );

static int cnvCreateHistgram( COLORREF *in,int size,COLORTBL **tbl,COLORREF *pal,int pp,
							  bool FourceZero,COLORREF FZColor );
static bool cnvCompare( PAL* Pal1, PAL* Pal2 );

static bool cnvRecolor8( COLORREF *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
						TAILPAT *tail,int tailcnt );
static bool cnvRecolor5( COLORREF *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
						TAILPAT *tail,int tailcnt );

static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, COLORREF *pal );
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog );

static void DrawScreen2( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen3( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen5( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen6( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen7( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen8( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreenC( const unsigned char *bmp, HDC hDC, const SETTING *Mode );

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
bool cnvCopy( COLORREF *in ,int inwidth ,int inheight ,
			  COLORREF *out,int outwidth,int outheight, PROGRESS prog, int seido )
{
	int	x,y,w;
	int	hh;
	COLORREF mask;

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
bool cnvCopyInter( COLORREF *in ,int inwidth ,int inheight ,
			  COLORREF *out,int outwidth,int outheight,PROGRESS prog,int seido )
{
	int	x,y;
	int	ww, hh;
	COLORREF c1,c2;
	COLORREF mask;

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
			out[ x ]=RGB(	( GetRValue( c1 ) + GetRValue( c2 ) ) / 2,
							( GetGValue( c1 ) + GetGValue( c2 ) ) / 2,
							( GetBValue( c1 ) + GetBValue( c2 ) ) / 2 ) & mask;
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
bool cnvNormResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy;
	COLORREF	c;
	COLORREF	mask;

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
			c   = GetPix( in, inwidth, inheight, xx, yy );
			SetPix( out, outwidth, outheight, x, y, RGB(GetRValue( c ), GetGValue( c ), GetBValue( c )) & mask );
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
bool cnvAntiResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy, ix, iy, ixm1, ixm2, iym1, iym2, ixw, iyw;
	float		ax, ay, prx, pgx, pbx, pry, pgy, pby, am;
	COLORREF	c;
	COLORREF	mask;
	bool		bWZoom;	//	幅拡大
	bool		bHZoom;	//	高さ拡大

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
					c   = GetPix( in, inwidth, inheight, xx, yy );
					prx = GetRValue( c ) * (1-ax) * (1-ay);
					pgx = GetGValue( c ) * (1-ax) * (1-ay);
					pbx = GetBValue( c ) * (1-ax) * (1-ay);
					c	= GetPix( in, inwidth, inheight, xx+1, yy );
					prx += GetRValue( c ) * ax * (1-ay);
					pgx += GetGValue( c ) * ax * (1-ay);
					pbx += GetBValue( c ) * ax * (1-ay);
					c   = GetPix( in, inwidth, inheight, xx, yy+1 );
					prx += GetRValue( c ) * (1-ax) * ay;
					pgx += GetGValue( c ) * (1-ax) * ay;
					pbx += GetBValue( c ) * (1-ax) * ay;
					c	= GetPix( in, inwidth, inheight, xx+1, yy+1 );
					prx += GetRValue( c ) * ax * ay;
					pgx += GetGValue( c ) * ax * ay;
					pbx += GetBValue( c ) * ax * ay;
					ax = 1.0f;
				} else {
					//	水平縮小の場合
					if( ixw == 0 ){
						c	= GetPix( in, inwidth, inheight, ixm1, yy );
						prx = GetRValue( c ) * (1-ay);
						pgx = GetGValue( c ) * (1-ay);
						pbx = GetBValue( c ) * (1-ay);
						c	= GetPix( in, inwidth, inheight, ixm1, yy+1 );
						prx += GetRValue( c ) * ay;
						pgx += GetGValue( c ) * ay;
						pbx += GetBValue( c ) * ay;
						ax  = am = 1.0f;
					}else{
						ax = 0;					// 重みの合計値（Ｘ軸方向）
						prx = pgx = pbx = 0;	// ピクセル値合計値（Ｘ軸方向）
						for( ix = ixm1; ix <= ixm2; ++ix ){
							am = float( ix - ixm1 ) / ixw;
							if( am < 0 ) am = -am;
							am = 1.0f - am;
							c = GetPix( in, inwidth, inheight, ix, yy );
							prx += GetRValue( c ) * am * (1-ay);
							pgx += GetGValue( c ) * am * (1-ay);
							pbx += GetBValue( c ) * am * (1-ay);
							c = GetPix( in, inwidth, inheight, ix, yy+1 );
							prx += GetRValue( c ) * am * ay;
							pgx += GetGValue( c ) * am * ay;
							pbx += GetBValue( c ) * am * ay;
							ax  += am;
						}	// ix
					}
				}
				SetPix( out, outwidth, outheight, x, y,
						RGB( int( prx/ax ), int( pgx/ax ), int( pbx/ax ) ) & mask );
			} else {
				//	垂直縮小の場合
				for( iy = iym1; iy <= iym2; ++iy ){
					if( bWZoom ) {
						//	水平拡大の場合
						xx  = x * inwidth / wwidth;
						ax  = (float(x) * inwidth / wwidth) - float(xx);	//	位置 xx+1 への一致度
						c   = GetPix( in, inwidth, inheight, xx, iy );
						prx = GetRValue( c ) * (1-ax);
						pgx = GetGValue( c ) * (1-ax);
						pbx = GetBValue( c ) * (1-ax);
						c = GetPix( in, inwidth, inheight, xx+1, iy );
						prx += GetRValue( c ) * ax;
						pgx += GetGValue( c ) * ax;
						pbx += GetBValue( c ) * ax;
						ax = 1.0f;
					} else {
						//	水平縮小の場合
						if( ixw == 0 ){
							c = GetPix( in, inwidth, inheight, ixm1, iy );
							prx = GetRValue( c );
							pgx = GetGValue( c );
							pbx = GetBValue( c );
							ax  = am = 1.0f;
						}else{
							ax = 0;					// 重みの合計値（Ｘ軸方向）
							prx = pgx = pbx = 0;	// ピクセル値合計値（Ｘ軸方向）
							for( ix = ixm1; ix <= ixm2; ++ix ){
								am = float( ix - ixm1 ) / ixw;
								if( am < 0 ) am = -am;
								am = 1.0f - am;
								c = GetPix( in, inwidth, inheight, ix, iy );
								prx += GetRValue( c ) * am;
								pgx += GetGValue( c ) * am;
								pbx += GetBValue( c ) * am;
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
				SetPix( out, outwidth, outheight, x, y,
						RGB( int( pry/ay ), int( pgy/ay ), int( pby/ay ) ) & mask );
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
static inline void PutDither( int *r, int *g, int *b, int mode, int ErrAdd, int x, int y, COLORREF c )
{
	if( mode >= EALGO_DITH1 ){
		switch( ErrAdd ){
		case EADD_ROTATE:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][       x & 7  ][ y & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][       y & 7  ][ x & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ 7 - ( x & 7 )][ y & 7 ] * 2 - 16;
			break;
		case EADD_NONE:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ] * 2 - 16;
			break;
		case EADD_X:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ ( x + 0 ) & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ ( x + 1 ) & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ ( x + 2 ) & 7 ] * 2 - 16;
			break;
		case EADD_Y:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][ ( y + 0 ) & 7 ][ x & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][ ( y + 1 ) & 7 ][ x & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ ( y + 2 ) & 7 ][ x & 7 ] * 2 - 16;
			break;
		}
	}else{
		*r = GetRValue( c );
		*g = GetGValue( c );
		*b = GetBValue( c );
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
bool cnvRecolor( COLORREF *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
					TAILPAT *tail,int tailcnt )
{
	bool	bRet;

	if( CnvMode->Mode == MD_SC8 || CnvMode->Mode == MD_SC8_256L ) {
		//	256色ビットマップへ変換する
		return cnvRecolor8( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );
	} else {
		//	16色ビットマップへ変換する
		bRet = cnvRecolor5( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );

		//	特殊な画面（SC2/3/4) にあわせて変換する		
		switch( CnvMode->Mode ) {
		case MD_SC2:
			bRet = bRet & cnvSC5toSC2( out, prog, pal );
			break;
		case MD_SC3:
			bRet = bRet & cnvSC5toSC3( out, prog );
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
static bool cnvRecolor8( COLORREF *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
					TAILPAT *tail,int tailcnt )
{
	int				x,y,ptr;
	int				cr,cg,cb;		// 元画素のＲＧＢ
	int				er,eg,eb;		// 誤差（正数へ丸める）
	int				n;
	COLORREF		c,cc,*pin,FZC;
	signed short	*errbuf[2], *errbuf0, *errbuf1;
	int k			= CnvMode->Gosa ? int(CnvMode->Gosaval*1024) : 0;
	int ee			= CnvMode->err;
	COLORREF mask;
	bool			ret = false;

	// 誤差蓄積用ﾊﾞｯﾌｧの作成
	memset( errbuf, 0, sizeof( errbuf ) );
	for( x = 0; x < 2; ++x ) {
		errbuf[ x ] = (signed short*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 4 * sizeof( signed short ) );
		if( errbuf[ x ]==NULL ) goto l_exit;
	}

	// 精度制限用マスク作成
	mask = ~(( 1 << CnvMode->Seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	FZC = CnvMode->FourceZero ? CnvMode->FZColor & mask : -1;

	// 全ピクセルを舐める
	ptr = 0;
	pin = in;

	if( !CnvMode->Tile ){
		// 非タイルモード
		if( k ) {
			//	誤差拡散有り
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 誤差を計算に入れる
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// 最も近い色を見つける
					er = ( cr + 18 ) * 7 / 255;
					eg = ( cg + 18 ) * 7 / 255;
					eb = ( cb + 36 ) * 3 / 255;
					er = AdjustNum( er, 0, 7 );
					eg = AdjustNum( eg, 0, 7 );
					eb = AdjustNum( eb, 0, 3 );
					n = ( er << 2 ) | ( eg << 5 ) | eb;
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[ ( n >> 2) & 0x07 ], ::cc[ ( n >> 5)  & 0x07 ], ::cb[ n & 0x03 ] );

					// 誤差を周囲のピクセルへ拡散させる
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// 微細な誤差は消滅させる
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// 右に拡散
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// 下に拡散
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
					out[ptr]=n;
					// 次の出力先
					++ptr;
				}	// x
			}	// y
		} else {
			//	誤差拡散無し
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 最も近い色を見つける
					er = ( cr + 18 ) * 7 / 255;
					eg = ( cg + 18 ) * 7 / 255;
					eb = ( cb + 36 ) * 3 / 255;
					er = AdjustNum( er, 0, 7 );
					eg = AdjustNum( eg, 0, 7 );
					eb = AdjustNum( eb, 0, 3 );
					n = ( er << 2 ) | ( eg << 5 ) | eb;
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[ ( n >> 2) & 0x07 ], ::cc[ ( n >> 5)  & 0x07 ], ::cb[ n & 0x03 ] );

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
					out[ptr]=n;
					// 次の出力先
					++ptr;
				}	// x
			}	// y
		}
	}else{
		// タイルモード
		if( k ) {
			//	誤差拡散有り
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 誤差を計算に入れる
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// 最も近い色を見つける
					er = ( cr +  9 ) * 14 / 255;
					eg = ( cg +  9 ) * 14 / 255;
					eb = ( cb + 18 ) *  6 / 255;
					er = AdjustNum( er, 0, 14 );
					eg = AdjustNum( eg, 0, 14 );
					eb = AdjustNum( eb, 0,  6 );
					n = (( er >> 1 ) << 2 ) | (( eg >> 1 ) << 5 ) | ( eb >> 1 );
					if( ( x ^ y ) & 1 ){
						n += (( er & 1 ) << 2 ) | (( eg & 1 ) << 5 ) | ( eb & 1 );
					}
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[( n >> 2 ) & 0x07 ], ::cc[ ( n >> 5 ) & 0x07 ], ::cb[ n & 0x03 ] );

					// 誤差を周囲のピクセルへ拡散させる
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// 微細な誤差は消滅させる
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// 右に拡散
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// 下に拡散
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
					out[ptr]=n;
					// 次の出力先
					++ptr;
				}	// x
			}	// y
		} else {
			//	誤差拡散無し
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 最も近い色を見つける
					er = ( cr +  9 ) * 14 / 255;
					eg = ( cg +  9 ) * 14 / 255;
					eb = ( cb + 18 ) *  6 / 255;
					er = AdjustNum( er, 0, 14 );
					eg = AdjustNum( eg, 0, 14 );
					eb = AdjustNum( eb, 0,  6 );
					n = (( er >> 1 ) << 2 ) | (( eg >> 1 ) << 5 ) | ( eb >> 1 );
					if( ( x ^ y ) & 1 ){
						n += (( er & 1 ) << 2 ) | (( eg & 1 ) << 5 ) | ( eb & 1 );
					}
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[( n >> 2 ) & 0x07 ], ::cc[ ( n >> 5 ) & 0x07 ], ::cb[ n & 0x03 ] );

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
					out[ptr]=n;
					// 次の出力先
					++ptr;
				}	// x
			}	// y
		}	//	誤差拡散
	}	//	タイル or 非タイル

	ret = true;
l_exit:
	if( errbuf[0] != NULL ) LocalFree( errbuf[0] );
	if( errbuf[1] != NULL ) LocalFree( errbuf[1] );
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
static bool cnvRecolor5( COLORREF *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
					TAILPAT *tail,int tailcnt )
{
	int				x,y,z,d,ptr;
	int				cr,cg,cb;		// 元画素のＲＧＢ
	int				er,eg,eb;		// 誤差（正数へ丸める）
	int				r,n,nr;
	COLORREF		c,cc,*pin,FZC;
	signed short	*errbuf[2], *errbuf0, *errbuf1;
	int palnum;
	int k			= CnvMode->Gosa ? int(CnvMode->Gosaval*1024) : 0;
	int ee			= CnvMode->err;
	COLORREF mask;
	bool			ret = false;

	// 誤差蓄積用ﾊﾞｯﾌｧの作成
	memset( errbuf, 0, sizeof( errbuf ) );
	for( x = 0; x < 2; ++x ) {
		errbuf[ x ] = (signed short*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 4 * sizeof( signed short ) );
		if( errbuf[ x ]==NULL ) goto l_exit;
	}

	// 精度制限用マスク作成
	mask = ~(( 1 << CnvMode->Seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	// パレット数
	switch( CnvMode->Mode ){
	case MD_SC6:
	case MD_SC6_256L:
		palnum = 4;		break;
	default:
		palnum = 16;	break;
	}
	FZC = CnvMode->FourceZero ? CnvMode->FZColor & mask : -1;

	// 全ピクセルを舐める
	ptr = 0;
	pin = in;

	if( !CnvMode->Tile ){
		// 非タイルモード
		if( k ) {
			//	誤差拡散有り
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 誤差を計算に入れる
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// 最も近い色を見つける
					nr = 0x7FFFFFFFL;

					if( CnvMode->NonZero ) z = 1; else z = 0;
					for( ; z < palnum; ++z ){
						if( CnvMode->PalEn[ z ] == PALEN_NONE ) continue;
						// 近いか？
						c = pal[ z ];
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = z;
							cc = c;
							if( r == 0 ) break;	// 完全一致なら抜ける
						}
					}	// z

					// 誤差を周囲のピクセルへ拡散させる
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// 微細な誤差は消滅させる
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// 右に拡散
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// 下に拡散
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
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
		} else {
			//	誤差拡散無し
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 最も近い色を見つける
					nr = 0x7FFFFFFFL;

					if( CnvMode->NonZero ) z = 1; else z = 0;
					for( ; z < palnum; ++z ){
						if( CnvMode->PalEn[ z ] == PALEN_NONE ) continue;
						// 近いか？
						c = pal[ z ];
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = z;
							cc = c;
							if( r == 0 ) break;	// 完全一致なら抜ける
						}
					}	// z

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
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
		}	//	誤差拡散
	} else {
		// タイルモード
		if( k ) {
			//	誤差拡散有り
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 誤差を計算に入れる
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// 最も近い色を見つける
					nr = 0x7FFFFFFFL;

					for( z = 0; z < tailcnt; ++z ){	// タイルパターン
						// 近いか？
						c = tail[ z ].c;
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = tail[z].p[ ( x ^ y ) & 1 ];
							cc = c;
							if( r == 0 ) break;	// 完全一致なら抜ける
						}
					}	// z

					// 誤差を周囲のピクセルへ拡散させる
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// 微細な誤差は消滅させる
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// 右に拡散
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// 下に拡散
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
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
		} else {
			//	誤差拡散無し
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// ディザ処理
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// 最も近い色を見つける
					nr = 0x7FFFFFFFL;

					for( z = 0; z < tailcnt; ++z ){	// タイルパターン
						// 近いか？
						c = tail[ z ].c;
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = tail[z].p[ ( x ^ y ) & 1 ];
							cc = c;
							if( r == 0 ) break;	// 完全一致なら抜ける
						}
					}	// z

					// 結果を出力する
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// 強制ゼロ化
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
		}	//	誤差拡散
	}	//	タイル or 非タイル
	ret = true;
l_exit:
	if( errbuf[0] != NULL ) LocalFree( errbuf[0] );
	if( errbuf[1] != NULL ) LocalFree( errbuf[1] );
	return ret;
}

// -------------------------------------------------------------
//	1.	日本語名
//		SCREEN5 イメージを SCREEN2/4 イメージへ変換する
//	2.	引数
//		out		...	VRAMイメージ
//		prog	...	経過表示関数
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		インターレースモードは使えない
// -------------------------------------------------------------
static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, COLORREF *pal )
{
	int	x, y, i, j, adr, dadr;
	int	cc, c, p;
	int	cnt, idx1, idx2, len, l;
	int	ccnt[ 8 ];
	int	ccol[ 8 ];
	COLORREF	c1, c2;

	memset( vram, 0, sizeof( vram ) );

	adr  = 0;
	for( y = 0; y < 192; ++y ) {
		for( x = 0; x < 32; ++x ) {
			//	８ドットペアの中で最も多く使われている２色を選ぶ
			cnt  = 0;
			idx1 = 0;							//	最も多い色のインデックス
			for( j = 0; j < 8; ++j ) {			//	8ドットなめる
				cc = out[ adr + j/2 ];
				if( j & 1 ) {
					cc = cc & 0x0F;
				} else {
					cc = (cc >> 4) & 0x0F;
				}
				for( i = 0; i < cnt; ++i ) {	//	同じ色を検索
					if( cc == ccol[ i ] ) {
						++ccnt[ i ];			//		見つけた色の数を増加
						if( ccnt[ i ] > ccnt[ idx1 ] ) {	//	最も多い
							idx1 = i;
						}
						break;
					}
				}
				if( i == cnt ) {				//	新しい色
					ccol[ cnt ] = cc;			//		見つけた色
					ccnt[ cnt ] = 1;			//		見つけた色の数
					++cnt;
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
				c1= pal[ ccol[ idx1 ] ];
				for( i = 0; i < cnt; ++i ) {
					if( i == idx1 ) continue;
					cc = ccol[i];
					l = Interval( GetRValue( pal[cc] ), GetGValue( pal[cc] ), GetBValue( pal[cc] ), GetRValue( c1 ), GetGValue( c1 ), GetBValue( c1 ));
					if( l >= len ) {
						len = l;
						idx2 = i;
					}
				}

				c = ( ccol[ idx1 ] << 4 ) | ccol[ idx2 ];
				p = 0;
				c2= pal[ ccol[ idx2 ] ];
				for( j = 0; j < 8; ++j ) {
					cc = out[ adr + j/2 ];
					if( j & 1 ) {
						cc = cc & 0x0F;
					} else {
						cc = (cc >> 4) & 0x0F;
					}
					//	選ばれた２色との色距離を比較
					p = p << 1;
					if( Interval( GetRValue( pal[cc] ), GetGValue( pal[cc] ), GetBValue( pal[cc] ), GetRValue( c1 ), GetGValue( c1 ), GetBValue( c1 )) 
					  < Interval( GetRValue( pal[cc] ), GetGValue( pal[cc] ), GetBValue( pal[cc] ), GetRValue( c2 ), GetGValue( c2 ), GetBValue( c2 )) ) {
						p = p | 1;
					}
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

	memcpy( out, vram, sizeof( vram ) );
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
bool cnvNtcolor( COLORREF *in ,int width ,int height ,unsigned char *out ,
				 SETTING *CnvMode ,PROGRESS prog )
{
	int				x, y, z, xx;
	int				z255p31[32];
	COLORREF		c;
	int				r[ 5 ], g[ 5 ], b[ 5 ];
	int				vy, min, tyy, txx;
	int				rr, gg, bb, er, eg, eb;
	int				jj, kk, yy[ 4 ], ii[ 4 ], s;
	int				mask;
	unsigned char	*pout = out;
	short			*ptmp,*tmp = NULL;
	int				*errbuf[ 2 ], *errbuf0, *errbuf1;

	int		k		= CnvMode->Gosa ? int( CnvMode->Gosaval * 1024 ) : 0;
	int		algo	= CnvMode->AlgoMode;
	bool	rc		= CnvMode->JKrc;
	int		ealgo	= CnvMode->ErrAlgo;
	bool	sc10	= ( CnvMode->Mode == MD_SC10 || CnvMode->Mode == MD_SC10_256L );
	int		zero	= CnvMode->FourceZero ? (sc10 ? 0x10 : 0x08) : 0;
	bool	ret		= false;

	//	作業変数
	for( y = 0; y < 32; ++y ) {
		z255p31[ y ] = y * 255 / 31;
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
				c = GetPix( in, width, height, x + z, y );
				r[ z ] = GetRValue( c ) + errbuf0[ txx + 0 ];
				g[ z ] = GetGValue( c ) + errbuf0[ txx + 1 ];
				b[ z ] = GetBValue( c ) + errbuf0[ txx + 2 ];
				vy   = AdjustNum( b[ z ] / 2 + r[ z ] / 4 + g[ z ] / 8, 0, 255 );
				jj += r[ z ] - vy;
				kk += g[ z ] - vy;
				txx += 3;
			}
			// J,K の値を平均して４画素ペアの J,K 値を確定する
			jj = AdjustNum( jj / 4, -256, 255 );
			kk = AdjustNum( kk / 4, -256, 255 );

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
					rr = AdjustNum( r[z], 0, 255 );
					gg = AdjustNum( g[z], 0, 255 );
					bb = AdjustNum( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = Interval( rr, gg, bb,
							AdjustNum( vy + jj, 0, 255 ),  
							AdjustNum( vy + kk, 0, 255 ), 
							AdjustNum( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
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
					rr = AdjustNum( r[z], 0, 255 );
					gg = AdjustNum( g[z], 0, 255 );
					bb = AdjustNum( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = Interval( rr, gg, bb,
							AdjustNum( vy + jj, 0, 255 ),  
							AdjustNum( vy + kk, 0, 255 ), 
							AdjustNum( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
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
				yy[ z ] = AdjustNum( tyy, 0, 255 );
				if( k ){	// 誤差拡散
					// 算出した 仮Y,J,K から R,G,B 値（ MSXで表示される色 ）を計算
					rr = z255p31[ AdjustNum( jj + yy[ z ], 0, 255 ) >> 3 ];
					gg = z255p31[ AdjustNum( kk + yy[ z ], 0, 255 ) >> 3 ];
					bb = z255p31[ AdjustNum( 5 * yy[ z ] / 4 - jj / 2 - kk / 4, 0, 255 ) >> 3 ];
					// 誤差を周囲のピクセルへ拡散させる
					er = AdjustNum( ( r[ z ] - rr ) * k / 1024 , -32768, 32767 );
					eg = AdjustNum( ( g[ z ] - gg ) * k / 1024 , -32768, 32767 );
					eb = AdjustNum( ( b[ z ] - bb ) * k / 1024 , -32768, 32767 );

					// 微細な誤差は消滅させる
					if( Abs( er ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 3 ] += er;
						errbuf1[ txx + 0 ] = er;
					} else {
						errbuf1[ txx + 0 ] = 0;
					}
					if( Abs( eg ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 4 ] += eg;
						errbuf1[ txx + 1 ] = eg;
					} else {
						errbuf1[ txx + 1 ] = 0;
					}
					if( Abs( eb ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 5 ] += eb;
						errbuf1[ txx + 2 ] = eb;
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
			jj = AdjustNum( jj * 31 / 255, -32, 31 );	// 小数点以下切り捨て
			kk = AdjustNum( kk * 31 / 255, -32, 31 );	// 小数点以下切り捨て
			
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
				*out=( kk     & 7) | ( AdjustNum( yy[0], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((kk>>3) & 7) | ( AdjustNum( yy[1], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=( jj     & 7) | ( AdjustNum( yy[2], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((jj>>3) & 7) | ( AdjustNum( yy[3], 0, 255 ) & mask );
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
			z  = AdjustNum( vy, 0, 255 ) & mask;				//	実際に採用するＹ値
			jj = ( vy - z255p31[ z >> 3 ] ) * k / 1024;			//	誤差
			if( jj >= (signed)CnvMode->err ) {
				// 誤差の拡散
				if( x != width - 1 ) {
					*(ptmp + 1) = (short)AdjustNum( (int)*(ptmp +     1) + jj, -32768, 32767 );
				}
				*(ptmp + width) = (short)AdjustNum( (int)*(ptmp + width) + jj, -32768, 32767 );
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
int cnvColorTblCompare( const void *tbl1,const void *tbl2 )
{
	return( (long)(((COLORTBL*)tbl2)->n )-(long)(((COLORTBL*)tbl1)->n ) );
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
bool cnvGetPalette( COLORREF *in,int width,int height,COLORREF *pal,int mode,int cnt,int pp,
				    bool FourceZero,COLORREF FZColor )
{
	COLORREF	c;
	COLORTBL	*tbl, tmp;
	int			n, i, j;
	int			r, g, b, v, w, y;

	// ２～１６色しか処理できない
	if( cnt < 2 || cnt > 16 || cnt <= pp ) return false;
	
	// ヒストグラムの作成
	n=cnvCreateHistgram( in, width * height, &tbl, pal, pp, FourceZero, FZColor );
	if( tbl == NULL ) return false;
	ZeroMemory( pal + pp, sizeof( COLORREF ) * ( cnt - pp ) );	

	// 色をソートする
	qsort( tbl, n, sizeof( COLORTBL ), cnvColorTblCompare );

	if( mode==0 ){
		// 目的の色数になるまで似た色を削除し続ける
		i=0;
		while( n > cnt ){
			// 着目色を取得
			c = tbl[ i ].c;
			r = GetRValue( c );
			g = GetGValue( c );
			b = GetBValue( c );
			y = -1;							// 選出した色のインデックス
			w = 0x7FFFFFFFL;				// 選出した色の距離
			for( j = i + 1; j < n; ++j ){	// 着目色に最も近くて使用数が少ない色を検索する
				if( j < pp ) continue;
				c = tbl[ j ].c;
				v = Interval( r, g, b, GetRValue( c ), GetGValue( c ), GetBValue( c ) );
				if( w < v ) continue;
				y = j;
				w = v;
			}
			if( y > 0 ){			// 検索して見つかった色を着目色に吸収する
				tbl[ i ].n += tbl[ y ].n;
				if( tbl[ i ].n < 0 )
					tbl[ i ].n = 0x7FFFFFFFL;
				if( y < n - 1 ) CopyMemory( &tbl[ y ], &tbl[ y + 1 ], sizeof( COLORTBL ) * ( n - 1 - y ) );
				--n;
				for( j = i; j > 0; --j ) {
					if( tbl[ j ].n <= tbl[ j-1 ].n ) break;
					tmp			= tbl[ j ];
					tbl[ j ]	= tbl[ j-1 ];
					tbl[ j-1 ]	= tmp;
				}
			}
			++i;					// 次の色
			if( i >= n ) i = 0;		// 見つかるまで循環して検索する
		}
	} else {
		//	多分布選色
		for( i = 0; i < pp; ++i ){
			// 着目色（選択色を使用の色）を取得
			c = tbl[ i ].c;
			r = GetRValue( c );
			g = GetGValue( c );
			b = GetBValue( c );
			y = -1;
			for( j = pp; j < n; ++j ){	// 着目色と一致する色を検索する（１つしかない）
				c = tbl[ j ].c;
				if( (r == GetRValue( c )) && (g == GetGValue( c )) && (b == GetBValue( c )) ) {
					y = j;
					break;
				}
			}
			if( y > 0 ){			// 検索して見つかった色を削除する
				if( y < n - 1 ) CopyMemory( &tbl[ y ], &tbl[ y + 1 ], sizeof( COLORTBL ) * ( n - 1 - y ) );
				--n;
			}
		}
	}

	// テーブル内のパレット先頭 cnt 個を採用する
	for( i = 0; i < n && i < cnt; ++i ){
		pal[ i ] = tbl[ i ].c;
	}

	// メモリを解放する
	LocalFree( tbl );
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
static int cnvCreateHistgram( COLORREF *in,int size,COLORTBL **tbl,COLORREF *pal,int pp,
							  bool FourceZero,COLORREF FZColor )
{
	COLORTBL	hash[ 512 ];
	int			cnv[ 256 ];
	int			i,n,cnt,t,r,g,b;
	COLORREF	c;
	// NULL クリア
	ZeroMemory( hash,sizeof( hash ) );
	*tbl=NULL;
	cnt=0;		// 色の使用数

	//	変換テーブルを生成する
	for( i = 0; i < 256; ++i ) {
		cnv[i] = ( i + 18 ) * 7 / 255;
	}

	// 画素の数を数える
	for( i = 0; i < size; ++i ){
		c = in[ i ];
		if( c == FZColor && FourceZero ) continue;
		r = cnv[ GetRValue( c ) ];
		g = cnv[ GetGValue( c ) ];
		b = cnv[ GetBValue( c ) ];
		n = r * 64 + g * 8 + b;		//	ハッシュ関数
		hash[ n ].c = RGB( cc[r], cc[g], cc[b] );
		++hash[ n ].n;
		if( hash[ n ].n == 1 ) ++cnt;
	}	// for

	// カラーテーブルを確保する
	*tbl=(COLORTBL*) LocalAlloc( LMEM_FIXED, sizeof( COLORTBL ) * ( cnt + pp ) );
	if( *tbl == NULL ) return 0;

	// ハッシュ表から tbl へ変換
	t=0;
	for( i = 0; i < pp ; ++i ){
		(*tbl)[ t ].c = pal[ i ];
		(*tbl)[ t ].n = 0x7FFFFFFF;
		++t;
	}
	for( i = 0; i < 512; ++i ){
		if( hash[i].n == 0 ) continue;
		(*tbl)[ t ] = hash[ i ];
		++t;
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
int cnvCreateTail4( PAL *pal,uchar *palen,bool zeroen,TAILPAT *tail, int mode )
{
	int i, j, k, l;
	int r,g,b;
	int rr,gg,bb;
	int n=0,m;
	bool ok;
	int	pnum;
	// すべての色の組み合わせを処理する
	l = zeroen ? 0 : 1;
	pnum = ( mode == MD_SC6 || mode == MD_SC6_256L ) ? 4 : 16;
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
					if( Interval( r, g, b, rr, gg, bb ) <
						Interval( pal[ tail[ k ].p[ 0 ] ].red,pal[ tail[ k ].p[ 0 ] ].green,pal[ tail[ k ].p[ 0 ] ].blue,
								  pal[ tail[ k ].p[ 1 ] ].red,pal[ tail[ k ].p[ 1 ] ].green,pal[ tail[ k ].p[ 1 ] ].blue ) ){
						m = k;
					}else{
						m = -1;
						break;
					}
				}
			}
			if( m != -1 ){
				tail[ m ].c = RGB( ( cc[ r ] + cc[ rr ] ) / 2, ( cc[ g ] + cc[ gg ] ) / 2, ( cc[ b ] + cc[ bb ] ) / 2 );
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
static bool cnvCompare( PAL* Pal1, PAL* Pal2 )
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
void cnvSortPalette( SETTING* Mode, COLORREF* Pal )
{
	PAL			Col[16]  , tBakCol;	// SC5/SC7 におけるＭＳＸ側パレット指定
	uchar		PalEn[16], tBakEn;
	COLORREF	pal[16]  , tBakPal;
	int			cnt, z, x, pnum;

	//	パレットをコピー
	cnt  = 0;
	pnum = ( Mode->Mode == MD_SC6 || Mode->Mode == MD_SC6_256L ) ? 4 : 16;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			Col[ cnt ]   = Mode->Col[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Col[ cnt ]   = Mode->Col[ z ];
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
				if( cnvCompare( &Col[ x ], &Col[ x + 1 ] ) ) {
					tBakCol        = Col[ x ];
					Col[ x ]       = Col[ x + 1 ];
					Col[ x + 1 ]   = tBakCol;
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
				if( !cnvCompare( &Col[ x ], &Col[ x + 1 ] ) ) {
					tBakCol        = Col[ x ];
					Col[ x ]       = Col[ x + 1 ];
					Col[ x + 1 ]   = tBakCol;
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
			Mode->Col[ z ]   = Col[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Mode->Col[ z ]   = Col[ cnt ];
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
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y )
{
	// 範囲外修正
	x = AdjustNum( x,0,width-1  );
	y = AdjustNum( y,0,height-1 );
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
static inline void SetPix( COLORREF *out,int width,int height,int x,int y,COLORREF c )
{
	// 範囲外修正
	x = AdjustNum( x,0,width-1  );
	y = AdjustNum( y,0,height-1 );
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
static inline int Abs( int a )
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
static inline int Interval( int r1, int g1, int b1, int r2, int g2, int b2 )
{
	int r, g, b;
	r = r2 - r1;
	g = g2 - g1;
	b = b2 - b1;
	return( r*r + g*g + b*b );
}

// -------------------------------------------------------------
//	プレビュー生成

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
int _GetHeight( const SETTING *Mode )
{
	if( Mode->b192 ) {
		return 192;
	} else {
		return 212;
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
void DrawScreen( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	RECT	rc = { 0, 0, 512, 424 };

	FillRect( hDC, &rc, ( HBRUSH )GetStockObject( BLACK_BRUSH ) );

	switch( Mode->Mode ){
	case MD_SC2:
		DrawScreen2( bmp, hDC, Mode);	return;
	case MD_SC3:
		DrawScreen3( bmp, hDC, Mode);	return;
	case MD_SC5:
	case MD_SC5_256L:
		DrawScreen5( bmp, hDC, Mode );	return;
	case MD_SC6:
	case MD_SC6_256L:
		DrawScreen6( bmp, hDC, Mode );	return;
	case MD_SC7:
	case MD_SC7_256L:
		DrawScreen7( bmp, hDC, Mode );	return;
	case MD_SC8:
	case MD_SC8_256L:
		DrawScreen8( bmp, hDC, Mode );	return;
	case MD_SC10:
	case MD_SC12:
	case MD_SC10_256L:
	case MD_SC12_256L:
		DrawScreenC( bmp, hDC, Mode );	return;
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
static void DrawScreen2( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r[2],g[2],b[2];
	int x, y, adr, i, c, bit;

	for( y=0; y<384; y+=2 ){
		for( x=0; x<512; x+=16 ){
			adr=y/2;
			adr=( adr >> 3 )*256 + ( x / 16 )*8 + ( adr & 7 );
			// 下位４ビット（バックグランド）
			r[0] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] & 0x0F ].red   ];
			g[0] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] & 0x0F ].green ];
			b[0] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] & 0x0F ].blue  ];
			// 上位４ビット（フォアグランド）
			r[1] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] >> 4 ].red   ];
			g[1] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] >> 4 ].green ];
			b[1] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] >> 4 ].blue  ];

			//	８ドットペアの描画
			c = bmp[ adr ];
			for( i = 0; i < 8; ++i ) {
				bit = ( c & 0x80 ) >> 7;
				SetPixel( hDC, x + i*2 + 0 , y + 20, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				SetPixel( hDC, x + i*2 + 1 , y + 20, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				SetPixel( hDC, x + i*2 + 0 , y + 21, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				SetPixel( hDC, x + i*2 + 1 , y + 21, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
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
static void DrawScreen3( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,i;

	for( y=0; y<384; ++y ){
		for( x=0; x<512; x+=16 ){
			adr = y/8;
			adr = ( adr >> 3 )*256 + (x/16)*8 + (adr&7);
			// 上位４ビット（左側）
			r=cc[ Mode->Col[ bmp[ adr ] >> 4 ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] >> 4 ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] >> 4 ].blue  ];
			for( i=0; i<8; ++i ) {
				SetPixel( hDC, x+i  , y+20, RGB( r,g,b ) );
			}
			// 下位４ビット（右側）
			r=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].blue  ];
			for( i=0; i<8; ++i ) {
				SetPixel( hDC, x+i+8, y+20, RGB( r,g,b ) );
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
static void DrawScreen5( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y=0; y<h; ++y ){
		for( x=0; x<512; x+=4 ){
			if( Mode->Inter ) adr=y; else adr=y/2;
			adr=adr*128+x/4;
			// 上位４ビット（左側）
			r=cc[ Mode->Col[ bmp[ adr ] >> 4 ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] >> 4 ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] >> 4 ].blue  ];
			SetPixel( hDC,x+0,y+t,RGB( r,g,b ) );
			SetPixel( hDC,x+1,y+t,RGB( r,g,b ) );
			// 下位４ビット（右側）
			r=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].blue  ];
			SetPixel( hDC,x+2,y+t,RGB( r,g,b ) );
			SetPixel( hDC,x+3,y+t,RGB( r,g,b ) );
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
static void DrawScreen6( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y=0; y<h; ++y ){
		for( x=0; x<512; x+=4 ){
			if( Mode->Inter ) adr=y; else adr=y/2;
			adr=adr*128+x/4;
			// 上位２ビット（左側）
			r=cc[ Mode->Col[ (bmp[ adr ] >> 6) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ] >> 6) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ] >> 6) & 0x03 ].blue  ];
			SetPixel( hDC, x+0, y+t, RGB( r, g, b ) );
			// 上位２ビット（中左側）
			r=cc[ Mode->Col[ (bmp[ adr ] >> 4) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ] >> 4) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ] >> 4) & 0x03 ].blue  ];
			SetPixel( hDC, x+1, y+t, RGB( r, g, b ) );
			// 下位２ビット（中右側）
			r=cc[ Mode->Col[ (bmp[ adr ] >> 2) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ] >> 2) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ] >> 2) & 0x03 ].blue  ];
			SetPixel( hDC, x+2, y+t, RGB( r, g, b ) );
			// 下位２ビット（右側）
			r=cc[ Mode->Col[ (bmp[ adr ]     ) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ]     ) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ]     ) & 0x03 ].blue  ];
			SetPixel( hDC, x+3, y+t, RGB( r, g, b ) );
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
static void DrawScreen7( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y=0; y<h; ++y ){
		for( x=0; x<512; x+=2 ){
			if( Mode->Inter ) adr=y; else adr=y/2;
			adr=adr*256+x/2;
			// 上位４ビット（左側）
			r=cc[ Mode->Col[ bmp[ adr ] >> 4 ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] >> 4 ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] >> 4 ].blue  ];
			SetPixel( hDC, x+0, y+t, RGB( r, g, b ) );
			// 下位４ビット（右側）
			r=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].blue  ];
			SetPixel( hDC, x+1, y+t, RGB( r, g, b ) );
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
static void DrawScreen8( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 2 ){
			if( Mode->Inter ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r=cc[(bmp[ adr ] & 0x1C) >> 2 ];
			g=cc[(bmp[ adr ] & 0xE0) >> 5 ];
			b=cb[ bmp[ adr ] & 0x03       ];
			SetPixel( hDC, x + 0, y+t, RGB( r, g, b ) );
			SetPixel( hDC, x + 1, y+t, RGB( r, g, b ) );
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
static void DrawScreenC( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b,j,k,yy[4];
	int x,y,z,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 8 ){
			if( Mode->Inter ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			//	輝度成分の取得
			for( z=0; z<4; ++z ){
				yy[ z ] = ( bmp[ adr + z ] & 0xF8 ) >> 3;
			}
			//	色成分の取得
			k = ( bmp[ adr + 0 ] & 0x07 ) | (( bmp[ adr + 1 ] & 0x07 ) << 3 );
			if( k > 31 ) k = k - 64;	//	5bit の２の歩数
			j = ( bmp[ adr + 2 ] & 0x07 ) | (( bmp[ adr + 3 ] & 0x07 ) << 3 );
			if( j > 31 ) j = j - 64;	//	5bit の２の歩数
			// 描画
			for( z = 0; z < 4; ++z ){
				r = AdjustNum( yy[ z ] + j							, 0, 31 )* 255 / 31;
				g = AdjustNum( yy[ z ] + k							, 0, 31 )* 255 / 31;
				b = AdjustNum( 5 * yy[ z ] / 4 - j / 2 - k / 4		, 0, 31 )* 255 / 31;
				if( r == 246 && g == 246 && b == 255 ) {
					r = r;
				}
				SetPixel( hDC, x + z * 2 + 0, y+t, RGB( r, g, b) );
				SetPixel( hDC, x + z * 2 + 1, y+t, RGB( r, g, b) );
			}
		}
	}
}

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
static inline int AdjustNum( int n, int min, int max )
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

	DrawScreen( bmp, hDC, Mode );
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
