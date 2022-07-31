// -----------------------------------------------------
//	TITLE		:	Windows Utility
//	FILE		:	winuty.cpp
//	COPYRIGHT	:	(C)2001 t.hara
//	COMENT		:	Windows �⏕�֐��Q
//	HISTORY		:	2001/02/11	v0.00	����J�n
// -----------------------------------------------------

// =====================================================
//	�C���N���[�h

#include "winuty.h"
#include <windowsx.h>

// =====================================================
//	�ݒ�

#define	WU_DEF_FILTER	"All files(*.*)\0*.*\0"

// =====================================================
//	�ϐ�

static HINSTANCE	h_ins = NULL;								//	�A�v���P�[�V�����C���X�^���X�n���h��

// =====================================================
//	�֐���`

// =====================================================
//	�A�v���P�[�V�����W��

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�C���X�^���X�n���h����o�^����
//	2.	����
//		h_ins	...	(I)	�C���X�^���X�n���h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_set_instance( HINSTANCE h_ins )
{
	::h_ins = h_ins;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�C���X�^���X�n���h�����擾����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�o�^�����C���X�^���X�n���h��
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
HINSTANCE wu_get_instance( void )
{
	return ::h_ins;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���b�Z�[�W����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		true	...	����
//		false	...	�A�v���P�[�V�����I���v��
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
bool wu_do_events( HWND h_wnd, HACCEL h_accel )
{
	MSG	msg;

	//	���b�Z�[�W��������� CPU ��������đҋ@����
	if( !PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ){
		WaitMessage();
		return true;
	}

	//	���b�Z�[�W�� WM_QUIT �Ȃ�I���v����Ԃ�
	if( !GetMessage( &msg, NULL, 0, 0 ) ) return false;

	//	�A�N�Z�����[�^���w�肳��Ă���Ώ�������
	if( h_wnd != NULL && h_accel != NULL ) {
		if( TranslateAccelerator( h_wnd, h_accel, &msg ) ) return true;
	}

	//	�g�����X���[�g�ƃf�B�X�p�b�`
	TranslateMessage( &msg );
	DispatchMessage( &msg );

	return true;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		��ʓI�ȃE�B���h�E�N���X�̐ݒ�
//	2.	����
//		p_wndclass	...	(O)	�E�B���h�E�N���X�\����
//		p_class		...	(I)	�E�B���h�E�N���X��
//		n_icon		...	(I)	�A�C�R���h�c
//		p_wndclass	...	(I)	�E�B���h�E�֐�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		���̊֐����g���O�� wu_set_instance �ŃC���X�^���X�n���h��
//		���w�肵�Ă����˂΂Ȃ�Ȃ�
// -----------------------------------------------------
void wu_get_wndclassex( WNDCLASSEX* p_wndclass, const char* p_class, int n_icon, WNDPROC p_wndproc )
{
	WNDCLASSEX*	&wc = p_wndclass;

	memset( wc, 0, sizeof( WNDCLASSEX ) );
	wc->cbSize			= sizeof( WNDCLASSEX );
	wc->hbrBackground	= ( HBRUSH ) GetStockObject( WHITE_BRUSH );
	wc->hCursor			= LoadCursor( NULL, IDC_ARROW );
	if( n_icon != NULL ) {
		wc->hIcon		= LoadIcon( h_ins, MAKEINTRESOURCE( n_icon ) );
		wc->hIconSm		= ( HICON ) LoadImage( h_ins, MAKEINTRESOURCE( n_icon ), 
							IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	} else {
		wc->hIcon		= LoadIcon( NULL, IDI_APPLICATION );
		wc->hIconSm		= LoadIcon( NULL, IDI_APPLICATION );
	}
	wc->hInstance		= h_ins;
	wc->lpfnWndProc		= p_wndproc;
	wc->lpszClassName	= p_class;
	wc->lpszMenuName	= NULL;
	wc->style			= 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�ݒ�
//	2.	����
//		*p_create		...	(O)	�E�B���h�E�����\����
//		*p_class		...	(I)	�E�B���h�E�N���X��
//		*p_caption		...	(I)	�E�B���h�E�^�C�g��
//		n_width			...	(I)	�E�B���h�E��
//									0 ... �ȗ��ƌ��Ȃ��f�t�H���g�l��ݒ肷��
//		n_height		...	(I)	�E�B���h�E����
//									0 ... �ȗ��ƌ��Ȃ��f�t�H���g�l��ݒ肷��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�E�B���h�E�ʒu�͉�ʂ̒����t�߂ɐݒ肳���
// -----------------------------------------------------
void wu_get_createstruct( CREATESTRUCT* p_create,const char* p_class, const char* p_caption, int n_width, int n_height )
{
	RECT			r_size;
	CREATESTRUCT*	&c	= p_create;

	memset( p_create, 0, sizeof( CREATESTRUCT ) );

	c->lpszClass		= p_class;
	c->lpszName			= p_caption;

	GetClientRect( GetDesktopWindow(), &r_size );

	if( n_width ){
		c->cx			= n_width;
	}else{
		c->cx			= 100;
	}

	if( n_height ){
		c->cy			= n_height;
	}else{
		c->cy			= 100;
	}

	c->x				= ( r_size.right  - c->cx ) / 2;
	c->y				= ( r_size.bottom - c->cy ) / 2;
	c->hMenu			= NULL;
	c->hwndParent		= NULL;
	c->dwExStyle		= 0;
	c->style			= WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX;
	c->hInstance		= h_ins;
	c->lpCreateParams	= NULL;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�̍쐬
//	2.	����
//		p_create	...	(I)	�E�B���h�E�����p�����[�^
//	3.	�Ԓl
//		NULL		...	�E�B���h�E�������s
//		��			...	���������E�B���h�E�̃E�B���h�E�n���h��
//	4.	���l
//		�E�B���h�E���������s����v���͎��̂Q���
//			(1)	�E�B���h�E�����p�����[�^�ɕs��������ꍇ
//			(2)	WM_CREATE �C�x���g�� -1 ���Ԃ��ꂽ�ꍇ
//		���̊֐��̌Ăяo���݂̂ł͂�������ʂł��Ȃ��B�֐��Ă�
//		�o���O�ɃO���[�o���ϐ��ɋK��l���i�[���AWM_CREATE �̏���
//		�擪�ɂĕʂ̋K��l���i�[����B���̃O���[�o���ϐ����Q�Ƃ�
//		�邱�Ƃŋ�ʂ��邱�Ƃ��ł���B
// -----------------------------------------------------
HWND wu_create_window( CREATESTRUCT* p_create )
{
	return( CreateWindowEx( p_create->dwExStyle, p_create->lpszClass, p_create->lpszName,
				p_create->style, p_create->x, p_create->y, p_create->cx, p_create->cy,
				p_create->hwndParent, p_create->hMenu, p_create->hInstance, p_create->lpCreateParams ) );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���s�t�@�C���̑��݂���f�B���N�g�����擾����
//	2.	����
//		p_buffer	...	(O)	�p�X���o�͂���o�b�t�@�̃A�h���X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�o�b�t�@�� MAX_PATH �̃T�C�Y�����m�ۂ��Ă����Ώ\��
// -----------------------------------------------------
void wu_get_exe_path( char* p_buffer )
{
	char		*p_path;

	p_path	= GetCommandLine();
	lstrcpy( p_buffer, p_path );
	p_path	= strrchr( p_buffer, '\\' );
	*p_path	= '\0';
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C�����J���_�C�A���O�\���i�P�ƃt�@�C���j
//	2.	����
//		h_wnd		...	(I)		�e�E�B���h�E�n���h��
//		p_name		...	(I/O)	�f�t�H���g�t�@�C��
//								�I�����ꂽ�t�@�C�����i�[�����B
//		p_filter	...	(I/O)	�g���q
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		p_name �� MAX_PATH �̃T�C�Y����K�v������
// -----------------------------------------------------
bool wu_dialog_select_open_file( HWND h_wnd, char* p_name, const char* p_filter )
{
	OPENFILENAME	ofn;

	memset( &ofn, 0, sizeof( ofn ) );
	ofn.hInstance	= h_ins;
	ofn.hwndOwner	= h_wnd;
	ofn.lStructSize	= sizeof( ofn );
	if( p_filter != NULL ) {
		ofn.lpstrFilter	= p_filter;
	} else {
		ofn.lpstrFilter	= WU_DEF_FILTER;
	}
	ofn.Flags		= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER
					| OFN_ENABLESIZING;
	ofn.lpstrFile	= p_name;
	ofn.nMaxFile	= MAX_PATH;
	ofn.nFilterIndex= 1;
	return( GetOpenFileName( &ofn ) != FALSE );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C�����J���_�C�A���O�\���i�����t�@�C���j
//	2.	����
//		h_wnd		...	(I)		�e�E�B���h�E�n���h��
//		p_name		...	(I/O)	�f�t�H���g�t�@�C��
//								�I�����ꂽ�t�@�C�����i�[�����B
//		p_filter	...	(I/O)	�g���q
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		p_name �� MAX_PATH �̃T�C�Y����K�v������
// -----------------------------------------------------
bool wu_dialog_multi_select_open_file( HWND h_wnd, char* p_name, const char* p_filter )
{
	OPENFILENAME	ofn;

	memset( &ofn, 0, sizeof( ofn ) );
	ofn.hInstance	= h_ins;
	ofn.hwndOwner	= h_wnd;
	ofn.lStructSize	= sizeof( ofn );
	if( p_filter != NULL ) {
		ofn.lpstrFilter	= p_filter;
	} else {
		ofn.lpstrFilter	= WU_DEF_FILTER;
	}
	ofn.Flags		= OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER
					| OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
	ofn.lpstrFile	= p_name;
	ofn.nMaxFile	= MAX_PATH;
	ofn.nFilterIndex= 1;
	return( GetOpenFileName( &ofn ) != FALSE );
}

// =====================================================
//	�E�B���h�E�ėp����֐�

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�N���C�A���g�̈�̃T�C�Y�w��
//	2.	����
//		h_wnd		...	(I)	�E�B���h�E�n���h��
//		n_width		...	(I)	��
//		n_height	...	(I)	����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		����̎��s��A�ݒ�s�\�Ȃ炠����߂�
// -----------------------------------------------------
void wu_set_client_rect( HWND h_wnd, int n_width, int n_height )
{
	RECT	r_size, r_winsize;
	int		i;

	for( i = 0; i < 3; ++i ) {
		//	�T�C�Y���擾
		GetClientRect( h_wnd, &r_size );
		GetWindowRect( h_wnd, &r_winsize );

		//	�T�C�Y�̔�r
		if( r_size.right == n_width && r_size.bottom == n_height ) break;

		//	�T�C�Y�̒���
		r_size.right	= r_winsize.right  + ( n_width  - r_size.right  ) - r_winsize.left;
		r_size.bottom	= r_winsize.bottom + ( n_height - r_size.bottom ) - r_winsize.top;

		//	�E�B���h�E�T�C�Y�̕ύX
		SetWindowPos( h_wnd, 0, 0, 0, r_size.right, r_size.bottom,
			SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER );
	}
}

// -----------------------------------------------------
//	1. �֐���
//		�E�B���h�E �� �w��̃E�B���h�E �̒����ֈړ�����
//	2. ����
//		h_wnd		...	(I)	�Ώۂ̃E�B���h�E�n���h��
//		h_basewnd	...	(I)	��̃E�B���h�E�n���h��
//	3. �Ԓl
//		�Ȃ�
//	4. ���l
//		��ƑΏۂ̃E�B���h�E�̐e�q�֌W�Ȃǂ͖��Ȃ�
//		��� NULL �ɂ���ƃf�X�N�g�b�v����ɂ���
// -----------------------------------------------------
void wu_move_center( HWND h_wnd, HWND h_basewnd )
{
	RECT	r_wsize, r_bsize;
	HWND	h_pwnd;
	POINT	p;

	// ��E�B���h�E�����w��̏ꍇ�̓f�X�N�g�b�v����ɂ���
	if( h_basewnd == NULL ) h_basewnd = GetDesktopWindow();

	// �E�B���h�E�̈ʒu�ƃT�C�Y�����߂�
	memset( &r_wsize, 0, sizeof( RECT ) );
	memset( &r_bsize, 0, sizeof( RECT ) );
	GetWindowRect( h_wnd,		&r_wsize );
	GetWindowRect( h_basewnd,	&r_bsize );

	// �����ʒu���Z�o����(�X�N���[�����W)
	p.x = ( ( r_bsize.left + r_bsize.right  ) - ( r_wsize.right  - r_wsize.left ) ) / 2;	
	p.y = ( ( r_bsize.top  + r_bsize.bottom ) - ( r_wsize.bottom - r_wsize.top  ) ) / 2;
	
	// �E�B���h�E���ړ�����
	h_pwnd = GetParent( h_wnd );
	
	if( h_pwnd != NULL && (GetWindowLong( h_wnd, GWL_STYLE ) & WS_CHILD) ){

		// �e�������Ă���ꍇ�͐e�E�B���h�E���W�n�ɕϊ�����
		ScreenToClient( h_pwnd, &p );
	}

	SetWindowPos( h_wnd, 0, p.x, p.y, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE );
}

// =====================================================
//	���X�g�r���[�֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		��̐����擾
//	2.	����
//		h_wnd		...	(I)	�e�E�B���h�E�n���h��
//		n_id		...	(I)	���X�g�r���[�̂h�c
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int wu_listview_get_column_count( HWND h_wnd, int n_id )
{
	HWND		h_list;
	LVCOLUMN	lvcolumn;
	int			n_cnt;

	h_list			= GetDlgItem( h_wnd, n_id );
	n_cnt			= 0;
	memset( &lvcolumn, 0, sizeof( lvcolumn ) );
	lvcolumn.mask	= LVCF_WIDTH;
	while( ListView_GetColumn( h_list, n_cnt, &lvcolumn ) ) ++n_cnt;
	return n_cnt;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���ǉ�����
//	2.	����
//		h_wnd		...	(I)	�e�E�B���h�E�n���h��
//		n_id		...	(I)	���X�g�r���[�̂h�c
//		s_name		...	(I)	��
//		n_width		...	(I)	��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_listview_add_column( HWND h_wnd, int n_id, const char* s_name, int n_width )
{
	HWND		h_list;
	LVCOLUMN	lvcolumn;
	int			n_cnt;

	h_list = GetDlgItem( h_wnd, n_id );

	//	�񐔂��擾����i�擪���ۂ����肷�邽�߁j
	n_cnt = wu_listview_get_column_count( h_wnd, n_id );

	//	�ǉ�����
	memset( &lvcolumn, 0, sizeof( lvcolumn ) );
	lvcolumn.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	if( n_cnt ) lvcolumn.mask |= LVCF_SUBITEM;
	lvcolumn.fmt		= LVCFMT_LEFT;
	lvcolumn.pszText	= (char*)s_name;
	lvcolumn.cx			= n_width;
	lvcolumn.iSubItem	= n_cnt;
	ListView_InsertColumn( h_list, n_cnt, &lvcolumn );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���ڐ����擾
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E
//		n_id	...	(I)	���X�g�r���[�̂h�c
//	3.	�Ԓl
//		���ڐ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int wu_listview_get_count( HWND h_wnd, int n_id )
{
	HWND		h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	return ListView_GetItemCount( h_list );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�V�������ڂ̒ǉ�
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E
//		n_id	...	(I)	���X�g�r���[�̂h�c
//		s_name	...	(I)	�ǉ����鍀�ږ�
//	3.	�Ԓl
//		�}���������ڂ̃C���f�b�N�X�i�s�ԍ��j
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int wu_listview_add_item( HWND h_wnd, int n_id, const char* s_name )
{
	LV_ITEM		lvitem;
	HWND		h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	memset( &lvitem, 0, sizeof( lvitem ) );

	lvitem.mask		= LVIF_TEXT;
	lvitem.pszText	= (char*)s_name;
	lvitem.iItem	= wu_listview_get_count( h_wnd, n_id );
	lvitem.iSubItem	= 0;
	return ListView_InsertItem( h_list, &lvitem );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���ڂ̃T�u�A�C�e���̐ݒ�
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	���X�g�r���[�̂h�c
//		n_row	...	(I)	�T�u�A�C�e���̍s�ԍ�
//		n_col	...	(I)	�T�u�A�C�e���̗�ԍ�
//		s_name	...	(I)	�T�u�A�C�e���ɐݒ肷�镶����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		n_col �� 0 �ɂ���ƍ��ږ����ύX�ł���
//		n_row �� -1 �ɂ���ƍŌ�̍s��ΏۂƂ���
// -----------------------------------------------------
void wu_listview_set_subitem( HWND h_wnd, int n_id, int n_row, int n_col,const char* s_name )
{
	LV_ITEM		lvitem;
	HWND		h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	if( n_row == -1 ) n_row = wu_listview_get_count( h_wnd, n_id )-1;

	memset( &lvitem, 0, sizeof( lvitem ) );

	lvitem.mask		= LVIF_TEXT;
	lvitem.pszText	= (char*)s_name;
	lvitem.iItem	= n_row;
	lvitem.iSubItem	= n_col;
	ListView_SetItem( h_list, &lvitem );
}

// =====================================================
//	�R���{�{�b�N�X�֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�R���{�{�b�N�X�ɕ����̕������ǉ�����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�R���{�{�b�N�X��ID
//		s_array	...	(I)	������|�C���^�̔z��A�h���X
//		n_count	...	(I)	s_array �̗v�f��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_combo_set_items( HWND h_wnd, int n_id, const char* s_array[], int n_count )
{
	int		i;
	HWND	h_combo;

	h_combo = GetDlgItem( h_wnd, n_id );

	for( i = 0; i < n_count; ++i ) {
		ComboBox_AddString( h_combo, s_array[ i ] );
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�R���{�{�b�N�X�̏�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		id		...	(I)	�R���{�{�b�N�X�̂h�c
//		sArray	...	(I)	������z��
//		n		...	(I)	������z��̗v�f��
//		index	...	(I)	�R���{�{�b�N�X�̏������C���f�b�N�X
// -------------------------------------------------------------
void ComboBox_Init( HWND hWnd, int id, const char **sArray, int n, int index ){
	HWND hCmb;
	int i;
	hCmb = GetDlgItem( hWnd, id );
	ComboBox_ResetContent( hCmb );
	for( i = 0; i < n; ++i ){
		ComboBox_AddString( hCmb, sArray[ i ] );
	}
	ComboBox_SetCurSel( hCmb, index );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�R���{�{�b�N�X�̑I���J�[�\���ʒu���w�肷��
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�R���{�{�b�N�X��ID
//		n_index	...	(I)	�I���ʒu
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_combo_set_index( HWND h_wnd, int n_id, int n_index )
{
	HWND	h_combo;

	h_combo = GetDlgItem( h_wnd, n_id );
	
	ComboBox_SetCurSel( h_combo, n_index );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�R���{�{�b�N�X�̑I���J�[�\���ʒu���擾����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�R���{�{�b�N�X��ID
//	3.	�Ԓl
//		�C���f�b�N�X�l
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int wu_combo_get_index( HWND h_wnd, int n_id )
{
	HWND	h_combo;

	h_combo = GetDlgItem( h_wnd, n_id );
	
	return ComboBox_GetCurSel( h_combo );
}

// =====================================================
//	�{�^���֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�{�^���ɃA�C�R�����Z�b�g����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�{�^����ID
//		hIcon	...	(I)	�Z�b�g����A�C�R��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_button_set_icon( HWND h_wnd, int n_id, HICON hIcon )
{
	HWND	h_button;

	h_button = GetDlgItem( h_wnd, n_id );
	
	SendMessage( h_button, BM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon ); 
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�{�^���̃`�F�b�N��Ԃ��Z�b�g����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�{�^����ID
//		b_check	...	(I)	�`�F�b�N���
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_button_set_check( HWND h_wnd, int n_id, bool b_check )
{
	HWND	h_button;

	h_button = GetDlgItem( h_wnd, n_id );
	
	SendMessage( h_button, BM_SETCHECK, b_check ? BST_CHECKED : BST_UNCHECKED, 0 ); 
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�{�^���̃`�F�b�N��Ԃ��擾����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�{�^����ID
//	3.	�Ԓl
//		true	...	�`�F�b�N�ς�
//		false	...	���`�F�b�N�A�O���C
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
bool wu_button_get_check( HWND h_wnd, int n_id )
{
	HWND	h_button;

	h_button = GetDlgItem( h_wnd, n_id );
	
	return( SendMessage( h_button, BM_GETCHECK, 0, 0 ) == BST_CHECKED ); 
}

// =====================================================
//	���j���[�֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�̃`�F�b�N��Ԃ��w�肷��
//	2.	����
//		h_wnd	...	(I)	���j���[���������E�B���h�E�n���h��
//		n_id	...	(I)	���j���[�A�C�e��ID
//		b_check	...	(I)	�`�F�b�N���
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_menu_set_check( HWND h_wnd, int n_id, BOOL b_check )
{
	HMENU	h_menu;

	//	���j���[�̎擾
	h_menu = GetMenu( h_wnd );
	if( h_menu == NULL ) return;

	//	�`�F�b�N��Ԃ̐ݒ�
	MENUITEMINFO	mi;

	memset( &mi, 0, sizeof( mi ) );
	mi.cbSize	= sizeof( mi );
	GetMenuItemInfo( h_menu, n_id, FALSE, &mi );
	mi.fMask	= MIIM_STATE;
	if( b_check ) {
		mi.fState	= MFS_CHECKED;
	} else {
		mi.fState	= MFS_UNCHECKED;
	}
	SetMenuItemInfo( h_menu, n_id, FALSE, &mi );

	SetMenu( h_wnd, h_menu );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���j���[�̗L��������ݒ肷��
//	2.	����
//		h_wnd	...	(I)	���j���[���������E�B���h�E�n���h��
//		n_id	...	(I)	���j���[�A�C�e��ID
//		b_enable	...	(I)	�L���������
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_menu_enabled( HWND h_wnd, int n_id, BOOL b_enable )
{
	HMENU	h_menu;

	//	���j���[�̎擾
	h_menu = GetMenu( h_wnd );
	if( h_menu == NULL ) return;

	//	�`�F�b�N��Ԃ̐ݒ�
	MENUITEMINFO	mi;

	memset( &mi, 0, sizeof( mi ) );
	mi.cbSize	= sizeof( mi );
	GetMenuItemInfo( h_menu, n_id, FALSE, &mi );
	mi.fMask	= MIIM_STATE;
	if( b_enable ) {
		mi.fState	= MFS_ENABLED;
	} else {
		mi.fState	= MFS_DISABLED | MFS_GRAYED;
	}
	SetMenuItemInfo( h_menu, n_id, FALSE, &mi );

	SetMenu( h_wnd, h_menu );
}

// =====================================================
//	���X�g�{�b�N�X�֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���X�g�{�b�N�X�ɕ����̕������ǉ�����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	���X�g�{�b�N�X��ID
//		s_array	...	(I)	������|�C���^�̔z��A�h���X
//		n_count	...	(I)	s_array �̗v�f��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_list_set_items( HWND h_wnd, int n_id, const char* s_array[], int n_count )
{
	int		i;
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	for( i = 0; i < n_count; ++i ) {
		ListBox_AddString( h_list, s_array[ i ] );
	}
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���X�g�{�b�N�X�ɕ������ǉ�����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	���X�g�{�b�N�X��ID
//		s_name	...	(I)	������
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_list_add_item( HWND h_wnd, int n_id, const char* s_name )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	ListBox_AddString( h_list, s_name );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���X�g�{�b�N�X����ɂ���
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	���X�g�{�b�N�X��ID
//		s_name	...	(I)	������
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_list_clear( HWND h_wnd, int n_id )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );

	ListBox_ResetContent( h_list );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���X�g�{�b�N�X�̑I���J�[�\���ʒu���w�肷��
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	���X�g�{�b�N�X��ID
//		n_index	...	(I)	�I���ʒu
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_list_set_index( HWND h_wnd, int n_id, int n_index )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );
	
	ListBox_SetCurSel( h_list, n_index );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���X�g�{�b�N�X�̑I���J�[�\���ʒu���擾����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	���X�g�{�b�N�X��ID
//	3.	�Ԓl
//		�C���f�b�N�X�l
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int wu_list_get_index( HWND h_wnd, int n_id )
{
	HWND	h_list;

	h_list = GetDlgItem( h_wnd, n_id );
	
	return ListBox_GetCurSel( h_list );
}

// =====================================================
//	�^�u�R���g���[���֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�^�u�R���g���[���̍��ڐ����擾����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�^�u�R���g���[����ID
//	3.	�Ԓl
//		���ڐ�
//	4.	���l
//		���s����� 0 ���Ԃ�
// -----------------------------------------------------
int wu_tab_get_count( HWND h_wnd, int n_id )
{
	HWND	h_tab;

	h_tab = GetDlgItem( h_wnd, n_id );

	return TabCtrl_GetItemCount( h_tab );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�^�u�R���g���[���ɍ��ڂ�ǉ�����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�^�u�R���g���[����ID
//		s_name	...	(I)	�ǉ�����^�u��
//	3.	�Ԓl
//		-1	...	���s
//		��	...	�V�����^�u�̃C���f�b�N�X
//	4.	���l
//		�o�^�ς݃^�u�̍Ō�ɒǉ�����
// -----------------------------------------------------
int wu_tab_add_item( HWND h_wnd, int n_id, const char* s_name )
{
	HWND	h_tab;
	int		n_index;
	TC_ITEM	tc;

	h_tab = GetDlgItem( h_wnd, n_id );
	memset( &tc, 0, sizeof( tc ) );
	tc.mask		= TCIF_TEXT;
	tc.pszText	= (LPSTR)s_name;
	n_index		= wu_tab_get_count( h_wnd, n_id );

	return TabCtrl_InsertItem( h_tab, n_index, &tc );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�^�u�R���g���[���ɕ����̕������ǉ�����
//	2.	����
//		h_wnd	...	(I)	�e�E�B���h�E�n���h��
//		n_id	...	(I)	�^�u�R���g���[����ID
//		s_array	...	(I)	������|�C���^�̔z��A�h���X
//		n_count	...	(I)	s_array �̗v�f��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_tab_set_items( HWND h_wnd, int n_id, const char* s_array[], int n_count )
{
	int		i;

	for( i = 0; i < n_count; ++i ) {
		wu_tab_add_item( h_wnd, n_id, s_array[ i ] );
	}
}

// =====================================================
//	�c�[���`�b�v

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�c�[���`�b�v���쐬����
//	2.	����
//		hWnd	...	(I)	�e�E�B���h�E�n���h��
//	3.	�Ԓl
//		!NULL	...	�c�[���`�b�v�̃E�B���h�E�n���h��
//		NULL	...	���s
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
HWND wu_ttip_create( HWND hWnd )
{
    HWND	hToolTip;

	hToolTip = CreateWindowEx(
				0,						//	�g���X�^�C��
				TOOLTIPS_CLASS,
				NULL,
				TTS_ALWAYSTIP,			//	�X�^�C��
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				hWnd,					// �e
				NULL,
				h_ins,
				NULL );

    SendMessage( hToolTip, TTM_ACTIVATE, (WPARAM)TRUE, 0);
	
	return hToolTip;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�c�[���`�b�v�ɕ\���Ώۂ�ǉ�
//	2.	����
//		hTip	...	�c�[���`�b�v�̃n���h��
//		nId		...	�R���g���[���̂h�c
//		s_tips	...	�\�����镶����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void wu_ttip_set_item( HWND hTip, int nId, char* s_tips )
{
    TOOLINFO	ti;
	RECT		rect;
	POINT		pt;
	HWND		hWnd = GetParent( hTip );

	GetWindowRect( GetDlgItem( hWnd, nId ), &rect );

	pt.x = rect.left;
	pt.y = rect.top;
	ScreenToClient( hWnd, &pt );
	ti.rect.left = pt.x;
	ti.rect.top  = pt.y;

	pt.x = rect.right;
	pt.y = rect.bottom;
	ScreenToClient( hWnd, &pt );
	ti.rect.right  = pt.x;
	ti.rect.bottom = pt.y;

	memset( &ti, 0, sizeof( ti ) );
	ti.cbSize	= sizeof( TOOLINFO );
    ti.hwnd		= hWnd;
    ti.hinst	= h_ins;
    ti.uFlags	= TTF_SUBCLASS | TTF_IDISHWND;
	ti.uId		= (WPARAM)GetDlgItem( hWnd, nId );
	ti.lpszText = (LPTSTR)s_tips;
	SendMessage( hTip, TTM_ADDTOOL, 0, (LPARAM)&ti );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//	�X���C�_�i�g���b�N�o�[�j�̍ő�l�ƍŏ��l��ݒ肷��
//	2.	����
//		max	...	(I)	�ő�l
//		min	...	(I)	�ŏ��l
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void SetSliderRange( HWND hWnd, short int max, short int min ){
	SendMessage( hWnd, TBM_SETRANGE, TRUE, MAKELONG( max, min ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�X���C�_�i�g���b�N�o�[�j�̈ʒu��ύX����
//	2.	����
//		val	...	(I)	�ʒu
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void SetSliderValue( HWND hWnd, short int val ){
	SendMessage( hWnd, TBM_SETPOS, TRUE, (LONG)val );
}
