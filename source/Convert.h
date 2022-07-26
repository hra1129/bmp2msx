// -------------------------------------------------------------
// BMP to MSX
//											(C)2000/5/6 HRA!
// -------------------------------------------------------------

#ifndef _CONVERTER_H_
#define	_CONVERTER_H_

#include <windows.h>

typedef bool ( *PROGRESS )( int );		// 経過表示用コールバック関数型
typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct {
	int		w;			// 幅
	int		h;			// 高さ
} SSIZE;

typedef struct {
	int		green;		// 緑
	int		red;		// 赤
	int		blue;		// 青
} PAL;

typedef struct {
	COLORREF	c;		// 色
	int			p[2];	// タイルに使われるパレットの番号
} TAILPAT;

#define TAILMAX	1575	// タイル生成最大数

typedef struct {
	uint		Mode;		// 変換先モード MD_SC??
	uint		err;		// この値より小さい誤差は無視する
	bool		Gosa;		// 誤差拡散 する:true / しない:false
	bool		Inter;		// インターレース する:true / しない:false
	uint		SelCol;		// 色選択モード 0:分散選択 / 1:多分布選択
	float		Gosaval;	// 誤差拡散係数 ( 0.000～0.500 )
	bool		Pal;		// 固定パレット する:true / しない:false
	bool		Resize;		// サイズ調節
	PAL			Col[16];	// SC5/SC7 におけるＭＳＸ側パレット指定
	uint		PltMode;	// パレット出力モード
	bool		AutoName;	// 自動ファイル名決定
	uint		AlgoMode;	// 自然画生成アルゴリズム番号
	bool		JKrc;		// 色差情報の再計算
	uint		ErrAlgo;	// ディザパターン
	uint		PreView;	// 1 ファイルのみ出力,2 プレビューのみ出力,3 ファイルとプレビュー出力
	uchar		PalEn[16];	// 2 パレット使用許可(固定) / 1 パレット使用未許可 / 0 パレット使用許可(自動)
	uint		Seido;		// 色再現精度
	uint		ErrAdd;		// ディザ加算方法
	bool		NonZero;	// ０番の色を使わない
	uint		FourceZero;	// 強制ゼロ化
	COLORREF	FZColor;	// 強制ゼロ化する色
	uint		FZX;		// 強制ゼロ化Ｘ座標
	uint		FZY;		// 強制ゼロ化Ｙ座標
	bool		Tile;		// 網がけタイルを使う
	int			SortMode;	// ﾊﾟﾚｯﾄのｿｰﾄ方法
	bool		bDefault;	// 常に再変換をﾃﾞﾌｫ化
	int			CnvMode;	// 変換設定
	bool		b192;		// 192ラインモード
	int			Resample;	// ｻｲｽﾞ調整ﾘｻﾝﾌﾟﾙ
	int			SizeMode;	// ｻｲｽﾞ調整ﾓｰﾄﾞ
	COLORREF	FCColor;	// 背景色
} SETTING;

// 画面モード
enum {
	MD_SC2,				// SCREEN 2
	MD_SC3,				// SCREEN 3
	MD_SC5,				// SCREEN 5
	MD_SC6,				// SCREEN 6
	MD_SC7,				// SCREEN 7
	MD_SC8,				// SCREEN 8
	MD_SC10,			// SCREEN 10
	MD_SC12,			// SCREEN 12
	MD_SC5_256L,		// SCREEN 5 (256 lines)
	MD_SC6_256L,		// SCREEN 6 (256 lines)
	MD_SC7_256L,		// SCREEN 7 (256 lines)
	MD_SC8_256L,		// SCREEN 8 (256 lines)
	MD_SC10_256L,		// SCREEN 10 (256 lines)
	MD_SC12_256L,		// SCREEN 12 (256 lines)
	MD_MAX,
};

// パレット決定アルゴリズム
enum {
	SELCOL_BUNSAN=0,	// 分散選択
	SELCOL_TABUNPU,		// 多分布選択
	SELCOL_MAX,
};

// パレット出力モード
enum {
	PLT_NONE=0,			// 出力しない
	PLT_BSAVE,			// 画像に内蔵
	PLT_PL,				// *.PL? 形式
	PLT_CSV,			// CSV形式
	PLT_COUNT,			// 出力モードの種類数
	PLT_MAX,
};

// 自然画生成アルゴリズム
enum {
	ALGO_NONE,			// 無し
	ALGO_RECALC,		// 逆算による誤差訂正（輝度再現優先）
	ALGO_POWERFUL,		// パワフル活用法のレイトレーシング法に使われていた式
	ALGO_NTSC2,			// NTSC方式改良版
	ALGO_RECALC2,		// 逆算による誤差訂正（色合い再現優先）
	ALGO_MAX,
};

// ディザ
enum {
	EALGO_NONE,			// 無し
	EALGO_DITH1,		// ディザ１
	EALGO_DITH2,		// ディザ２
	EALGO_DITH3,		// ディザ３
	EALGO_DITH4,		// ディザ４
	EALGO_DITH5,		// ディザ５
	EALGO_DITH6,		// ディザ６
	EALGO_DITH7,		// ディザ７
	EALGO_DITH8,		// ディザ８
	EALGO_MAX,
};

// ディザ加算方法
enum {
	EADD_ROTATE,		// 回転
	EADD_NONE,			// 同位相
	EADD_X,				// Ｘずらし
	EADD_Y,				// Ｙずらし
	EADD_MAX,
};

// 色再現精度
enum {
	SEIDO_24,			// 24bit(0bit捨て)
	SEIDO_21,			// 21bit(1bit捨て)
	SEIDO_18,			// 18bit(2bit捨て)
	SEIDO_15,			// 15bit(3bit捨て)
	SEIDO_12,			// 12bit(4bit捨て)
	SEIDO_MAX,
};

// パレット使用許可
enum {
	PALEN_AUTO,			// パレット使用許可（自動）
	PALEN_NONE,			// パレット使用無許可
	PALEN_USE,			// パレット使用許可（固定）
	PALEN_MAX,
};

// 強制ゼロ化
enum {
	FZ_NONE,			// 使わない
	FZ_COLOR,			// 指定色
	FZ_MATRIX,			// 指定座標の色
	FZ_MAX,
};

//	ソート
enum {
	SM_IGNORE,			//	ソートしない
	SM_INCAUTO,			//	自動決定のみ昇順
	SM_DECAUTO,			//	自動決定のみ降順
	SM_INC,				//	昇順
	SM_DEC,				//	降順
};

//	変換設定
enum {
	CM_DEFAULT,			//	デフォルト設定
	CM_LAST,			//	最終変換の設定
	CM_IMAGE,			//	表示画像の変換設定
};

//	サイズ調節
enum {
	RS_NORMAL,			//	単純調節
	RS_ANTIALIAS,		//	アンチエイリアス付き調節
};

//	アスペクト比
enum {
	RM_FORCE,			//	強制的に出力サイズ
	RM_MSX,				//	ＭＳＸ実機表示で縦横比が崩れないように調整
};

//	RGB値からテーブルインデックスに変換
int convert_rgb_to_palette( const int *p_convert_table, int n, int v );

//	縮小処理
bool cnvNormResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido );
bool cnvAntiResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido );

//	複製処理
bool cnvCopy( COLORREF *in ,int inwidth ,int inheight ,
			  COLORREF *out,int outwidth,int outheight,PROGRESS prog,int seido );
bool cnvCopyInter(	COLORREF *in ,int inwidth ,int inheight ,
					COLORREF *out,int outwidth,int outheight,PROGRESS prog,int seido );

//	減色処理
bool cnvRecolor( COLORREF *in,int width,int height,unsigned char *out,SETTING *CnvMode,PROGRESS prog,
				 COLORREF *pal,TAILPAT *tail,int tailcnt );

//	減色処理（自然画コンバータ）
bool cnvNtcolor( COLORREF *in,int width,int height,unsigned char *out,SETTING *CnvMode,PROGRESS prog );

//	色比較
int cnvColorTblCompare( const void *tbl1,const void *tbl2 );

//	色取得
bool cnvGetPalette( COLORREF *in,int width,int height,COLORREF *pal,int mode,int cnt,int pp,
				    bool FourceZero,COLORREF FZColor );
bool cnvGetPaletteS8( COLORREF *pal );

//	タイルパターン取得(SC5/SC7専用)
int cnvCreateTail4( PAL *pal,uchar *palen,bool zeroen,TAILPAT *tail, int mode );

//	プレビュー作成（逆変換）
void DrawScreen( const unsigned char *bmp,HDC hDC,const SETTING *Mode );

//	パレットのソート
void cnvSortPalette( SETTING* Mode, COLORREF* Pal );

//	プレビューのＢＭＰ保存
bool cnvSaveBmpFile( const char *szInFileName, LPBYTE bmp, int width, int height, SETTING *Mode );


// MSX階調 → Win階調 ( 0～255 ) 変換テーブル
extern const int init_convert7to255_r[ 8 ];
extern const int init_convert7to255_g[ 8 ];
extern const int init_convert7to255_b[ 8 ];

// MSX階調 SCREEN8 → Win階調 ( 0～255 ) 変換テーブル
extern const int init_convert7to255_s8r[ 8 ];
extern const int init_convert7to255_s8g[ 8 ];
extern const int init_convert3to255_s8b[ 4 ];

// MSX階調 SCREEN12 → Win階調 ( 0～255 ) 変換テーブル
extern const int init_convert31to255_s12r[ 32 ];
extern const int init_convert31to255_s12g[ 32 ];
extern const int init_convert31to255_s12b[ 32 ];

// MSX階調 → Win階調 ( 0～255 ) 変換テーブル
extern int convert7to255_r[ 8 ];
extern int convert7to255_g[ 8 ];
extern int convert7to255_b[ 8 ];

// MSX階調 SCREEN8 → Win階調 ( 0～255 ) 変換テーブル
extern int convert7to255_s8r[ 8 ];
extern int convert7to255_s8g[ 8 ];
extern int convert3to255_s8b[ 4 ];

// MSX階調 SCREEN12 → Win階調 ( 0～255 ) 変換テーブル
extern int convert31to255_s12r[ 32 ];
extern int convert31to255_s12g[ 32 ];
extern int convert31to255_s12b[ 32 ];

#endif	// ndef _CONVERTER_H_
