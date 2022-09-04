// -------------------------------------------------------------
// BMP to MSX
// ＢＳＡＶＥ保存
//										(C)2000 HRA!
// -------------------------------------------------------------

#include <cstring>
#include "bsave.h"
#include "utils.h"
#include "fileuty.h"

// -------------------------------------------------------------
// 設定
// -------------------------------------------------------------
static const char* smode="235678AC5678AC";			// ファイル名の最後につける記号

// -------------------------------------------------------------
// 保存用クラス
// -------------------------------------------------------------
class C_BSAVE {
private:
	uint32_t	screen_mode;
	bool	interlace;
	uint32_t	palette_mode;
};

// -------------------------------------------------------------
// プロトタイプ宣言
// -------------------------------------------------------------
static int _SaveHeader( FU_FILE *hf, int screen_mode, int width, int height, int paltbl );
static int _SaveBodyS2( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SaveBodyS3( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SaveBodyAL( FU_FILE *hf, const byte* ptr, int width, int height, int pitch, int *outadr );
static int _SaveColorS2( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SavePaletteTable( FU_FILE *hf, int paltbl, C_PALETTE *pal, int *outadr );
static void _GetOutFilename( char* szOutFile, const char* szInFile, int nScrMode, int nInter, bool pal );
static void _SaveCSVFile( HWND hWnd, const char *szFileName, SETTING *Mode );
static bool _save_basic_file( SETTING *Mode, const char *szFileName );

// -------------------------------------------------------------
//	1.	日本語名
//		BSAVE 形式で画像を保存する
//	2.	引数
//		hWnd	...	(I)	ウィンドウハンドル
//		bmp		...	(I)	画像データのアドレス
//		width	...	(I)	幅
//		height	...	(I)	高さ
//		inter	...	(I)	インターレース
//		paltbl	...	(I)	パレットテーブルのアドレス( 0 なら付加しない )
//	3.	返値
//		エラーコード
//	4.	備考
//		なし
// -------------------------------------------------------------
int bsvSaveBmp( HWND hWnd, const char *szInFileName, byte* bmp, int width, int height, SETTING *Mode ) {
	char			szFileName[ MAX_PATH ];
	int				save_width, save_height, interc, outadr, pitch;
	byte*			ptr;
	FU_FILE			*hf;
	int				paltbl	= PalTblAdr[ Mode->screen_mode ];
	bool			inter	= Mode->interlace;
	int				ecode	= 0;
#ifdef _ENGLISH
	const char		*szTitle[]={
						"Save As",
						"Save As ( Odd line )",
						"Save As ( Even line )",
					};
#else
	const char		*szTitle[]={
						"名前を付けて保存",
						"名前を付けて保存（奇数ライン）",
						"名前を付けて保存（偶数ライン）",
					};
#endif

	switch( Mode->screen_mode ){
	case MD_SC5:
	case MD_SC7:
		save_width = width >> 1;
		break;
	case MD_SC5_256L:
	case MD_SC7_256L:
		save_width = width >> 1;
		break;
	case MD_SC6:
		save_width = width >> 2;
		break;
	case MD_SC6_256L:
		save_width = width >> 1;
		break;
	default:
		save_width = width;
		break;
	}
	pitch = save_width;

	// インターレースと通常セーブの区別
	if( inter ){
		interc  = 2;			// ファイル数
		save_height = height / 2;
		pitch = pitch << 1;
		ptr = bmp + save_width;
	}else{
		interc	= 1;
		save_height = height;
		ptr = bmp;
	}

	//	保存する
	while( interc ){

		//	ファイル名
		hf = FU_INVALID_HANDLE;
		if( Mode->AutoName ){	//	自動決定の場合
			_GetOutFilename( szFileName, szInFileName, Mode->screen_mode, interc - 1 + ( inter ? 1 : 0 ), ( Mode->PltMode == PLT_BSAVE ) );
		}else{					//	手動決定の場合
			if( !GetName( hWnd, szFileName, MAX_PATH, szTitle[ interc - 1 + ( inter ? 1 : 0 ) ], cszDefExp, NULL ) ){
				break;
			}
		}
		
		//	ファイルを開く
		hf = fu_open( szFileName, "wb" );
		if( hf == FU_INVALID_HANDLE ) {
			ecode = BSV_ERR_CREATE;
			break;
		}

		//	ヘッダ
		if( Mode->PltMode == PLT_BSAVE ) {
			ecode = _SaveHeader( hf, Mode->screen_mode, save_width, save_height, paltbl );
		} else {
			ecode = _SaveHeader( hf, Mode->screen_mode, save_width, save_height, 0 );
		}
		if( ecode != BSV_NOERR ) break;

		// ピクセルの保存
		switch( Mode->screen_mode ) {
		case MD_SC2:	//	SCREEN 2
			ecode = _SaveBodyS2( hf, ptr, &outadr );
			break;
		case MD_SC3:	//	SCREEN 3
			ecode = _SaveBodyS3( hf, ptr, &outadr );
			break;
		default:		//	SCREEN 5 以降
			ecode = _SaveBodyAL( hf, ptr, save_width, save_height, pitch, &outadr );
		}
		if( ecode != BSV_NOERR ) break;
		
		// パレットテーブルが有れば保存する
		if( paltbl != 0 && Mode->PltMode == PLT_BSAVE && Mode->screen_mode < MD_SC5_256L ){
			ecode = _SavePaletteTable( hf, paltbl, Mode->color_palette, &outadr );
			if( ecode != BSV_NOERR ) break;
		}

		//	カラーテーブル( SCREEN 2/4 )
		if( Mode->screen_mode == MD_SC2 ) {
			ecode = _SaveColorS2( hf, ptr, &outadr );
			if( ecode != BSV_NOERR ) break;
		}

		// ファイルを閉じる
		fu_close( hf );
		hf = FU_INVALID_HANDLE;

		// パレットファイルを出力する場合
		if( paltbl != 0 ){
			switch( Mode->PltMode ) {
			case PLT_PL:	// グラフサウルス *.PL? 形式
				if( GetName( hWnd, szFileName, MAX_PATH, PLSave, cszDefPL, NULL ) ){
					bsvSavePLFile( hWnd, szFileName, Mode );
				}
				break;
			case PLT_CSV:	// ＣＳＶ形式
				if( GetName( hWnd, szFileName, MAX_PATH, CSVSave, cszDefCSV, CfgCsv ) ){
					_SaveCSVFile( hWnd, szFileName, Mode );
				}
				break;
			}
		}
		// 次のファイル
		ptr = bmp;
		interc--;
	}
	if( hf != FU_INVALID_HANDLE ) {
		fu_close( hf );
	}
	if( Mode->output_basic_enable ) {
		if( !_save_basic_file( Mode, szFileName ) ) {
			MessageBox( hWnd, WriteErr, MsgCap, MB_OK | MB_ICONWARNING );
		}
	}
	return ecode;
}

// -------------------------------------------------------------
//	1.	日本語名
//		PL? 形式でパレットを保存する
//	2.	引数
//		szFileName	...	(I)	ファイル名
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void bsvSavePLFile( HWND hWnd, const char *szFileName, SETTING *Mode ) {
	FU_FILE	*hf;
	byte	Buf[32];
	int		i;

	// ファイルを開く
	hf = fu_open( szFileName, "wb" );
	if( hf == FU_INVALID_HANDLE ) {
		MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
		return;		// ファイルが作れない
	}
	// 出力データを作成する
	for( i=0;i<16;++i ) {
		Buf[ i*2+0 ] = (Mode->color_palette[i].red<<4) | Mode->color_palette[i].blue;
		Buf[ i*2+1 ] =  Mode->color_palette[i].green;
	}
	// データを出力する
	for( i=0;i<8;++i ) {
		fu_write( hf,Buf, sizeof( Buf ) );
	}
	// 終了
	fu_close( hf );
}

// -------------------------------------------------------------
//	1.	日本語名
//		BSAVEﾌｧｲﾙ内のﾊﾟﾚｯﾄを読み込む
//	2.	引数
//		mode		...	(I)	ﾊﾟﾚｯﾄ格納先
//		szFileName	...	(I)	ファイル名
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
void bsvLoadPLBsaveFile( SETTING *screen_mode, const char *szFileName ) {
	FU_FILE	*hf;
	byte	Buf[ 32 ];
	int		i, adr;

	//	拡張子から画面ﾓｰﾄﾞを判定する
	i = szFileName[ strlen( szFileName )-1 ];
	if( (i=='0') || (i=='1') ) {
		//	インターレース画像
		i = szFileName[ strlen( szFileName )-2 ];
	}
	switch( i ) {
	case '2':	i = MD_SC2;	break;
	case '3':	i = MD_SC3;	break;
	case '4':	i = MD_SC2;	break;
	case '5':	i = MD_SC5;	break;
	case '6':	i = MD_SC6;	break;
	case '7':	i = MD_SC7;	break;
	default:	return;				//	モード不明画像
	}

	//	パレットテーブルのオフセット = アドレス + sizeof( BSAVE Header ) を取得
	adr = PalTblAdr[ i ] + 7;

	//	ファイルを開く
	hf = fu_open( szFileName, "rb" );
	if( hf == FU_INVALID_HANDLE ) return;

	//	パレットテーブルをシーク
	fu_seek( hf, adr, FU_SEEK_TOP );

	// データを入力する
	memset( Buf, 0, sizeof( Buf ) );
	if( fu_read( hf, Buf, sizeof(Buf) ) ) {
		// 入力データを反映する
		for( i = 0; i < 16; ++i ) {
			screen_mode->color_palette[ i ].red  = (Buf[ i * 2 + 0 ] >> 4 ) & 0x07;
			screen_mode->color_palette[ i ].blue =  Buf[ i * 2 + 0 ]        & 0x07;
			screen_mode->color_palette[ i ].green=  Buf[ i * 2 + 1 ]        & 0x07;
		}
	}
	// 終了
	fu_close( hf );
}

// =============================================================
//	ローカル関数

// -------------------------------------------------------------
//	1.	日本語名
//		カラーテーブルの保存
//	2.	引数
//		hf		...	(I)	ファイルハンドル
//		ptr		...	(I)	MSX VRAM イメージ
//		outadr	...	(I/O)	出力バイト数を格納している変数のアドレス
//	3.	返値
//		エラーコード
//	4.	備考
//		なし
// -------------------------------------------------------------
static int _SaveColorS2( FU_FILE *hf, const byte* ptr, int *outadr ) {
	byte	dmy;

	// カラーテーブルまでを 0 で埋める
	dmy = 0;
	while( *outadr < 0x2000 ) {
		if( !fu_write( hf, &dmy, 1 ) ) return BSV_ERR_WRITE;
		(*outadr)++;
	}
	//	カラーテーブル
	if( !fu_write( hf, ptr + 0x2000, 0x1800 ) ) return BSV_ERR_WRITE;
	*outadr += 0x1800;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	日本語名
//		パレットテーブルの保存
//	2.	引数
//		hf		...	(I)	ファイルハンドル
//		paltbl	...	(I)	パレットテーブルのアドレス
//		pal		...	(I)	パレット値
//		outadr	...	(I/O)	出力バイト数を格納している変数のアドレス
//	3.	返値
//		エラーコード
//	4.	備考
//		なし
// -------------------------------------------------------------
static int _SavePaletteTable( FU_FILE *hf, int paltbl, C_PALETTE *pal, int *outadr ) {
	int		i;
	byte	dmy[] = { 0, 0 };

	// パレットテーブルまでを 0 で埋める
	while( *outadr < paltbl ) {
		if( !fu_write( hf, dmy, 1 ) ) return BSV_ERR_WRITE;
		(*outadr)++;
	}

	// パレットを出力する
	for( i = 0; i < 16; ++i ) {
		dmy[0]=(byte)(( pal[i].red<<4 ) | pal[i].blue );
		dmy[1]=(byte)(  pal[i].green );
		if( !fu_write( hf, dmy, 2 ) ) return BSV_ERR_WRITE;
		*outadr += 2;
	}

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	日本語名
//		Screen2 データの本体を保存する
//	2.	引数
//		hf		...	(I)	ファイルハンドル
//		ptr		...	(I)	MSX VRAMイメージ
//		outadr	...	(O)	出力データ数を格納するアドレス
//	3.	返値
//		エラーコード
//	4.	備考
//		エラー時は出力バイト数を返さない
// -------------------------------------------------------------
static int _SaveBodyS2( FU_FILE *hf, const byte* ptr, int *outadr ) {
	int		i;
	byte	sPGen[ 768 ];

	//	パターンジェネレータテーブル
	if( !fu_write( hf, ptr, 0x1800 ) ) return BSV_ERR_WRITE;

	//	パターンネームテーブル
	for( i = 0; i < 768; ++i ) {
		sPGen[ i ] = (unsigned char)( i & 255 );
	}
	if( !fu_write( hf, sPGen, 768 ) ) return BSV_ERR_WRITE;

	//	出力バイト数
	*outadr = 0x1800 + 768;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	日本語名
//		Screen3 データの本体を保存する
//	2.	引数
//		hf		...	(I)	ファイルハンドル
//		ptr		...	(I)	MSX VRAMイメージ
//		outadr	...	(O)	出力データ数を格納するアドレス
//	3.	返値
//		エラーコード
//	4.	備考
//		エラー時は出力バイト数を返さない
// -------------------------------------------------------------
static int _SaveBodyS3( FU_FILE *hf, const byte* ptr, int *outadr ) {
	int		i, j, k;
	byte	sPGen[ 768 ];

	//	パターンジェネレータテーブル
	if( !fu_write( hf, ptr, 0x800 ) ) return BSV_ERR_WRITE;

	//	パターンネームテーブル
	for( i = 0; i < 6; ++i ) {
		for( j = 0; j < 32; ++j ) {
			for( k = 0; k < 4; ++k ) {
				sPGen[ k*32 + j + i*128 ] = (unsigned char)( j + i*32 );
			}
		}
	}
	if( !fu_write( hf, sPGen, 768 ) ) return BSV_ERR_WRITE;

	//	出力バイト数
	*outadr = 0x800 + 768;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	日本語名
//		データの本体を保存する
//	2.	引数
//		hf		...	(I)	ファイルハンドル
//		ptr		...	(I)	MSX VRAMイメージ
//		width	...	(I)	画像幅（バイト数）
//		height	...	(I)	画像高さ
//		pitch	...	(I)	移動単位（バイト数）
//		outadr	...	(O)	出力データ数を格納するアドレス
//	3.	返値
//		エラーコード
//	4.	備考
//		エラー時は出力バイト数を返さない
// -------------------------------------------------------------
static int _SaveBodyAL( FU_FILE *hf, const byte* ptr, int width, int height, int pitch, int *outadr ) {
	int	i;

	*outadr = 0;
	for( i = 0; i < height; i++ ){
		if( !fu_write( hf, ptr + i * pitch, width ) ) return BSV_ERR_WRITE;
		*outadr += width;
	}

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	日本語名
//		bsave ヘッダを書き込む
//	2.	引数
//		hf	...	(I)	ファイルハンドル
//		mode	...	(I)	MSXスクリーンモード
//		width	...	(I)	画像幅
//		height	...	(I)	画像高さ
//		paltbl	...	(I)	パレットテーブル（保存しない場合は 0）
//	3.	返値
//		エラーコード
//	4.	備考
//		なし
// -------------------------------------------------------------
static int _SaveHeader( FU_FILE *hf, int screen_mode, int width, int height, int paltbl ) {
	BSAVEHEADER		bsh =  { 0 };

	//	ヘッダの構築
	bsh.type	= 0xFE;
	bsh.start	= 0;
	switch( screen_mode ) {
	case MD_SC2:
		bsh.end	= 0x37FF;
		break;
	default:
		if( screen_mode >= MD_SC5_256L ) {
			bsh.end = width * height - 1;
		}
		else {
			if( paltbl ){
				bsh.end = paltbl + 32 - 1;
			}
			else{
				bsh.end = width * height - 1;
			}
		}
	}
	//	未使用の run の部分に実際のサイズを入れておく
	bsh.run		= bsh.end;
	//	end が 0xFFFF になってしまう場合は、0xFFFE に置き換える
	if( bsh.end == 0xFFFF ) {
		bsh.end = 0xFFFE;
	}

	//	保存
	if( !fu_write( hf, &bsh, sizeof(BSAVEHEADER) ) ) return BSV_ERR_WRITE;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイル名を決定する
//	2.	引数
//		szOutFile	...	(I)	ファイル名を格納するアドレス
//		szInFile	...	(I)	ソースのファイル名 (*.BMP)
//		nScrMode	...	(I)	出力画面モード
//		nInter		...	(I)	0 = ノーマル
//							1 = インターレース(Page0)
//							2 = インターレース(Page1)
//		pal			...	(I)	false = パレット無し
//							true = パレット有り
//	3.	返値
//		なし
//	4.	備考
//		ソースのファイル名は必ず "～.BMP" の形を取っていなけ
//		ればならない。ファイル名格納アドレスはソースのファイ
//		ル名以上のサイズでなければならない。
// -------------------------------------------------------------
static void _GetOutFilename( char* szOutFile, const char* szInFile, int nScrMode, int nInter, bool pal ) {
	char *p;
	char tmp[]={ ".SRX" };

	if( pal && (nScrMode<MD_SC8 || (nScrMode>=MD_SC5_256L && nScrMode<MD_SC8_256L)) ) {
		tmp[2]='C';
	}else{
		tmp[2]='R';
	}
	switch( nInter ){
	case 1:
		tmp[2]=smode[nScrMode];
		tmp[3]='0';
		break;
	case 2:
		tmp[2]=smode[nScrMode];
		tmp[3]='1';
		break;
	default:
		tmp[3]=smode[nScrMode];
	}
	strcpy( szOutFile,szInFile );
	p=strrchr( szOutFile,'.' );
	strcpy( p,tmp );
}

// -------------------------------------------------------------
//	1.	日本語名
//		CSV 形式でパレットを保存する
//	2.	引数
//		szFileName	...	(I)	ファイル名
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void _SaveCSVFile( HWND hWnd, const char *szFileName, SETTING *Mode ) {
	FU_FILE	*hf;
	byte	Buf[2];
	int		i;
	char	szBuf[32];
	int		palnum;

	// ファイルを開く
	hf = fu_open( szFileName, "wb" );
	if( hf == FU_INVALID_HANDLE ) {
		MessageBox( hWnd, WriteErr, MsgCap,MB_OK | MB_ICONWARNING );
		return;		// ファイルを作れない
	}

	// データを出力する
	fu_write( hf, cszCSVHead, lstrlen(cszCSVHead) );
	palnum = ( Mode->screen_mode == MD_SC6 || Mode->screen_mode == MD_SC6_256L ) ? 4 : 16;
	for( i = 0; i < palnum; ++i ){
		Buf[ 0 ] = (Mode->color_palette[i].red<<4) | Mode->color_palette[i].blue;
		Buf[ 1 ] =  Mode->color_palette[i].green;
		wsprintf( szBuf,"%02X,%02X%s", Buf[0], Buf[1], (i==15)?"\x0D\x0A":"," );
		fu_write( hf, szBuf, lstrlen(szBuf) );
	}
	// 終了
	fu_close( hf );
}

// -------------------------------------------------------------
//	1.	日本語名
//		BASICファイルを保存する
//	2.	引数
//		szFileName	...	(I)	ファイル名
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static bool _save_basic_file( SETTING *Mode, const char *szFileName ) {
	char s_basic_name[ MAX_PATH + 3 ];
	char *s_file_name = s_basic_name;
	char *p, *pp;
	FU_FILE *hf;
	int line_no = 100;
	int i;

	strcpy( s_basic_name, szFileName );
	p = strrchr( s_basic_name, '.' );
	strcpy( p, ".BAS" );
	// ファイルを開く
	hf = fu_open( s_basic_name, "w" );
	if( hf == FU_INVALID_HANDLE ){
		return false;		// ファイルを作れない
	}

	fu_printf( hf, "%d DEFINT A-Z:COLOR 15,0,0:SCREEN %d", line_no, screen_mode[ Mode->screen_mode ] );
	if( Mode->interlace ) {
		fu_printf( hf, ",,,,,3" );
	}
	fu_printf( hf, "\r\n" );
	line_no += 10;

	if( Mode->screen_mode >= MD_SC5_256L ){
		//	256ラインモードの場合は、スプライトを表示禁止にする
		fu_printf( hf, "%d VDP(9)=VDP(9)OR2\r\n", line_no );
		line_no += 10;
	}

	if( screen_mode[ Mode->screen_mode ] < 4 ) {
		fu_printf( hf, "%d IF PEEK(&H2D) > 1 THEN FOR I=0 TO 15:COLOR=(I,0,0,0):NEXT\r\n", line_no );
		line_no += 10;
	}
	else if( screen_mode[ Mode->screen_mode ] < 8 || screen_mode[ Mode->screen_mode ] == 10 ) {
		fu_printf( hf, "%d FOR I=0 TO 15:COLOR=(I,0,0,0):NEXT\r\n", line_no );
		line_no += 10;
	}
	else {
		//	パレット無しのモード ( SCREEN 8, 12 )
	}

	strcpy( s_file_name, szFileName );
	p = strrchr( s_file_name, '.' );
	pp = strrchr( s_file_name, '\\' );
	if( pp == NULL ) {
		pp = strrchr( s_file_name, '/' );
		if( pp == NULL ) {
			pp = s_file_name;
		}
		else {
			pp++;
		}
	}
	else {
		pp++;
	}
	if( Mode->interlace ) {
		p[ 3 ] = '0';
		fu_printf( hf, "%d SET PAGE 0,0:BLOAD\"%s\",S\r\n", line_no, pp );
		line_no += 10;
		p[ 3 ] = '1';
		fu_printf( hf, "%d SET PAGE 1,1:BLOAD\"%s\",S\r\n", line_no, pp );
		line_no += 10;
	}
	else {
		fu_printf( hf, "%d BLOAD\"%s\",S\r\n", line_no, pp );
		line_no += 10;
	}

	if( Mode->PltMode == PLT_BSAVE ) {
		if( screen_mode[ Mode->screen_mode ] < 4 ){
			fu_printf( hf, "%d IF PEEK(&H2D) > 1 THEN COLOR=RESTORE\r\n", line_no );
			line_no += 10;
		}
		else if( screen_mode[ Mode->screen_mode ] < 8 || screen_mode[ Mode->screen_mode ] == 10 ){
			fu_printf( hf, "%d COLOR=RESTORE\r\n", line_no );
			line_no += 10;
		}
		else{
			//	パレット無しのモード ( SCREEN 8, 12 )
		}
	}
	else {
		if( screen_mode[ Mode->screen_mode ] < 4 ){
			fu_printf( hf, "%d P$=\"", line_no );
			for( i = 0; i < 16; i++ ){
				fu_printf( hf, "%d%d%d", Mode->color_palette[ i ].red, Mode->color_palette[ i ].green, Mode->color_palette[ i ].blue );
			}
			fu_printf( hf, "\"\r\n" );
			line_no += 10;

			fu_printf( hf, "%d IF PEEK(&H2D) > 1 THEN FOR I=0 TO 15:R=VAL(MID$(P$,I*3+1,1)):G=VAL(MID$(P$,I*3+2,1)):B=VAL(MID$(P$,I*3+3,1)):COLOR=(I,R,G,B):NEXT\r\n", line_no );
			line_no += 10;
		}
		else if( screen_mode[ Mode->screen_mode ] < 8 || screen_mode[ Mode->screen_mode ] == 10 ){
			fu_printf( hf, "%d P$=\"", line_no );
			for( i = 0; i < 16; i++ ){
				fu_printf( hf, "%d%d%d", Mode->color_palette[ i ].red, Mode->color_palette[ i ].green, Mode->color_palette[ i ].blue );
			}
			fu_printf( hf, "\"\r\n" );
			line_no += 10;

			fu_printf( hf, "%d FOR I=0 TO 15:R=VAL(MID$(P$,I*3+1,1)):G=VAL(MID$(P$,I*3+2,1)):B=VAL(MID$(P$,I*3+3,1)):COLOR=(I,R,G,B):NEXT\r\n", line_no );
			line_no += 10;
		}
		else{
			//	パレット無しのモード ( SCREEN 8, 12 )
		}
	}
	if( Mode->screen_mode >= MD_SC5_256L ) {
		fu_printf( hf, "%d A$=INKEY$:if A$<>" "THEN VDP(24)=(VDP(24)+1)AND255:FORI=0TO100:NEXT:GOTO %d\r\n", line_no, line_no );
		line_no += 10;
	}
	else {
		fu_printf( hf, "%d A$=INPUT$(1)\r\n", line_no );
		line_no += 10;
	}

	if( Mode->interlace ) {
		fu_printf( hf, "%d SCREEN,,,,,0\r\n", line_no );
		line_no += 10;
	}

	// 終了
	fu_close( hf );
	return true;
}
