// -------------------------------------------------------------
//	TITLE		:	Data Type Utility
//	FILE		:	typeuty.cpp
//	COPYRIGHT	:	(C)2001 t.hara
//	COMENT		:	データ型処理マクロ群
//	HISTORY		:	2001/02/09	v0.00	製作開始
// -------------------------------------------------------------

#ifndef	_typeuty_h_
#define	_typeuty_h_

// =============================================================
//	インクルード

// =============================================================
//	型定義

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
//	データ処理マクロ

//	絶対値
template <class typ> inline typ tu_abs( typ a ){
	return( a < 0 ? -a : a );
}

//	符号
template <class typ> inline typ tu_sign( typ a ){
	if( a < 0 ) return -1; else if( a > 0 ) return 1;
	return 0;
}

//	大文字
inline char tu_to_upper( char c ){
	if( c >= 'a' && c <= 'z' ) return( c - 'a' + 'A' );
	return c;
}

//	小文字
inline char tu_to_lower( char c ){
	if( c >= 'A' && c <= 'Z' ) return( c - 'A' + 'a' );
	return c;
}

//	１０進数文字判定
inline bool tu_is_digit( char c ){
	return( c >= '0' && c <= '9' );
}

//	１６進数文字判定
inline bool tu_is_xdigit( char c ){
	return( tu_is_digit(c) || ( tu_to_upper(c) >= 'A' && tu_to_upper(c) <= 'F' ) );
}

//	ホワイトスペース判定
inline bool tu_is_space( char c ){
	return( c == ' ' || c == '\t' );
}

//	ホワイトスペースの次のアドレス
inline char* tu_skip_space( char* p ){
	while( tu_is_space( *p ) ) ++p;
	return p;
}

//	配列変数の要素数を求める（コンパイル時に要素数が確定する配列のみ）
#define	tu_elm_count( a )	( sizeof( a )/ sizeof( (a)[0] ) )
 
#endif	//	_typeuty_h_
