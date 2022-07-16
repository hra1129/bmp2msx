// -------------------------------------------------------------
// BMP to MSX
// ＢＳＡＶＥ保存
//										(C)2000 HRA!
// -------------------------------------------------------------

#include "bsave.h"
#include "utils.h"
#include "fileuty.h"

// -------------------------------------------------------------
// 設定
// -------------------------------------------------------------
static const char* smode="235678AC5678AC";			// ファイル名の最後につける記号

// -------------------------------------------------------------
// プロトタイプ宣言
// -------------------------------------------------------------
static int _SaveHeader( FU_FILE *hf, int mode, int width, int height, int paltbl );
static int _SaveBodyS2( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SaveBodyS3( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SaveBodyAL( FU_FILE *hf, const byte* ptr, int width, int height, int pitch, int *outadr );
static int _SaveColorS2( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SavePaletteTable( FU_FILE *hf, int paltbl, PAL *pal, int *outadr );
static void _GetOutFilename( char* szOutFile, const char* szInFile, int nScrMode, int nInter, bool pal );
static void _SaveCSVFile( HWND hWnd, const char *szFileName, SETTING *Mode );

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
	int				swidth, sheight, interc, outadr;
	byte*			ptr;
	FU_FILE			*hf;
	int				paltbl	= PalTblAdr[ Mode->Mode ];
	bool			inter	= Mode->Inter;
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

	// インターレースと通常セーブの区別
	if( inter ){
		interc  = 2;			// ファイル数
		swidth  = width*2;
		sheight = height/2;
	}else{
		interc	= 1;
		swidth  = width;
		sheight = height;
	}

	//	保存する
	ptr = bmp;
	while( interc ){

		//	ファイル名
		hf = FU_INVALID_HANDLE;
		if( Mode->AutoName ){	//	自動決定の場合
			_GetOutFilename( szFileName, szInFileName, Mode->Mode, interc-1+( inter?1:0 ), (Mode->PltMode==PLT_BSAVE ) );
		}else{					//	手動決定の場合
			if( !GetName( hWnd, szFileName, MAX_PATH, szTitle[ interc-1+( inter?1:0 ) ], cszDefExp, NULL ) ){
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
			ecode = _SaveHeader( hf, Mode->Mode, width, sheight, paltbl );
		} else {
			ecode = _SaveHeader( hf, Mode->Mode, width, sheight, 0 );
		}
		if( ecode != BSV_NOERR ) break;

		// ピクセルの保存
		switch( Mode->Mode ) {
		case MD_SC2:	//	SCREEN 2
			ecode = _SaveBodyS2( hf, ptr, &outadr );
			break;
		case MD_SC3:	//	SCREEN 3
			ecode = _SaveBodyS3( hf, ptr, &outadr );
			break;
		default:		//	SCREEN 5 以降
			ecode = _SaveBodyAL( hf, ptr, width, sheight, swidth, &outadr );
		}
		if( ecode != BSV_NOERR ) break;
		
		// パレットテーブルが有れば保存する
		if( paltbl!=0 && Mode->PltMode==PLT_BSAVE && Mode->Mode < MD_SC5_256L ) {
			ecode = _SavePaletteTable( hf, paltbl, Mode->Col, &outadr );
			if( ecode != BSV_NOERR ) break;
		}

		//	カラーテーブル( SCREEN 2/4 )
		if( Mode->Mode == MD_SC2 ) {
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
				if( GetName( hWnd,szFileName,MAX_PATH,PLSave ,cszDefPL, NULL ) ) {
					bsvSavePLFile( hWnd,szFileName,Mode );
				}
				break;
			case PLT_CSV:	// ＣＳＶ形式
				if( GetName( hWnd,szFileName,MAX_PATH,CSVSave,cszDefCSV, CfgCsv ) ) {
					_SaveCSVFile( hWnd,szFileName,Mode );
				}
				break;
			}
		}
		// 次のファイル
		ptr=bmp+width;
		interc--;
	}
	if( hf != FU_INVALID_HANDLE ) fu_close( hf );
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
		Buf[ i*2+0 ] = (Mode->Col[i].red<<4) | Mode->Col[i].blue;
		Buf[ i*2+1 ] =  Mode->Col[i].green;
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
void bsvLoadPLBsaveFile( SETTING *mode, const char *szFileName ) {
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
			mode->Col[ i ].red  = (Buf[ i * 2 + 0 ] >> 4 ) & 0x07;
			mode->Col[ i ].blue =  Buf[ i * 2 + 0 ]        & 0x07;
			mode->Col[ i ].green=  Buf[ i * 2 + 1 ]        & 0x07;
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
static int _SavePaletteTable( FU_FILE *hf, int paltbl, PAL *pal, int *outadr ) {
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
	for( i=0; i<height; i++ ) {
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
static int _SaveHeader( FU_FILE *hf, int mode, int width, int height, int paltbl ) {
	BSAVEHEADER		bsh;

	//	ヘッダの構築
	bsh.type	= 0xFE;
	bsh.start	= 0;
	switch( mode ) {
	case MD_SC2:
		bsh.end	= 0x37FF;
		break;
	default:
		if( paltbl ) {
			bsh.end	= paltbl + 32 - 1;
		} else {
			bsh.end	= width * height - 1;
		}
	}
	bsh.run		= 0;

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
		return;		// ファイルが作れない
	}

	// データを出力する
	fu_write( hf, cszCSVHead, lstrlen(cszCSVHead) );
	palnum = ( Mode->Mode == MD_SC6 || Mode->Mode == MD_SC6_256L ) ? 4 : 16;
	for( i = 0; i < palnum; ++i ){
		Buf[ 0 ] = (Mode->Col[i].red<<4) | Mode->Col[i].blue;
		Buf[ 1 ] =  Mode->Col[i].green;
		wsprintf( szBuf,"%02X,%02X%s", Buf[0], Buf[1], (i==15)?"\x0D\x0A":"," );
		fu_write( hf, szBuf, lstrlen(szBuf) );
	}
	// 終了
	fu_close( hf );
}
