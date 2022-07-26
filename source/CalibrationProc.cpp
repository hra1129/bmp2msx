// -------------------------------------------------------------
// BMP to MSX
// �L�����u���[�V�����_�C�A���O
//											(C)2022/7/20 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include "Config.h"
#include "Setting.h"
#include "Resource.h"
#include <commctrl.h>
#include "Convert.h"
#include "utils.h"
#include "winuty.h"
#include "bsave.h"
#include "fileuty.h"

// -------------------------------------------------------------
// �e�[�u��
// -------------------------------------------------------------
// �o�͉�ʃ��[�h�R���{�{�b�N�X�̓��e
static const char *p_screen_mode_name[]={
	"RGB 512 Colors palette",
	"SCREEN8",
	"SCREEN12",
};

// --------------------------------------------------------------------
// �v�f�̖��O
static const char *p_element_pal512[] = {
	"RED 0", "RED 1", "RED 2", "RED 3", "RED 4", "RED 5", "RED 6", "RED 7",
	"GREEN 0", "GREEN 1", "GREEN 2", "GREEN 3", "GREEN 4", "GREEN 5", "GREEN 6", "GREEN 7",
	"BLUE 0", "BLUE 1", "BLUE 2", "BLUE 3", "BLUE 4", "BLUE 5", "BLUE 6", "BLUE 7",
};

// --------------------------------------------------------------------
static const char *p_element_screen8[] = {
	"RED 0", "RED 1", "RED 2", "RED 3", "RED 4", "RED 5", "RED 6", "RED 7",
	"GREEN 0", "GREEN 1", "GREEN 2", "GREEN 3", "GREEN 4", "GREEN 5", "GREEN 6", "GREEN 7",
	"BLUE 0", "BLUE 1", "BLUE 2", "BLUE 3",
};

// --------------------------------------------------------------------
static const char *p_element_screen12[] = {
	"RED 0", "RED 1", "RED 2", "RED 3", "RED 4", "RED 5", "RED 6", "RED 7",
	"RED 8", "RED 9", "RED 10", "RED 11", "RED 12", "RED 13", "RED 14", "RED 15",
	"RED 16", "RED 17", "RED 18", "RED 19", "RED 20", "RED 21", "RED 22", "RED 23",
	"RED 24", "RED 25", "RED 26", "RED 27", "RED 28", "RED 29", "RED 30", "RED 31",
	"GREEN 0", "GREEN 1", "GREEN 2", "GREEN 3", "GREEN 4", "GREEN 5", "GREEN 6", "GREEN 7",
	"GREEN 8", "GREEN 9", "GREEN 10", "GREEN 11", "GREEN 12", "GREEN 13", "GREEN 14", "GREEN 15",
	"GREEN 16", "GREEN 17", "GREEN 18", "GREEN 19", "GREEN 20", "GREEN 21", "GREEN 22", "GREEN 23",
	"GREEN 24", "GREEN 25", "GREEN 26", "GREEN 27", "GREEN 28", "GREEN 29", "GREEN 30", "GREEN 31",
	"BLUE 0", "BLUE 1", "BLUE 2", "BLUE 3", "BLUE 4", "BLUE 5", "BLUE 6", "BLUE 7",
	"BLUE 8", "BLUE 9", "BLUE 10", "BLUE 11", "BLUE 12", "BLUE 13", "BLUE 14", "BLUE 15",
	"BLUE 16", "BLUE 17", "BLUE 18", "BLUE 19", "BLUE 20", "BLUE 21", "BLUE 22", "BLUE 23",
	"BLUE 24", "BLUE 25", "BLUE 26", "BLUE 27", "BLUE 28", "BLUE 29", "BLUE 30", "BLUE 31",
};

// -------------------------------------------------------------
// �ϐ�
static HDC		hMemDC = NULL;				// �J���[�e�[�u���i�[�p�f�o�C�X
static HBITMAP	hMemBM = NULL;				// �J���[�e�[�u���i�[�p�r�b�g�}�b�v
static WNDPROC	ColorBarProc = NULL;		// �J���[�o�[�̃f�t�H���g�E�B���h�E�֐�
static int w;								// �J���[�o�[�̃T�C�Y
static int h;								// �J���[�o�[�̃T�C�Y

//	�������̊K�����
static int pal512[ 3 * 8 ];					//	0�`7: Red, 8�`15: Green, 16�`23: Blue
static int screen8[ 2 * 8 + 4 ];
static int screen12[ 3 * 32 ];

// -------------------------------------------------------------
// �v���g�^�C�v
EVENT( onCalibrationInitDlg		);
EVENT( onCalibrationClose		);
EVENT( onCalibrationCommand		);
EVENT( onCalHScroll				);

// -------------------------------------------------------------
// �C�x���g����

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( CalibrationProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onCalibrationClose	);
		ONEVENT( WM_COMMAND		, onCalibrationCommand	);
		ONEVENT( WM_INITDIALOG	, onCalibrationInitDlg	);
		ONEVENT( WM_HSCROLL		, onCalHScroll			);
	}
	return FALSE;
}

// -------------------------------------------------------------
// ��ʃC�x���g

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���W�ϊ�
//	2.	����
//		x ....	(I/O)	X���W
//		y ....	(I/O)	Y���W
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		256 x 192 �ȍ��W�n�� rc �̓����ɔz�u�������W�n�ɕϊ�����
// -------------------------------------------------------------
static void convert_xy( LONG &x, LONG &y ){

	x = (int)( (x * ( w * 0.8 ) / 256.) + (w * 0.1) );
	y = (int)( (y * ( h * 0.8 ) / 192.) + (h * 0.1) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�o�[�̕\�����e���X�V����
//	2.	����
//		hWnd	.....	(I)	�E�B���h�E�n���h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void update_color_bar_pal512( HWND hWnd, RECT &rc ){
	HBRUSH hBrush;
	int x, y, index;
	int r, g, b;
	RECT fill_rc;

	index = 0;
	for( y = 0; y < 32; y++ ){
		for( x = 0; x < 16; x++ ){
			fill_rc.left = x * 16;
			fill_rc.top = y * 6;
			fill_rc.right = fill_rc.left + 16;
			fill_rc.bottom = fill_rc.top + 6;
			convert_xy( fill_rc.left, fill_rc.top );
			convert_xy( fill_rc.right, fill_rc.bottom );
			r = pal512[ index & 7 ];
			g = pal512[ (( index >> 3 ) & 7) + 8 ];
			b = pal512[ (( index >> 6 ) & 7) + 16 ];
			hBrush = CreateSolidBrush( RGB( r, g, b ) );
			FillRect( hMemDC, &fill_rc, hBrush );
			DeleteObject( hBrush );
			index++;
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�o�[�̕\�����e���X�V����
//	2.	����
//		hWnd	.....	(I)	�E�B���h�E�n���h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void update_color_bar_screen8( HWND hWnd, RECT &rc ){
	HBRUSH hBrush;
	int x, y, index;
	int r, g, b;
	RECT fill_rc;

	index = 0;
	for( y = 0; y < 16; y++ ){
		for( x = 0; x < 16; x++ ){
			fill_rc.left = x * 16;
			fill_rc.top = y * 12;
			fill_rc.right = fill_rc.left + 16;
			fill_rc.bottom = fill_rc.top + 12;
			convert_xy( fill_rc.left, fill_rc.top );
			convert_xy( fill_rc.right, fill_rc.bottom );
			r = screen8[ index & 7 ];
			g = screen8[ (( index >> 3 ) & 7) + 8 ];
			b = screen8[ (( index >> 6 ) & 3) + 16 ];
			hBrush = CreateSolidBrush( RGB( r, g, b ) );
			FillRect( hMemDC, &fill_rc, hBrush );
			DeleteObject( hBrush );
			index++;
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�o�[�̕\�����e���X�V����
//	2.	����
//		hWnd	.....	(I)	�E�B���h�E�n���h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void update_color_bar_screen12( HWND hWnd, RECT &rc ){
	HBRUSH hBrush;
	int x, y;
	int r, g, b;
	RECT fill_rc;

	for( y = 0; y < 8; y++ ){
		for( x = 0; x < 32; x++ ){
			fill_rc.left = x * 8;
			fill_rc.top = y * 24;
			fill_rc.right = fill_rc.left + 8;
			fill_rc.bottom = fill_rc.top + 24;
			convert_xy( fill_rc.left, fill_rc.top );
			convert_xy( fill_rc.right, fill_rc.bottom );
			r = screen12[  x * ( y       & 1 )       ];
			g = screen12[ (x * ((y >> 1) & 1 )) + 32 ];
			b = screen12[ (x * ((y >> 2) & 1 )) + 64 ];
			hBrush = CreateSolidBrush( RGB( r, g, b ) );
			FillRect( hMemDC, &fill_rc, hBrush );
			DeleteObject( hBrush );
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�o�[�̕\�����e���X�V����
//	2.	����
//		hWnd	.....	(I)	�E�B���h�E�n���h��
//		index ...	(I) SCREEN MODE �̃C���f�b�N�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void update_color_bar( HWND hWnd, int index ){
	HWND hColorBar;
	RECT rc;
	HBRUSH hBrush;

	hColorBar = GetDlgItem( hWnd, IDC_COLORBAR );

	//	�܂��S�̂����œh��Ԃ�
	hBrush = CreateSolidBrush( RGB( 0, 0, 0 ) );
	GetClientRect( hColorBar, &rc );
	FillRect( hMemDC, &rc, hBrush );
	DeleteObject( hBrush );

	switch( index ){
	default:
	case 0:
		update_color_bar_pal512( hColorBar, rc );
		break;
	case 1:
		update_color_bar_screen8( hColorBar, rc );
		break;
	case 2:
		update_color_bar_screen12( hColorBar, rc );
		break;
	}
	InvalidateRect( hColorBar, NULL, FALSE );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�v�f�̃R���{�{�b�N�X�̓��e���X�V����
//	2.	����
//		hWnd	.....	(I)	�E�B���h�E�n���h��
//		index ...	(I) SCREEN MODE �̃C���f�b�N�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void update_cmb_element( HWND hWnd, int index ){

	switch( index ){
	default:
	case 0:
		ComboBox_Init( hWnd, IDC_CMB_ELEMENT, p_element_pal512, sizeof( p_element_pal512 ) / sizeof( p_element_pal512[ 0 ] ), 0 );
		break;
	case 1:
		ComboBox_Init( hWnd, IDC_CMB_ELEMENT, p_element_screen8, sizeof( p_element_screen8 ) / sizeof( p_element_screen8[ 0 ] ), 0 );
		break;
	case 2:
		ComboBox_Init( hWnd, IDC_CMB_ELEMENT, p_element_screen12, sizeof( p_element_screen12 ) / sizeof( p_element_screen12[ 0 ] ), 0 );
		break;
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�o�[�\���̓��e���X�V����
//	2.	����
//		hWnd	.....	(I)	�E�B���h�E�n���h��
//		index ...	(I) SCREEN MODE �̃C���f�b�N�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onColorBarPaint ){
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd, &ps );

	BitBlt( hDC, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY );

	EndPaint( hWnd, &ps );
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�̃T�u�N���X�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( SubColorBarProc ){
	switch( msg ){
		ONEVENT( WM_PAINT	, onColorBarPaint );
	}
	return CallWindowProc( ColorBarProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���݂̐ݒ�����[�h
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void load_palette_data( void ){

	memcpy( pal512 +  0, convert7to255_r, sizeof( convert7to255_r ) );
	memcpy( pal512 +  8, convert7to255_g, sizeof( convert7to255_g ) );
	memcpy( pal512 + 16, convert7to255_b, sizeof( convert7to255_b ) );

	memcpy( screen8 + 0, convert7to255_s8r, sizeof( convert7to255_s8r ) );
	memcpy( screen8 + 8, convert7to255_s8g, sizeof( convert7to255_s8g ) );
	memcpy( screen8 + 16, convert3to255_s8b, sizeof( convert3to255_s8b ) );

	memcpy( screen12 +  0, convert31to255_s12r, sizeof( convert31to255_s12r ) );
	memcpy( screen12 + 32, convert31to255_s12g, sizeof( convert31to255_s12g ) );
	memcpy( screen12 + 64, convert31to255_s12b, sizeof( convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����l�����[�h
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void load_initial_data( void ){

	memcpy( pal512 + 0, init_convert7to255_r, sizeof( init_convert7to255_r ) );
	memcpy( pal512 + 8, init_convert7to255_g, sizeof( init_convert7to255_g ) );
	memcpy( pal512 + 16, init_convert7to255_b, sizeof( init_convert7to255_b ) );

	memcpy( screen8 + 0, init_convert7to255_s8r, sizeof( init_convert7to255_s8r ) );
	memcpy( screen8 + 8, init_convert7to255_s8g, sizeof( init_convert7to255_s8g ) );
	memcpy( screen8 + 16, init_convert3to255_s8b, sizeof( init_convert3to255_s8b ) );

	memcpy( screen12 + 0, init_convert31to255_s12r, sizeof( init_convert31to255_s12r ) );
	memcpy( screen12 + 32, init_convert31to255_s12g, sizeof( init_convert31to255_s12g ) );
	memcpy( screen12 + 64, init_convert31to255_s12b, sizeof( init_convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���݂̐ݒ�����[�h
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void save_palette_data( void ){

	memcpy( convert7to255_r, pal512 + 0, sizeof( convert7to255_r ) );
	memcpy( convert7to255_g, pal512 + 8, sizeof( convert7to255_g ) );
	memcpy( convert7to255_b, pal512 + 16, sizeof( convert7to255_b ) );

	memcpy( convert7to255_s8r, screen8 + 0, sizeof( convert7to255_s8r ) );
	memcpy( convert7to255_s8g, screen8 + 8, sizeof( convert7to255_s8g ) );
	memcpy( convert3to255_s8b, screen8 + 16, sizeof( convert3to255_s8b ) );

	memcpy( convert31to255_s12r, screen12 + 0, sizeof( convert31to255_s12r ) );
	memcpy( convert31to255_s12g, screen12 + 32, sizeof( convert31to255_s12g ) );
	memcpy( convert31to255_s12b, screen12 + 64, sizeof( convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�l�����X���C�_�[�p�̒l�\���X�V
//	2.	����
//		hWnd .... (I) �E�B���h�E�n���h��
//		Value ... (I) �l
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void set_adjust_value_text( HWND hWnd, int Value ){
	char	szBuf[ 32 ];

	wsprintf( szBuf, "%d", Value );
	SetDlgItemText( hWnd, IDC_VALUE, szBuf );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�l�����X���C�_�[�p�̒l�\���X�V
//	2.	����
//		hWnd .... (I) �E�B���h�E�n���h��
//		Value ... (I) �l
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void update_adjust_value( HWND hWnd ){
	int mode, pos, index;

	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );
	index = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_ELEMENT ) );

	switch( mode ){
	default:
	case 0:
		pos = pal512[ index ];
		break;
	case 1:
		pos = screen8[ index ];
		break;
	case 2:
		pos = screen12[ index ];
		break;
	}
	set_adjust_value_text( hWnd, pos );
	SetSliderValue( GetDlgItem( hWnd, IDC_ADJUSTER ), pos );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���ɖ߂��{�^��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onBtnUndo ){
	int mode;

	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );
	load_palette_data();
	update_cmb_element( hWnd, mode );
	update_color_bar( hWnd, mode );
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����l�ɖ߂��{�^��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onBtnLoadDefault ){
	int mode;

	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );
	load_initial_data();
	update_cmb_element( hWnd, mode );
	update_color_bar( hWnd, mode );
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�_�C�A���O�̏�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCalibrationInitDlg ) {
	HDC		hDC;
	RECT	rc;

	//	�ݒ�l���R�s�[����
	load_palette_data();

	//	�J���[�p���b�g�̃T�u�N���X���ƃr�b�g�}�b�v�̍쐬
	HWND hColorBar = GetDlgItem( hWnd, IDC_COLORBAR );
	GetClientRect( hColorBar, &rc );
	w = rc.right;
	h = rc.bottom;

	hDC = GetDC( hWnd );
	hMemDC = CreateCompatibleDC( hDC );
	hMemBM = CreateCompatibleBitmap( hDC, w, h );
	if( hMemDC == NULL || hMemBM == NULL ) return TRUE;
	SelectBrush( hMemDC, GetStockBrush( NULL_BRUSH ) );
	SelectBitmap( hMemDC, hMemBM );

	ColorBarProc = (WNDPROC)GetWindowLong( hColorBar, GWL_WNDPROC );
	SetWindowLong( hColorBar, GWL_WNDPROC, (LONG)SubColorBarProc );

	SetSliderRange( GetDlgItem( hWnd, IDC_ADJUSTER ), 0, 255 );

	ComboBox_Init( hWnd, IDC_CMB_SCREEN_MODE, p_screen_mode_name, sizeof(p_screen_mode_name)/sizeof( p_screen_mode_name[0]), 0 );
	update_color_bar( hWnd, 0 );
	update_cmb_element( hWnd, 0 );
	update_adjust_value( hWnd );
	return TRUE;	// �����ōŏ��̃t�H�[�J�X�𓖂ĂĂ��炤
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E���� / �{�^���FOK
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCalibrationOk ){

	// �ݒ肵���K�������̗p����
	save_palette_data();

	// ���\�[�X�̉��
	if( hMemDC != NULL ) DeleteDC( hMemDC );
	if( hMemBM != NULL ) DeleteBitmap( hMemBM );
	hMemDC = NULL;
	hMemBM = NULL;
	EndDialog( hWnd, 0 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E���� / �{�^���F�L�����Z��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCalibrationClose ) {

	// ���\�[�X�̉��
	if( hMemDC != NULL ) DeleteDC( hMemDC );
	if( hMemBM != NULL ) DeleteBitmap( hMemBM );
	hMemDC = NULL;
	hMemBM = NULL;
	EndDialog( hWnd, -1 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		SCREEN MODE �R���{�{�b�N�X���I�����ꂽ
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCmbScreenMode ) {
	int mode;

	if( HIWORD( wp ) != CBN_SELCHANGE ) return TRUE;
	mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMB_SCREEN_MODE ) );

	update_color_bar( hWnd, mode );
	update_cmb_element( hWnd, mode );
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		SCREEN MODE �R���{�{�b�N�X���I�����ꂽ
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCmbElement ){

	if( HIWORD( wp ) != CBN_SELCHANGE ) return TRUE;
	update_adjust_value( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�l�����X���C�_�[
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCalHScroll ){
	HWND		hCol = (HWND)lp;

	if( GetDlgCtrlID( hCol ) != IDC_ADJUSTER ){
		return TRUE;
	}

	int pos = SendMessage( hCol, TBM_GETPOS, 0, 0 );
	set_adjust_value_text( hWnd, pos );

	int mode = wu_combo_get_index( hWnd, IDC_CMB_SCREEN_MODE );
	int index = wu_combo_get_index( hWnd, IDC_CMB_ELEMENT );
	switch( mode ){
	default:
	case 0:
		pal512[ index ] = pos;
		break;
	case 1:
		screen8[ index ] = pos;
		break;
	case 2:
		screen12[ index ] = pos;
		break;
	}
	update_color_bar( hWnd, mode );
	InvalidateRect( GetDlgItem( hWnd, IDC_COLORBAR ), NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onCalibrationCommand ) {
	switch( LOWORD( wp ) ) {
	ONEVENT( IDOK					, onCalibrationOk		);
	ONEVENT( IDCANCEL				, onCalibrationClose	);
	ONEVENT( IDC_CMB_SCREEN_MODE	, onCmbScreenMode		);
	ONEVENT( IDC_CMB_ELEMENT		, onCmbElement			);
	ONEVENT( IDC_BTN_LOAD_DEFAULT	, onBtnLoadDefault		);
	ONEVENT( IDC_BTN_UNDO			, onBtnUndo				);
	}
	return TRUE;
}
