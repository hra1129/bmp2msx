// -------------------------------------------------------------
//	TITLE		:	File Access Utility
//	FILE		:	fileuty.cpp
//	COPYRIGHT	:	(C)2001 t.hara
//	COMENT		:	�t�@�C���A�N�Z�X�֐��Q
//					�b�W���̃t�@�C���A�N�Z�X�֐��Ǝ����C���^�[�t�F�[�X
//					���̗p���� WindowsAPI �x�[�X�̊֐��Q
//					�b�W���̃t�@�C���A�N�Z�X�ɂ��s�s��������A����
//					Windows �ȊO�ւ̈ڐA�����ێ�����ړI
//	HISTORY		:	2001/02/07	v0.00	����J�n
//					2001/02/11	v1.00	��{�@�\�̎�������
//					2001/02/14	v1.01	�������A�N�Z�X�@�\��ǉ�
// -------------------------------------------------------------

// =============================================================
//	�C���N���[�h

#include "fileuty.h"				//	���g�̃w�b�_
#include "typeuty.h"				//	�^���[�e�B���e�B
#include <stdio.h>					//	vstringf
#include <stdarg.h>					//	va_start / va_end

// =============================================================
//	�⏕�֐��̃v���g�^�C�v�i����J�֐��j

inline bool _fu_check_enable( FU_FILE* p_file );
void _fu_change_cr( char* s_buffer );
void _fu_change_crlf( char* s_buffer );

// =============================================================
//	�֐���`

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C�����J��
//	2.	����
//		s_filename	...	(I)	�t�@�C����
//		s_option	...	(I)	�A�N�Z�X�w��
//								"rt"	...	�e�L�X�g�ǂݍ��݃A�N�Z�X
//								"wt"	...	�e�L�X�g�ǂݏ����A�N�Z�X
//								"at"	...	�e�L�X�g�ǉ��������݃A�N�Z�X
//								"rb"	...	�o�C�i���ǂݍ��݃A�N�Z�X
//								"wb"	...	�o�C�i���ǂݏ����A�N�Z�X
//								"ab"	...	�o�C�i���ǉ��������݃A�N�Z�X
//							�����̏��ԁE�����͖��֌W�B
//							��ɂ�����̕������L���B
//							�f�t�H���g�� "rt"
//	3.	�Ԓl
//		NULL	...	���s
//		���̑�	...	�����i�t�@�C���\���̃A�h���X�j
//	4.	���l
//		s_option �� NULL ���w�肷��� "rb" ���w�肵�����ƂɂȂ�
// -------------------------------------------------------------
FU_FILE* fu_open( const char* s_filename, const char* s_option )
{
	const char*	p_option;
	FU_FILE*	p_file = NULL;
	dword		n_option;
	bool		b_append;
	dword		n_access;

	//	�n���h�����擾����
	p_file = ( FU_FILE* ) LocalAlloc( LPTR, sizeof( FU_FILE ) );
	if( p_file == NULL ) goto file_exit;

	p_file->n_mode = FU_MODE_FILE;

	//	�I�v�V���������߂���
	if( s_option == NULL ) {
		p_option = "rb";
	} else {
		p_option = s_option;
	}

	p_file->b_binary	= false;								//	�f�t�H���g�̓e�L�X�g
	n_option			= GENERIC_READ;							//	�f�t�H���g�̓��[�h
	b_append			= false;								//	�f�t�H���g�͐擪����A�N�Z�X
	n_access			= OPEN_EXISTING;						//	�f�t�H���g�͑��݂���t�@�C���ɃA�N�Z�X

	while( *p_option != '\0' ) {
		switch( tu_to_upper( *p_option ) ) {
		case 'R':
			n_option = GENERIC_READ;
			n_access = OPEN_EXISTING;
			break;
		case 'W':
			n_option = GENERIC_WRITE;
			n_access = CREATE_ALWAYS;
			break;
		case 'T':
			p_file->b_binary = false;
			break;
		case 'B':
			p_file->b_binary = true;
			break;
		case 'A':
			b_append = true;
		}
		++p_option;
	}

	//	�t�@�C�����J��
	p_file->h_file = CreateFile( s_filename, n_option, FILE_SHARE_READ, NULL, n_access,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( p_file->h_file == INVALID_HANDLE_VALUE ) goto file_exit;

	//	����ȏꍇ
	return p_file;

	//	�G���[�̏ꍇ
file_exit:
	if( p_file != NULL ) LocalFree( p_file );
	return NULL;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�A�����������J��
//	2.	����
//		p_memory	...	(I)	�����N�惁����
//		n_size		...	(I)	�T�C�Y�i�o�C�g���j
//	3.	�Ԓl
//		NULL	...	���s
//		���̑�	...	�����i�t�@�C���\���̃A�h���X�j
//	4.	���l
//		p_memory �Ŏ�����郁�����ɃA�N�Z�X�\�ȃt�@�C���\����
//		��Ԃ��Bfu_close ���Ă� p_memory �͉�����Ȃ��B
//		���������ꍇ�̓A�v���I���O�� fu_close ���Ă΂˂΂Ȃ�Ȃ��B
//		�A�N�Z�X���[�h�́A��Ƀo�C�i���A���[�h���C�g�B
//		p_memory �̕s���̓`�F�b�N���Ȃ��B
// -------------------------------------------------------------
FU_FILE* fu_open_linkmemory( void* p_memory, dword n_size )
{
	FU_FILE*	p_file = NULL;

	//	�n���h�����擾����
	p_file = ( FU_FILE* ) LocalAlloc( LPTR, sizeof( FU_FILE ) );
	if( p_file == NULL ) goto file_exit;

	p_file->n_mode = FU_MODE_LINKMEMORY;

	//	�I�v�V���������߂���
	p_file->b_binary	= true;									//	�o�C�i��

	//	�A��
	p_file->n_size		= n_size;
	p_file->p_memory	= ( byte* ) p_memory;
	p_file->n_pointer	= 0;

	//	����ȏꍇ
	return p_file;

	//	�G���[�̏ꍇ
file_exit:
	if( p_file != NULL ) LocalFree( p_file );
	return NULL;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�m�ۃ��������J��
//	2.	����
//		n_size		...	(I)	�T�C�Y�i�o�C�g���j
//	3.	�Ԓl
//		NULL	...	���s
//		���̑�	...	�����i�t�@�C���\���̃A�h���X�j
//	4.	���l
//		fu_close ����Ɗm�ۂ������������������B
//		���������ꍇ�̓A�v���I���O�� fu_close ���Ă΂˂΂Ȃ�Ȃ��B
//		�A�N�Z�X���[�h�́A��Ƀo�C�i���A���[�h���C�g�B
// -------------------------------------------------------------
FU_FILE* fu_open_allocmemory( dword n_size )
{
	FU_FILE*	p_file = NULL;

	//	�n���h�����擾����
	p_file = ( FU_FILE* ) LocalAlloc( LPTR, sizeof( FU_FILE ) );
	if( p_file == NULL ) goto file_exit;

	p_file->n_mode = FU_MODE_ALLOCMEMORY;

	//	�I�v�V���������߂���
	p_file->b_binary	= true;									//	�o�C�i��

	//	�A��
	p_file->n_size		= n_size;
	p_file->p_memory	= ( byte* ) LocalAlloc( LPTR, n_size );
	p_file->n_pointer	= 0;
	if( p_file->p_memory == NULL ) goto file_exit;

	//	����ȏꍇ
	return p_file;

	//	�G���[�̏ꍇ
file_exit:
	if( p_file != NULL ) LocalFree( p_file );
	return NULL;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C�������
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		���������� p_file ���w�肵�Ă͂Ȃ�Ȃ�
// -------------------------------------------------------------
void fu_close( FU_FILE* p_file )
{
	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) return;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		//	�t�@�C�������
		CloseHandle( p_file->h_file );
		break;
	case FU_MODE_LINKMEMORY:
		//	�Ȃɂ����Ȃ�
		break;
	case FU_MODE_ALLOCMEMORY:
		//	���������������
		LocalFree( p_file->p_memory );
		break;
	}

	//	�t�@�C���\���̂��������
	LocalFree( p_file );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���ɕ�������o�͂���
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//		s_format	...	(I)	�o�͂��鏑��
//		...			...	(I)	�����Q
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�e�L�X�g���[�h�̏ꍇ�A\n �� \r\n �ɕϊ����ďo�͂���
// -------------------------------------------------------------
void fu_printf( FU_FILE* p_file, const char* s_format, ... )
{
	va_list	mark;
	char	s_buffer[ FU_BUFFER_LENGTH ];

	//	�������X�g�̎擾
	va_start( mark, s_format );

	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) goto printf_exit;

	//	������ɍ\��
	vsprintf( s_buffer, s_format, mark );

	//	�o��
	fu_write( p_file, s_buffer, lstrlen( s_buffer ) );

printf_exit:
	va_end( mark );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���̏I�[�ɒB�������ǂ������ׂ�
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//	3.	�Ԓl
//		true	...	�I�[�ɒB���Ă���
//		false	...	�I�[�ɒB���Ă��Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool fu_eof( FU_FILE* p_file )
{
	byte	s_buffer;

	//	�P�o�C�g�ǂ�ł݂�
	if( fu_read( p_file, &s_buffer, 1 ) == 0 ) return true;

	//	�t�@�C���|�C���^���P�o�C�g�߂�
	fu_seek( p_file, -1, FILE_CURRENT );

	return false;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�P�s���͂���
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//		s_buffer	...	(O)	���̓f�[�^�̊i�[��A�h���X
//		n_size		...	(I)	s_buffer �̃T�C�Y
//	3.	�Ԓl
//		s_buffer
//	4.	���l
//		�����Ȃ莸�s�����ꍇ�́As_buffer �� "" ���i�[����
//		�r���܂œǂݍ��񂾏ꍇ�́A�ǂݍ��񂾂Ƃ���܂ł� s_buffer
//		�Ɋi�[����
//		�e�L�X�g���[�h�̏ꍇ�́A\r\n �� \n �ɕϊ�����
// -------------------------------------------------------------
char* fu_gets( FU_FILE* p_file, char* s_buffer, int n_size )
{
	char	s_template[ FU_BUFFER_LENGTH ];
	char*	p_template;
	int		i;

	//	�N���A
	memset( s_template, 0, sizeof( s_template ) );

	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) goto gets_exit;

	//	�P�s���͂���
	p_template = s_template;
	for( i = 0; i < FU_BUFFER_LENGTH-1; ++i ) {
		//	�P�o�C�g����
		if( !fu_read( p_file, p_template, 1 ) ) break;
		//	���s�Ȃ甲����
		if( *p_template == '\n' ) break;
		//	��
		++p_template;
	}

gets_exit:
	//	�R�s�[
	lstrcpy( s_buffer, s_template );

	//	�e�L�X�g���[�h�Ȃ�R�[�h��ϊ�����
	if( !p_file->b_binary ) _fu_change_crlf( s_buffer );

	return s_buffer;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�w��̃o�C�g�����������o��
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//		s_buffer	...	(I)	�����o���f�[�^�̃A�h���X
//		n_size		...	(I)	�����o���f�[�^�̃T�C�Y byte
//	3.	�Ԓl
//		�����o�����o�C�g��
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
int fu_write( FU_FILE* p_file, const void* s_buffer, int n_size )
{
	dword	n_template	= 0;
	byte*	p_buffer	= (byte*) s_buffer;

	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) goto write_exit;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		//	�t�@�C���̏����o��
		if( !WriteFile( p_file->h_file, s_buffer, n_size, &n_template, NULL ) ) goto write_exit;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		//	�������̏����o��
		n_template = n_size;
		for( ; p_file->n_pointer < p_file->n_size && n_size; --p_file->n_pointer, --n_size ) {
			p_file->p_memory[ p_file->n_pointer ] = *p_buffer;
			++p_buffer;
		}
		n_template -= n_size;
		break;
	}

write_exit:
	return n_template;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�w��̃o�C�g�������ǂݍ���
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//		s_buffer	...	(O)	�ǂݍ��ݐ�A�h���X
//		n_size		...	(I)	�ǂݍ��ރf�[�^�̃T�C�Y byte
//	3.	�Ԓl
//		�ǂݏo�����o�C�g��
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
int fu_read( FU_FILE* p_file, void* s_buffer, int n_size )
{
	dword	n_template	= 0;
	byte*	p_buffer	= (byte*) s_buffer;

	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) goto read_exit;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		//	�t�@�C���̓ǂݏo��
		if( !ReadFile( p_file->h_file, s_buffer, n_size, &n_template, NULL ) ) goto read_exit;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		//	�������̓ǂݏo��
		n_template = n_size;
		for( ; p_file->n_pointer < p_file->n_size && n_size; --p_file->n_pointer, --n_size ) {
			*p_buffer = p_file->p_memory[ p_file->n_pointer ];
			++p_buffer;
		}
		n_template -= n_size;
		break;
	}

read_exit:
	return n_template;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���V�[�N
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//		n_offset	...	(I)	�t�@�C���ʒu
//		n_origin	...	(I)	�V�[�N��ʒu
//							FU_SEEK_TOP		...	�t�@�C���擪
//							FU_SEEK_CURRENT	...	���݈ʒu
//							FU_SEEK_BOTTOM	...	�t�@�C���I�[
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool fu_seek( FU_FILE* p_file, sint32 n_offset, int n_origin )
{
	dword	n_template;

	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) return false;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		if( SetFilePointer( p_file->h_file, n_offset, NULL, n_origin ) == 0xFFFFFFFF ) return false;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		//	��ʒu
		switch( n_origin ) {
		case FU_SEEK_TOP:		n_template = 0;						break;
		case FU_SEEK_CURRENT:	n_template = p_file->n_pointer;		break;
		case FU_SEEK_BOTTOM:	n_template = p_file->n_size - 1;	break;
		default:				n_template = p_file->n_pointer;
		}
		//	�͂ݏo���Ȃ�������
		n_template += n_offset;
		if( n_template < 0 || n_template >= p_file->n_size ) return false;
		p_file->n_pointer = n_template;
		break;
	}

	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���̒��������߂�
//	2.	����
//		p_file	...	(I)	�t�@�C���n���h��
//	3.	�Ԓl
//		�t�@�C���̒���
//	4.	���l
//		�S�f�a�ȓ��̃t�@�C���̂�
// -------------------------------------------------------------
unsigned long fu_length( FU_FILE* p_file ) {
	unsigned long	size;

	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( !_fu_check_enable( p_file ) ) return false;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		size = GetFileSize( p_file->h_file, NULL );
		if( size == 0xFFFFFFFF ) size = 0;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		size = p_file->n_size;
		break;
	}

	return size;
}

// =============================================================
//	�⏕�֐�

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�n���h�������炩�Ɉُ킩���ׂ�
//	2.	����
//		p_file		...	(I)	�t�@�C���\���̃A�h���X
//	3.	�Ԓl
//		true	...	����̉\��������
//		false	...	�ُ�
//	4.	���l
//		���������̍\���̂��w�肵���ꍇ�́Atrue ��Ԃ��ꍇ������
// -------------------------------------------------------------
inline bool _fu_check_enable( FU_FILE* p_file )
{
	//	���炩�Ɉُ�ȏꍇ�͂Ȃɂ����Ȃ�
	if( p_file == NULL ) return false;
	if( p_file->h_file == NULL ) return false;

	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		\n �� \r\n �ɕϊ�����
//	2.	����
//		s_buffer	...	(O)	�ϊ����̃f�[�^���i�[�����A�h���X
//							�ϊ����ʂ������Ɋi�[����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		s_buffer �� '\0' ���I�[�Ƃ��Ċ܂܂˂΂Ȃ�Ȃ�
//		�ϊ��オ FU_BUFFER_LENGTH �Ɏ��܂�Ȃ���΂Ȃ�Ȃ�
// -------------------------------------------------------------
void _fu_change_cr( char* s_buffer )
{
	char	s_template[ FU_BUFFER_LENGTH ];
	char	*p_buffer, *p_template;

	//	�ϊ����Ȃ��� s_template �Ɋi�[����
	p_buffer	= s_buffer;
	p_template	= s_template;
	while( *p_buffer != '\0' ) {
		if( *p_buffer == '\n' ) {
			*p_template = '\r';
		}
		*p_template = *p_buffer;
		//	���̕���
		++p_template;
		++p_buffer;
	}
	*p_template = *p_buffer;
	lstrcpy( s_buffer, s_template );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		\r\n �� \n �ɕϊ�����
//	2.	����
//		s_buffer	...	(O)	�ϊ����̃f�[�^���i�[�����A�h���X
//							�ϊ����ʂ������Ɋi�[����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		s_buffer �� '\0' ���I�[�Ƃ��Ċ܂܂˂΂Ȃ�Ȃ�
//		�ϊ��オ FU_BUFFER_LENGTH �Ɏ��܂�Ȃ���΂Ȃ�Ȃ�
// -------------------------------------------------------------
void _fu_change_crlf( char* s_buffer )
{
	char	s_template[ FU_BUFFER_LENGTH ];
	char	*p_buffer, *p_template;

	//	�ϊ����Ȃ��� s_template �Ɋi�[����
	p_buffer	= s_buffer;
	p_template	= s_template;
	while( *p_buffer != '\0' ) {
		if( *p_buffer == '\r' ) {
			++p_template;
		}
		*p_template = *p_buffer;
		//	���̕���
		++p_template;
		++p_buffer;
	}
	*p_template = *p_buffer;
	lstrcpy( s_buffer, s_template );
}
