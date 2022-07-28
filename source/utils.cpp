// -------------------------------------------------------------
// BMP to MSX
// 各種ユーティリティ
//												(C)2000 HRA!
// -------------------------------------------------------------

#include "utils.h"
#include "fileuty.h"

extern HINSTANCE hIns;

// MSX1の色に似せたパレット値  { G, R, B }
static const PAL msx1_palette[]={
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 5, 3, 3 },
	{ 6, 4, 4 },
	{ 2, 3, 7 },
	{ 3, 4, 7 },
	{ 3, 5, 2 },
	{ 5, 3, 6 },
	{ 3, 6, 2 },
	{ 4, 6, 3 },
	{ 6, 5, 3 },
	{ 6, 6, 4 },
	{ 4, 2, 1 },
	{ 3, 5, 5 },
	{ 5, 5, 5 },
	{ 7, 7, 7 },
};

// MSX2デフォルトパレット（MSX2電源投入時パレット値）
static const PAL msx2_palette[] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 6, 1, 1 },
	{ 7, 3, 3 },
	{ 1, 1, 7 },
	{ 3, 2, 7 },
	{ 1, 5, 1 },
	{ 6, 2, 7 },
	{ 1, 7, 1 },
	{ 3, 7, 3 },
	{ 6, 6, 1 },
	{ 6, 6, 4 },
	{ 4, 1, 1 },
	{ 2, 6, 5 },
	{ 5, 5, 5 },
	{ 7, 7, 7 },
};

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

// =====================================================
// ファイル名関連

// -------------------------------------------------------------
//	1.	日本語名
//		ファイル名を入力する
//	2.	引数
//		szFileName	...	ファイル名格納アドレス
//		size		...	上記メモリのサイズ
//		szTitle		...	タイトル
//		szFilter	...	フィルタ
//		szExt		...	デフォルト拡張子
//	3.	返値
//			OK = true
//			キャンセル = false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetName( HWND hWnd,char *szFileName,int size,const char *szTitle,const char *szFilter, const char* szExt )
{
	OPENFILENAME	ofn;
	// ファイル名を入力
	szFileName[0]='\0';
	ZeroMemory( &ofn,sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= szFilter;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= size;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// カレント
	ofn.lpstrTitle		= szTitle;
	ofn.lpstrDefExt		= szExt;
	return( GetSaveFileName( &ofn )!=FALSE );
}

// =====================================================
// ＢＭＰファイル関連

// -------------------------------------------------------------
//	1.	日本語名
//		BMPファイルを読み込む
//	2.	引数
//			out		...	読み込んだアドレスを格納する変数のアドレス
//			ptr		...	メモリモードのポインタ
//	3.	返値
//			成功 true
//			失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp( COLORREF **out,int *width,int *height,char **ptr )
{
	BITMAPINFOHEADER	bih;
	// インフォヘッダを読み込む
	memcpy( &bih,*ptr,sizeof( bih ) );
	*ptr += sizeof( bih );
	// ファイルの種類を調べる
	if( bih.biCompression!=BI_RGB &&
		bih.biCompression!=BI_BITFIELDS ) return false;			// 対応形式（非圧縮）
	*out = (COLORREF*)LocalAlloc( LMEM_FIXED,sizeof( COLORREF )*bih.biWidth * bih.biHeight + 1 );
	if( *out == NULL ) return false;							// メモリ確保失敗
	// サイズの取得
	*width  = bih.biWidth;
	*height = bih.biHeight;
	// 変換( BMPファイル形式→COLORREF配列 )
	switch( bih.biBitCount ){
	case 1:		//2色パレット
		if( !GetBmp1(  &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 4:		//16色パレット
		if( !GetBmp4(  &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 8:		//256色パレット
		if( !GetBmp8(  &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 16:	// 16bitハイカラー
		if( !GetBmp16( &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 24:	// 24bitフルカラー
		if( !GetBmp24( &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 32:	// 24bit+8bitフルカラー
		if( !GetBmp32( &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	default:	// 非対応のビット数
		goto _error_exit;
	}
	return true;

	//	エラーの場合
_error_exit:
	LocalFree(*out);
	*out=NULL;
	return false;
}

// -------------------------------------------------------------
//	1.	日本語名
//		1bit BMPファイルを読み込んで変換する
//	2.	引数
//			bih		...	ビットマップインフォヘッダのアドレス
//			out		...	出力先メモリのアドレス
//	3.	返値
//			成功 true / 失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp1( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	RGBQUAD		p[2];		// パレット情報
	int			i,w,x,y;	// 雑用、座標
	BYTE		c;			// 読み込んだデータ
	BYTE		*pinp;		// 入力ポインタ
	// パレットを読み込む
	w=( (bih->biWidth+7)/8 +3) & 0xFFFFFFFC;
	if( !(bih->biClrUsed) ){ i=2; }else{ i=bih->biClrUsed; }
	memcpy( p,*ptr,sizeof( RGBQUAD )*i );
	*ptr += sizeof( RGBQUAD )*i;
	// 変換
	for( y=bih->biHeight-1; y>=0 ; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = *pinp;
			c = (c >> ( 7-(x & 7) )) & 1;
			if( (x & 7)==7 ) ++pinp;
			*pout = RGB( p[c].rgbRed,p[c].rgbGreen,p[c].rgbBlue );
			++pout;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		4bit BMPファイルを読み込んで変換する
//	2.	引数
//			bih		...	ビットマップインフォヘッダのアドレス
//			out		...	出力先メモリのアドレス
//	3.	返値
//			成功 true
//			失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp4( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	RGBQUAD		p[16];		// パレット情報
	int			i,w,x,y;	// 雑用、座標
	BYTE		c;			// 読み込んだデータ
	BYTE		*pinp;		// 入力ポインタ
	// パレットを読み込む
	w=( (bih->biWidth+1)/2 +3) & 0xFFFFFFFC;
	if( !(bih->biClrUsed) ){ i=16; }else{ i=bih->biClrUsed; }
	memcpy( p,*ptr,sizeof( RGBQUAD )*i );
	*ptr += sizeof( RGBQUAD )*i;
	// メモリの確保
	// 変換
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = *pinp;
			if( x & 1 ){
				c = c & 0x0F;
				++pinp;
			}else{
				c =(c >> 4) & 0x0F;
			}
			*pout = RGB( p[c].rgbRed,p[c].rgbGreen,p[c].rgbBlue );
			++pout;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		8bit BMPファイルを読み込んで変換する
//	2.	引数
//			bih		...	ビットマップインフォヘッダのアドレス
//			pout	...	出力先メモリのアドレス
//	3.	返値
//			成功 true
//			失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp8( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	RGBQUAD	p[256];			// パレット情報
	int		i,w,x,y;		// 雑用、座標
	BYTE	c;				// 読み込んだデータ
	BYTE	*pinp;			// 入力ポインタ
	// パレットを読み込む
	w=(bih->biWidth+3) & 0xFFFFFFFC;
	if( !(bih->biClrUsed) ){ i=256; }else{ i=bih->biClrUsed; }
	memcpy( p,*ptr,sizeof( RGBQUAD )*i );
	*ptr += sizeof( RGBQUAD )*i;
	// 変換
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = *pinp;
			*pout = RGB( p[c].rgbRed,p[c].rgbGreen,p[c].rgbBlue );
			++pout;
			++pinp;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		16bit BMPファイルを読み込んで変換する
//	2.	引数
//			bih		...	ビットマップインフォヘッダのアドレス
//			pout	...	出力先メモリのアドレス
//	3.	返値
//			成功 true
//			失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp16( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	int		w,x,y;			// 雑用、座標
	BYTE	r,g,b;			// ＲＧＢ
	WORD	c;				// 読み込んだデータ
	BYTE	*pinp;			// 入力ポインタ
	// 入力幅
	w=(bih->biWidth*2+3) & 0xFFFFFFFC;
	// 変換
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = (WORD)pinp[0]+((WORD)pinp[1]<<8);
			b = (c & 0x001F)     *255/31;
			g =((c & 0x07E0)>> 5)*255/63;
			r =((c & 0xF800)>>11)*255/31;
			*pout = RGB( r,g,b );
			pout++;
			pinp+=2;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		24bit BMPファイルを読み込んで変換する
//	2.	引数
//			bih		...	ビットマップインフォヘッダのアドレス
//			pout	...	出力先メモリのアドレス
//	3.	返値
//			成功 true
//			失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp24( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	int		w,x,y;			// 雑用、座標
	BYTE	r,g,b;			// ＲＧＢ
	BYTE	*pinp;			// 入力ポインタ
	// 入力幅
	w = (bih->biWidth*3+3) & 0xFFFFFFFC;
	// 変換
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			b = pinp[0];
			g = pinp[1];
			r = pinp[2];
			pinp+=3;
			*pout = RGB( r,g,b );
			pout++;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		32bit BMPファイルを読み込んで変換する
//	2.	引数
//			bih		...	ビットマップインフォヘッダのアドレス
//			pout	...	出力先メモリのアドレス
//	3.	返値
//			成功 true
//			失敗 false
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetBmp32( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	int		w,x,y;			// 雑用、座標
	BYTE	r,g,b;			// ＲＧＢ
	BYTE	*pinp;			// 入力ポインタ
	// 入力幅
	w = bih->biWidth*4;
	// 変換
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			b = pinp[0];
			g = pinp[1];
			r = pinp[2];
			pinp+=4;
			*pout = RGB( r,g,b );
			pout++;
		}
	}
	return true;
}

// =====================================================
// 設定関連

// -------------------------------------------------------------
//	1.	日本語名
//		パレットをＭＳＸ１の色にする
//	2.	引数
//		pal	...	(O)	パレットのアドレス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void set_msx1_palette( PAL *pal )
{
	memcpy( pal, msx1_palette, sizeof(PAL)*16 );
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットをＭＳＸ１の色にする
//	2.	引数
//		pal	...	(O)	パレットのアドレス
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void set_msx2_palette( PAL *pal ){
	memcpy( pal, msx2_palette, sizeof( PAL ) * 16 );
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定をデフォルトにする
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void GetDefCfg( SETTING *Mode )
{
	memset( Mode,0,sizeof( SETTING ) );
	memcpy( Mode->Col,msx1_palette,sizeof(PAL)*16 );
	Mode->Mode = MD_SC5;			// SCREEN5
	Mode->Gosaval = 0.43f;			// 誤差拡散係数 0.430
	Mode->err = 0;					// 切り捨て誤差 0
	Mode->Gosa = true;				// 誤差拡散 ON
	Mode->Pal = false;				// 固定パレット 未使用
	Mode->Inter = false;			// インターレース 未使用
	Mode->Resize = false;			// サイズ調節 未使用
	Mode->SelCol = 0;				// 色選択モード
	Mode->PltMode = PLT_BSAVE;		// ﾊﾟﾚｯﾄ出力ﾓｰﾄﾞ BSAVE に結合
	Mode->AutoName = true;			// 自動ファイル名決定
	Mode->AlgoMode = ALGO_RECALC;	// 自然画生成アルゴリズム
	Mode->JKrc = true;				// 色差情報の再計算
	Mode->ErrAlgo = EALGO_DITH2;	// エラー訂正アルゴリズム
	Mode->PreView = 3;				// ファイルとプレビューに出力
	Mode->Seido = SEIDO_24;			// 24bit 精度
	Mode->ErrAdd = EADD_ROTATE;		// ディザ加算方法 回転
	Mode->NonZero = false;			// ０番の色を使う
	Mode->FourceZero = FZ_NONE;		// 強制ゼロ化しない
	Mode->FZColor = 0;				// 強制ゼロ化する色
	Mode->Tile = false;				// タイル
	Mode->bDefault = false;			// 常にデフォルトコピー
	Mode->CnvMode = CM_LAST;		// 最終変換設定を採用
	Mode->GosaRatio = 0.5;			// 誤差拡散 X-Y比
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定を読み込む
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetCfgFile( SETTING *Mode,const char *sCfgFile )
{
	FU_FILE		*hFile;

	// 設定ファイルを開く
	GetDefCfg(Mode);	// ファイルが無ければデフォルト設定にする
	hFile = fu_open( sCfgFile, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		return false;
	}
	// 設定を読み込む
	fu_read( hFile, Mode, sizeof( SETTING ) );
	fu_close( hFile );
	// 不正な値を訂正
	Mode->PreView		= Mode->PreView & 3;
	if( Mode->PreView == 0 ) Mode->PreView = 1;
	Mode->ErrAlgo		= Mode->ErrAlgo % EALGO_MAX;
	Mode->AlgoMode		= Mode->AlgoMode % ALGO_MAX;
	Mode->SelCol		= Mode->SelCol % SELCOL_MAX;
	Mode->Mode			= Mode->Mode % MD_MAX;
	Mode->PltMode		= Mode->PltMode % PLT_MAX;
	Mode->Seido			= Mode->Seido % SEIDO_MAX;
	Mode->ErrAdd		= Mode->ErrAdd % EADD_MAX;
	Mode->FourceZero	= Mode->FourceZero % FZ_MAX;
	if( Mode->err > 255 ) Mode->err = 255;
	if( Mode->Gosaval < 0.0f ) Mode->Gosaval = 0.0f;
	if( Mode->Gosaval > 0.5f ) Mode->Gosaval = 0.5f;
	if( Mode->GosaRatio < 0.0f ) Mode->GosaRatio = 0.0f;
	if( Mode->GosaRatio > 1.0f ) Mode->GosaRatio = 1.0f;
	if( Mode->Mode == MD_SC2 || Mode->Mode == MD_SC3 ) {
		Mode->Inter = false;
	}

	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定を保存する
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void SaveCfgFile( SETTING *Mode,const char *sCfgFile ) {
	FU_FILE		*hFile;

	// 設定ファイルを開く
	hFile = fu_open( sCfgFile, "wb" );
	if( hFile == FU_INVALID_HANDLE ) return;		// ファイルが作れなければ保存しない

	// 設定を書き込む
	fu_write( hFile, Mode, sizeof( SETTING ) );
	fu_close( hFile );
}

// -------------------------------------------------------------
//	1.	日本語名
//		パスを読み込む
//	2.	引数
//		sPathFile	...	(I)	パスデータの入ったファイル名
//		sPath		...	(I)	パスを格納する変数
//		len			...	(I)	レコード長
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetPathFile( const char *sPathFile, char *sPath, int len ) {
	FU_FILE		*hFile;
	char		s_temp[ MAX_PATH ];
	char		s_path[ MAX_PATH ];
	char		*ptr;

	//	デフォルトを作成する
	strcpy( s_temp, GetCommandLine() );
	if( s_temp[ 0 ] == '\"' ) {
		strcpy( s_path, &s_temp[1] );
	} else {
		strcpy( s_path, s_temp );
	}
	ptr  = strrchr( s_path, '\\' );
	if( ptr != NULL ) ptr[1] = '\0';
	strcat( s_path, "plugin\\" );

	// パスファイルを開く
	hFile = fu_open( sPathFile, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		strcpy( sPath, s_path );
		return false;
	}
	if( !fu_read( hFile, sPath, len ) ) {
		strcpy( sPath, s_path );
	}
	fu_close( hFile );
	
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パスを保存する
//	2.	引数
//		sPathFile	...	(I)	パスデータを格納するファイル名
//		sPath		...	(I)	パスを格納してある変数
//		len			...	(I)	レコード長
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void SavePathFile( const char *sPathFile, char *sPath, int len ) {
	FU_FILE		*hFile;

	// パスファイルを開く
	hFile = fu_open( sPathFile, "wb" );
	if( hFile == FU_INVALID_HANDLE ){
		return;		// ファイルが作れなければ保存しない
	}
	// 設定を書き込む
	fu_write( hFile, sPath, len );
	fu_close( hFile );
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定をデフォルトにする
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void GetDefCustom( void ) {

	memcpy( convert7to255_r, init_convert7to255_r, sizeof( init_convert7to255_r ) );
	memcpy( convert7to255_g, init_convert7to255_g, sizeof( init_convert7to255_g ) );
	memcpy( convert7to255_b, init_convert7to255_b, sizeof( init_convert7to255_b ) );

	memcpy( convert7to255_s8r, init_convert7to255_s8r, sizeof( init_convert7to255_s8r ) );
	memcpy( convert7to255_s8g, init_convert7to255_s8g, sizeof( init_convert7to255_s8g ) );
	memcpy( convert3to255_s8b, init_convert3to255_s8b, sizeof( init_convert3to255_s8b ) );

	memcpy( convert31to255_s12r, init_convert31to255_s12r, sizeof( init_convert31to255_s12r ) );
	memcpy( convert31to255_s12g, init_convert31to255_s12g, sizeof( init_convert31to255_s12g ) );
	memcpy( convert31to255_s12b, init_convert31to255_s12b, sizeof( init_convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定を読み込む
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
bool GetCustomFile( const char *sCustomFile ) {
	FU_FILE *hFile;

	// 設定ファイルを開く
	GetDefCustom();			// ファイルが無ければデフォルト設定にする
	hFile = fu_open( sCustomFile, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		return false;
	}
	// 設定を読み込む
	fu_read( hFile, convert7to255_r, sizeof( convert7to255_r ) );
	fu_read( hFile, convert7to255_g, sizeof( convert7to255_g ) );
	fu_read( hFile, convert7to255_b, sizeof( convert7to255_b ) );
	fu_read( hFile, convert7to255_s8r, sizeof( convert7to255_s8r ) );
	fu_read( hFile, convert7to255_s8g, sizeof( convert7to255_s8g ) );
	fu_read( hFile, convert3to255_s8b, sizeof( convert3to255_s8b ) );
	fu_read( hFile, convert31to255_s12r, sizeof( convert31to255_s12r ) );
	fu_read( hFile, convert31to255_s12g, sizeof( convert31to255_s12g ) );
	fu_read( hFile, convert31to255_s12b, sizeof( convert31to255_s12b ) );
	fu_close( hFile );
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		設定を保存する
//	2.	引数
//		なし
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void SaveCustomFile( const char *sCustomFile ) {
	FU_FILE *hFile;

	// 設定ファイルを開く
	hFile = fu_open( sCustomFile, "wb" );
	if( hFile == FU_INVALID_HANDLE ) return;		// ファイルが作れなければ保存しない

	// 設定を書き込む
	fu_write( hFile, convert7to255_r, sizeof( convert7to255_r ) );
	fu_write( hFile, convert7to255_g, sizeof( convert7to255_g ) );
	fu_write( hFile, convert7to255_b, sizeof( convert7to255_b ) );
	fu_write( hFile, convert7to255_s8r, sizeof( convert7to255_s8r ) );
	fu_write( hFile, convert7to255_s8g, sizeof( convert7to255_s8g ) );
	fu_write( hFile, convert3to255_s8b, sizeof( convert3to255_s8b ) );
	fu_write( hFile, convert31to255_s12r, sizeof( convert31to255_s12r ) );
	fu_write( hFile, convert31to255_s12g, sizeof( convert31to255_s12g ) );
	fu_write( hFile, convert31to255_s12b, sizeof( convert31to255_s12b ) );
	fu_close( hFile );
}

