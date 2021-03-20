// -------------------------------------------------------------
// BMP to MSX
// プラグインダイアログ
//										(C)2001/5/11 HRA!
// -------------------------------------------------------------

#ifndef	_PLUGPROC_H_
#define	_PLUGPROC_H_

#include <windows.h>

// -----------------------------------------------------
//	1.	日本語名
//		ダイアログの表示
//	2.	引数
//		hWnd	...	(I)	親ウィンドウ
//		pText	...	(O)	パスを格納するアドレス
//		nMax	...	(I)	pText のサイズ
//	3.	返値
//		-1	...	更新無し
//		0	...	更新あり
//	4.	備考
//		キャンセルが押されると pText の内容は変化しない
// -----------------------------------------------------
int piShow( HWND hWnd, char *pText, int nMax );

#endif

