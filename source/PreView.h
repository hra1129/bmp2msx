// -------------------------------------------------------------
//	TITLE	:	BMP to MSX プレビュー管理関連
//	INPUT	:	HRA!
// -------------------------------------------------------------

#ifndef	_PREVIEW_H_
#define	_PREVIEW_H_

#include <windows.h>

// =============================================================
//	型

// オリジナル画像
typedef struct {
	char*	image;					// 画像ファイルイメージ
	char	sFileName[ MAX_PATH ];	// 変換元のファイル名（クリップボードの場合は "" をさす）
	int		n;						// 参照カウンタ
} BMPDATA;

// プレビュー
typedef struct {
	HBITMAP		hBmp;					// プレビュー画像（ディスティネーション）
	HBITMAP		hBmpSrc;				// プレビュー画像（ソース）
	int			nSrc;					// オリジナル画像のインデックス
	int			nTime;					// 登録連番（古さの決定基準）
	SETTING		tMode;					// プレビュー画像生成時の設定
	unsigned char*	bmp;				// 変換結果のＶＲＡＭイメージ
} PVDATA;

// =============================================================
//	公開関数

void pvInitialize( void );
int pvBlankIndex( void );
void pvDelete( int Index );
void pvAllDelete( void );
int pvNextPreview( int Index );
int pvPrevPreview( int Index );
bool pvAddPreview( int Index );
void pvDrawBmp( HDC hDC, int Index, COLORREF *in, int width, int height );
int pvGetNum( int Index );
int pvGetCnt( void );

int bdBlankIndex( void );
void bdDelete( int Index );
int bdAddFile( const char *cszFileName );
int bdAddImage( char *bmp );

// =============================================================
//	公開変数

extern BMPDATA	tBmpview[ PV_MAX ];		// ＢＭＰファイル情報
extern PVDATA	tPreview[ PV_MAX ];		// プレビュー情報
extern int		pvIndex;				// 現在表示中のプレビュー
extern int		nTime;					// プレビュー登録時刻（古さの決定基準）

#endif	//	_PREVIEW_H_
