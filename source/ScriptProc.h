// -------------------------------------------------------------
// BMP to MSX
// スクリプトウィンドウ
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _SCRIPTPROC_H_
#define _SCRIPTPROC_H_

#include "winEvent.h"

// ウィンドウ関数
WINPROC( ScriptProc );

// スクリプト実行
void scrPlayScript( char *szText,char *szFileName );

#endif
