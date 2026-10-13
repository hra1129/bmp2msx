// -----------------------------------------------------
//	TITLE		:	Converter for Windows
//	FILE		:	winConvert.cpp
//	COPYRIGHT	:	(C)2022 t.hara
//	COMENT		:	Windows 用の変換補助艦数群
//	HISTORY		:	2022/08/01	v1.00	初版 Convert.cpp から分離
// -----------------------------------------------------

#include <Windows.h>
#include "Convert.h"

#ifndef __WIN_CONVERT_H__
#define __WIN_CONVERT_H__

void draw_screen( const unsigned char *bmp,HDC hDC,const SETTING *Mode );

#endif
