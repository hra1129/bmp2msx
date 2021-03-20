// -------------------------------------------------------------
//	TITLE		:	Data Type Utility
//	FILE		:	typeuty.cpp
//	COPYRIGHT	:	(C)2001 t.hara
//	COMENT		:	�f�[�^�^�����}�N���Q
//	HISTORY		:	2001/02/09	v0.00	����J�n
// -------------------------------------------------------------

#ifndef	_typeuty_h_
#define	_typeuty_h_

// =============================================================
//	�C���N���[�h

// =============================================================
//	�^��`

typedef unsigned long	uint32;									// 32bit unsigned integer
typedef unsigned short	uint16;									// 32bit unsigned integer
typedef unsigned char	uint8;									//  8bit unsigned integer

typedef signed long		sint32;									// 32bit signed integer
typedef signed short	sint16;									// 16bit signed integer
typedef signed char		sint8;									//  8bit signed integer

typedef uint32			dword;									// double word
typedef uint16			word;									// word
typedef uint8			byte;									// byte

// =============================================================
//	�f�[�^�����}�N��

//	��Βl
template <class typ> inline typ tu_abs( typ a ){
	return( a < 0 ? -a : a );
}

//	����
template <class typ> inline typ tu_sign( typ a ){
	if( a < 0 ) return -1; else if( a > 0 ) return 1;
	return 0;
}

//	�啶��
inline char tu_to_upper( char c ){
	if( c >= 'a' && c <= 'z' ) return( c - 'a' + 'A' );
	return c;
}

//	������
inline char tu_to_lower( char c ){
	if( c >= 'A' && c <= 'Z' ) return( c - 'A' + 'a' );
	return c;
}

//	�P�O�i����������
inline bool tu_is_digit( char c ){
	return( c >= '0' && c <= '9' );
}

//	�P�U�i����������
inline bool tu_is_xdigit( char c ){
	return( tu_is_digit(c) || ( tu_to_upper(c) >= 'A' && tu_to_upper(c) <= 'F' ) );
}

//	�z���C�g�X�y�[�X����
inline bool tu_is_space( char c ){
	return( c == ' ' || c == '\t' );
}

//	�z���C�g�X�y�[�X�̎��̃A�h���X
inline char* tu_skip_space( char* p ){
	while( tu_is_space( *p ) ) ++p;
	return p;
}

//	�z��ϐ��̗v�f�������߂�i�R���p�C�����ɗv�f�����m�肷��z��̂݁j
#define	tu_elm_count( a )	( sizeof( a )/ sizeof( (a)[0] ) )
 
#endif	//	_typeuty_h_
