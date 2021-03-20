// -------------------------------------------------------------
//	TITLE		:	File Access Utility
//	FILE		:	fileuty.cpp
//	COPYRIGHT	:	(C)2001 t.hara
//	COMENT		:	ファイルアクセス関数群
//					Ｃ標準のファイルアクセス関数と似たインターフェース
//					を採用した WindowsAPI ベースの関数群
//					Ｃ標準のファイルアクセスによる不都合を避け、かつ
//					Windows 以外への移植性を維持する目的
//	HISTORY		:	2001/02/07	v0.00	製作開始
//					2001/02/11	v1.00	基本機能の実装完了
//					2001/02/14	v1.01	メモリアクセス機能を追加
// -------------------------------------------------------------

// =============================================================
//	インクルード

#include "fileuty.h"				//	自身のヘッダ
#include "typeuty.h"				//	型ユーティリティ
#include <stdio.h>					//	vstringf
#include <stdarg.h>					//	va_start / va_end

// =============================================================
//	補助関数のプロトタイプ（非公開関数）

inline bool _fu_check_enable( FU_FILE* p_file );
void _fu_change_cr( char* s_buffer );
void _fu_change_crlf( char* s_buffer );

// =============================================================
//	関数定義

// -------------------------------------------------------------
//	1.	日本語名
//		ファイルを開く
//	2.	引数
//		s_filename	...	(I)	ファイル名
//		s_option	...	(I)	アクセス指定
//								"rt"	...	テキスト読み込みアクセス
//								"wt"	...	テキスト読み書きアクセス
//								"at"	...	テキスト追加書き込みアクセス
//								"rb"	...	バイナリ読み込みアクセス
//								"wb"	...	バイナリ読み書きアクセス
//								"ab"	...	バイナリ追加書き込みアクセス
//							文字の順番・長さは無関係。
//							後にある方の文字が有効。
//							デフォルトは "rt"
//	3.	返値
//		NULL	...	失敗
//		その他	...	成功（ファイル構造のアドレス）
//	4.	備考
//		s_option に NULL を指定すると "rb" を指定したことになる
// -------------------------------------------------------------
FU_FILE* fu_open( const char* s_filename, const char* s_option )
{
	const char*	p_option;
	FU_FILE*	p_file = NULL;
	dword		n_option;
	bool		b_append;
	dword		n_access;

	//	ハンドルを取得する
	p_file = ( FU_FILE* ) LocalAlloc( LPTR, sizeof( FU_FILE ) );
	if( p_file == NULL ) goto file_exit;

	p_file->n_mode = FU_MODE_FILE;

	//	オプションを解釈する
	if( s_option == NULL ) {
		p_option = "rb";
	} else {
		p_option = s_option;
	}

	p_file->b_binary	= false;								//	デフォルトはテキスト
	n_option			= GENERIC_READ;							//	デフォルトはリード
	b_append			= false;								//	デフォルトは先頭からアクセス
	n_access			= OPEN_EXISTING;						//	デフォルトは存在するファイルにアクセス

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

	//	ファイルを開く
	p_file->h_file = CreateFile( s_filename, n_option, FILE_SHARE_READ, NULL, n_access,
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( p_file->h_file == INVALID_HANDLE_VALUE ) goto file_exit;

	//	正常な場合
	return p_file;

	//	エラーの場合
file_exit:
	if( p_file != NULL ) LocalFree( p_file );
	return NULL;
}

// -------------------------------------------------------------
//	1.	日本語名
//		連結メモリを開く
//	2.	引数
//		p_memory	...	(I)	リンク先メモリ
//		n_size		...	(I)	サイズ（バイト数）
//	3.	返値
//		NULL	...	失敗
//		その他	...	成功（ファイル構造のアドレス）
//	4.	備考
//		p_memory で示されるメモリにアクセス可能なファイル構造体
//		を返す。fu_close しても p_memory は解放しない。
//		成功した場合はアプリ終了前に fu_close を呼ばねばならない。
//		アクセスモードは、常にバイナリ、リードライト。
//		p_memory の不正はチェックしない。
// -------------------------------------------------------------
FU_FILE* fu_open_linkmemory( void* p_memory, dword n_size )
{
	FU_FILE*	p_file = NULL;

	//	ハンドルを取得する
	p_file = ( FU_FILE* ) LocalAlloc( LPTR, sizeof( FU_FILE ) );
	if( p_file == NULL ) goto file_exit;

	p_file->n_mode = FU_MODE_LINKMEMORY;

	//	オプションを解釈する
	p_file->b_binary	= true;									//	バイナリ

	//	連結
	p_file->n_size		= n_size;
	p_file->p_memory	= ( byte* ) p_memory;
	p_file->n_pointer	= 0;

	//	正常な場合
	return p_file;

	//	エラーの場合
file_exit:
	if( p_file != NULL ) LocalFree( p_file );
	return NULL;
}

// -------------------------------------------------------------
//	1.	日本語名
//		確保メモリを開く
//	2.	引数
//		n_size		...	(I)	サイズ（バイト数）
//	3.	返値
//		NULL	...	失敗
//		その他	...	成功（ファイル構造のアドレス）
//	4.	備考
//		fu_close すると確保したメモリを解放する。
//		成功した場合はアプリ終了前に fu_close を呼ばねばならない。
//		アクセスモードは、常にバイナリ、リードライト。
// -------------------------------------------------------------
FU_FILE* fu_open_allocmemory( dword n_size )
{
	FU_FILE*	p_file = NULL;

	//	ハンドルを取得する
	p_file = ( FU_FILE* ) LocalAlloc( LPTR, sizeof( FU_FILE ) );
	if( p_file == NULL ) goto file_exit;

	p_file->n_mode = FU_MODE_ALLOCMEMORY;

	//	オプションを解釈する
	p_file->b_binary	= true;									//	バイナリ

	//	連結
	p_file->n_size		= n_size;
	p_file->p_memory	= ( byte* ) LocalAlloc( LPTR, n_size );
	p_file->n_pointer	= 0;
	if( p_file->p_memory == NULL ) goto file_exit;

	//	正常な場合
	return p_file;

	//	エラーの場合
file_exit:
	if( p_file != NULL ) LocalFree( p_file );
	return NULL;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイルを閉じる
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//	3.	返値
//		なし
//	4.	備考
//		未初期化の p_file を指定してはならない
// -------------------------------------------------------------
void fu_close( FU_FILE* p_file )
{
	//	明らかに異常な場合はなにもしない
	if( !_fu_check_enable( p_file ) ) return;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		//	ファイルを閉じる
		CloseHandle( p_file->h_file );
		break;
	case FU_MODE_LINKMEMORY:
		//	なにもしない
		break;
	case FU_MODE_ALLOCMEMORY:
		//	メモリを解放する
		LocalFree( p_file->p_memory );
		break;
	}

	//	ファイル構造体を解放する
	LocalFree( p_file );
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイルに文字列を出力する
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//		s_format	...	(I)	出力する書式
//		...			...	(I)	引数群
//	3.	返値
//		なし
//	4.	備考
//		テキストモードの場合、\n を \r\n に変換して出力する
// -------------------------------------------------------------
void fu_printf( FU_FILE* p_file, const char* s_format, ... )
{
	va_list	mark;
	char	s_buffer[ FU_BUFFER_LENGTH ];

	//	引数リストの取得
	va_start( mark, s_format );

	//	明らかに異常な場合はなにもしない
	if( !_fu_check_enable( p_file ) ) goto printf_exit;

	//	文字列に構成
	vsprintf( s_buffer, s_format, mark );

	//	出力
	fu_write( p_file, s_buffer, lstrlen( s_buffer ) );

printf_exit:
	va_end( mark );
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイルの終端に達したかどうか調べる
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//	3.	返値
//		true	...	終端に達している
//		false	...	終端に達していない
//	4.	備考
//		なし
// -------------------------------------------------------------
bool fu_eof( FU_FILE* p_file )
{
	byte	s_buffer;

	//	１バイト読んでみる
	if( fu_read( p_file, &s_buffer, 1 ) == 0 ) return true;

	//	ファイルポインタを１バイト戻す
	fu_seek( p_file, -1, FILE_CURRENT );

	return false;
}

// -------------------------------------------------------------
//	1.	日本語名
//		１行入力する
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//		s_buffer	...	(O)	入力データの格納先アドレス
//		n_size		...	(I)	s_buffer のサイズ
//	3.	返値
//		s_buffer
//	4.	備考
//		いきなり失敗した場合は、s_buffer に "" を格納する
//		途中まで読み込んだ場合は、読み込んだところまでを s_buffer
//		に格納する
//		テキストモードの場合は、\r\n を \n に変換する
// -------------------------------------------------------------
char* fu_gets( FU_FILE* p_file, char* s_buffer, int n_size )
{
	char	s_template[ FU_BUFFER_LENGTH ];
	char*	p_template;
	int		i;

	//	クリア
	memset( s_template, 0, sizeof( s_template ) );

	//	明らかに異常な場合はなにもしない
	if( !_fu_check_enable( p_file ) ) goto gets_exit;

	//	１行入力する
	p_template = s_template;
	for( i = 0; i < FU_BUFFER_LENGTH-1; ++i ) {
		//	１バイト入力
		if( !fu_read( p_file, p_template, 1 ) ) break;
		//	改行なら抜ける
		if( *p_template == '\n' ) break;
		//	次
		++p_template;
	}

gets_exit:
	//	コピー
	lstrcpy( s_buffer, s_template );

	//	テキストモードならコードを変換する
	if( !p_file->b_binary ) _fu_change_crlf( s_buffer );

	return s_buffer;
}

// -------------------------------------------------------------
//	1.	日本語名
//		指定のバイト数だけ書き出す
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//		s_buffer	...	(I)	書き出すデータのアドレス
//		n_size		...	(I)	書き出すデータのサイズ byte
//	3.	返値
//		書き出したバイト数
//	4.	備考
//		なし
// -------------------------------------------------------------
int fu_write( FU_FILE* p_file, const void* s_buffer, int n_size )
{
	dword	n_template	= 0;
	byte*	p_buffer	= (byte*) s_buffer;

	//	明らかに異常な場合はなにもしない
	if( !_fu_check_enable( p_file ) ) goto write_exit;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		//	ファイルの書き出し
		if( !WriteFile( p_file->h_file, s_buffer, n_size, &n_template, NULL ) ) goto write_exit;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		//	メモリの書き出し
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
//	1.	日本語名
//		指定のバイト数だけ読み込む
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//		s_buffer	...	(O)	読み込み先アドレス
//		n_size		...	(I)	読み込むデータのサイズ byte
//	3.	返値
//		読み出したバイト数
//	4.	備考
//		なし
// -------------------------------------------------------------
int fu_read( FU_FILE* p_file, void* s_buffer, int n_size )
{
	dword	n_template	= 0;
	byte*	p_buffer	= (byte*) s_buffer;

	//	明らかに異常な場合はなにもしない
	if( !_fu_check_enable( p_file ) ) goto read_exit;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		//	ファイルの読み出し
		if( !ReadFile( p_file->h_file, s_buffer, n_size, &n_template, NULL ) ) goto read_exit;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		//	メモリの読み出し
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
//	1.	日本語名
//		ファイルシーク
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//		n_offset	...	(I)	ファイル位置
//		n_origin	...	(I)	シーク基準位置
//							FU_SEEK_TOP		...	ファイル先頭
//							FU_SEEK_CURRENT	...	現在位置
//							FU_SEEK_BOTTOM	...	ファイル終端
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool fu_seek( FU_FILE* p_file, sint32 n_offset, int n_origin )
{
	dword	n_template;

	//	明らかに異常な場合はなにもしない
	if( !_fu_check_enable( p_file ) ) return false;

	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		if( SetFilePointer( p_file->h_file, n_offset, NULL, n_origin ) == 0xFFFFFFFF ) return false;
		break;
	case FU_MODE_LINKMEMORY:
	case FU_MODE_ALLOCMEMORY:
		//	基準位置
		switch( n_origin ) {
		case FU_SEEK_TOP:		n_template = 0;						break;
		case FU_SEEK_CURRENT:	n_template = p_file->n_pointer;		break;
		case FU_SEEK_BOTTOM:	n_template = p_file->n_size - 1;	break;
		default:				n_template = p_file->n_pointer;
		}
		//	はみ出さないか判定
		n_template += n_offset;
		if( n_template < 0 || n_template >= p_file->n_size ) return false;
		p_file->n_pointer = n_template;
		break;
	}

	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		ファイルの長さを求める
//	2.	引数
//		p_file	...	(I)	ファイルハンドル
//	3.	返値
//		ファイルの長さ
//	4.	備考
//		４ＧＢ以内のファイルのみ
// -------------------------------------------------------------
unsigned long fu_length( FU_FILE* p_file ) {
	unsigned long	size;

	//	明らかに異常な場合はなにもしない
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
//	補助関数

// -------------------------------------------------------------
//	1.	日本語名
//		ハンドルが明らかに異常か調べる
//	2.	引数
//		p_file		...	(I)	ファイル構造のアドレス
//	3.	返値
//		true	...	正常の可能性が高い
//		false	...	異常
//	4.	備考
//		未初期化の構造体を指定した場合は、true を返す場合がある
// -------------------------------------------------------------
inline bool _fu_check_enable( FU_FILE* p_file )
{
	//	明らかに異常な場合はなにもしない
	if( p_file == NULL ) return false;
	if( p_file->h_file == NULL ) return false;

	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		\n を \r\n に変換する
//	2.	引数
//		s_buffer	...	(O)	変換元のデータを格納したアドレス
//							変換結果をここに格納する
//	3.	返値
//		なし
//	4.	備考
//		s_buffer は '\0' を終端として含まねばならない
//		変換後が FU_BUFFER_LENGTH に収まらなければならない
// -------------------------------------------------------------
void _fu_change_cr( char* s_buffer )
{
	char	s_template[ FU_BUFFER_LENGTH ];
	char	*p_buffer, *p_template;

	//	変換しながら s_template に格納する
	p_buffer	= s_buffer;
	p_template	= s_template;
	while( *p_buffer != '\0' ) {
		if( *p_buffer == '\n' ) {
			*p_template = '\r';
		}
		*p_template = *p_buffer;
		//	次の文字
		++p_template;
		++p_buffer;
	}
	*p_template = *p_buffer;
	lstrcpy( s_buffer, s_template );
}

// -------------------------------------------------------------
//	1.	日本語名
//		\r\n を \n に変換する
//	2.	引数
//		s_buffer	...	(O)	変換元のデータを格納したアドレス
//							変換結果をここに格納する
//	3.	返値
//		なし
//	4.	備考
//		s_buffer は '\0' を終端として含まねばならない
//		変換後が FU_BUFFER_LENGTH に収まらなければならない
// -------------------------------------------------------------
void _fu_change_crlf( char* s_buffer )
{
	char	s_template[ FU_BUFFER_LENGTH ];
	char	*p_buffer, *p_template;

	//	変換しながら s_template に格納する
	p_buffer	= s_buffer;
	p_template	= s_template;
	while( *p_buffer != '\0' ) {
		if( *p_buffer == '\r' ) {
			++p_template;
		}
		*p_template = *p_buffer;
		//	次の文字
		++p_template;
		++p_buffer;
	}
	*p_template = *p_buffer;
	lstrcpy( s_buffer, s_template );
}
