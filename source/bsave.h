// -------------------------------------------------------------
// BMP to MSX
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _BSAVE_H_
#define _BSAVE_H_

#include <windows.h>
#include "convert.h"

// BSAVE 形式ファイルヘッダ
#pragma pack( push,bsave,1 )
typedef struct {
	unsigned char	type;		// 0xFE;
	unsigned short	start;		// 開始アドレス
	unsigned short	end;		// 終了アドレス
	unsigned short	run;		// 実行開始アドレス
} BSAVEHEADER;
#pragma pack( pop,bsave )

// パレットテーブルアドレス
static const int PalTblAdr[]={
	0x01B80L,		// screen 2/4
	0x02020L,		// screen 3
	0x07680L,		// screen 5
	0x07680L,		// screen 6
	0x0fA80L,		// screen 7
	0,				// screen 8
	0,				// screen 10/11
	0,				// screen 12
	0x07680L,		// screen 5 (256 lines)
	0x07680L,		// screen 6 (256 lines)
	0x0fA80L,		// screen 7 (256 lines)
	0,				// screen 8 (256 lines)
	0,				// screen 10/11 (256 lines)
	0,				// screen 12 (256 lines)
};

enum {
	BSV_NOERR = 0,
	BSV_ERR_CREATE,
	BSV_ERR_WRITE,
};

int bsvSaveBmp( HWND hWnd,const char *szInFileName,LPBYTE bmp,int width,int height,SETTING *Mode );
void bsvSavePLFile( HWND hWnd, const char *szFileName, SETTING *Mode );
void bsvLoadPLBsaveFile( SETTING *mode, const char* szFileName );

#endif	// _BSAVE_H_
