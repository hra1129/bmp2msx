// -------------------------------------------------------------
// BMP to MSX
//										(C)2000/5/6 HRA!
// -------------------------------------------------------------

#ifndef _SETTING_H_
#define	_SETTING_H_

#include "winEvent.h"
#include "Convert.h"

// エクスポート
WINPROC( SetProc );
extern SETTING EdtMode;		// 設定
extern char szCaption[];	// タイトル

extern const char *ScrModeName[];
extern const char *PltMode[];
extern const char *AlgoName[];
extern const char *ErrAlgoName[];
extern const char *PreViewName[];
extern const char *ColorSeido[];
extern const char *ErrAlgo2Name[];
extern const char *PalEnName[];

#endif	// ndef _SETTING_H_
