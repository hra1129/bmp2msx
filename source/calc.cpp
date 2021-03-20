// -------------------------------------------------------------
// 四則演算処理
//
//												(C)2000 HRA!
// -------------------------------------------------------------

#include "calc.h"
#include "string.h"
#include <ctype.h>

// -------------------------------------------------------------
// 設定

static const int calc_level_max = 50;		// カッコのネストレベル最大
#define WORD_SIZE	128						// 単語の最大サイズ

// -------------------------------------------------------------
// グローバル変数

typedef struct {
	char	*s_word;
	int		n_class;
} t_calc_words;

const t_calc_words calc_operator[]={
	{ "("	, calc_class_lpar	},
	{ ")"	, calc_class_rpar	},
	{ "+"	, calc_class_plus	},
	{ "-"	, calc_class_minus	},
	{ "*"	, calc_class_mul	},
	{ "/"	, calc_class_div	},
	{ "%"	, calc_class_mod	},
	{ "&"	, calc_class_and	},
	{ "|"	, calc_class_or		},
	{ "^"	, calc_class_xor	},
	{ "~"	, calc_class_not	},
	{ "MOD"	, calc_class_mod	},
	{ "AND"	, calc_class_and	},
	{ "OR"	, calc_class_or		},
	{ "XOR"	, calc_class_xor	},
	{ "NOT"	, calc_class_not	},
};

// -------------------------------------------------------------
// プロトタイプ宣言

static void calc_exp0( t_calc_param *p_calcpar );
static void calc_exp1( t_calc_param *p_calcpar );
static void calc_exp2( t_calc_param *p_calcpar );
static void calc_exp3( t_calc_param *p_calcpar );
static void calc_exp4( t_calc_param *p_calcpar );
static void calc_exp5( t_calc_param *p_calcpar );
static void calc_getword( t_calc_param *p_calcpar );
static bool calc_isexp( t_calc_param *p_calcpar );
static bool calc_isnumber( t_calc_param *p_calcpar );
static bool calc_ishexnumber( t_calc_param *p_calcpar );
static bool calc_isdecnumber( t_calc_param *p_calcpar );
static bool calc_isbinnumber( t_calc_param *p_calcpar );
static inline bool isbindigit( int c );
static inline void calc_skipspace( t_calc_param *p_calcpar );

// -------------------------------------------------------------
//	1.	日本語名
//		演算処理開始
//	2.	引数
//		p_calcpar	...	(I/O)	演算処理用構造体のポインタ（中身は
//								初期化されます）
//		s_line		...	(I)		演算処理をしたい文字列
//	3.	返値
//		true	...	成功
//		false	...	失敗
//	4.	備考
//		なし
// -------------------------------------------------------------
bool calc_exp( int *n_result,const char *s_line,int *ptr )
{
	t_calc_param p_calcpar;
	// 構造体の初期化
	p_calcpar.n_class		= calc_class_none;
	p_calcpar.s_line		= s_line;
	p_calcpar.n_ptr			= 0;
	p_calcpar.n_result		= 0;
	p_calcpar.b_lpar		= false;
	p_calcpar.n_level		= 0;
	p_calcpar.b_number		= true;
	// 演算処理開始
	try{
		calc_exp0( &p_calcpar );
	}catch( int err ){
		*n_result = err;
		return false;
	}
	*n_result = p_calcpar.n_result;
	*ptr = p_calcpar.n_ptr;
	return true;
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算処理
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void calc_exp0( t_calc_param *p_calcpar )
{
	// カッコネストレベルを増加
	++(p_calcpar->n_level);
	if( p_calcpar->n_level > calc_level_max ){
		throw( (int)calc_err_level );
		return;
	}
	// 項を取得する
	calc_exp1( p_calcpar );
	// 項を判定する
	if( p_calcpar->b_lpar ){
		if( p_calcpar->n_class == calc_class_rpar ){
			--(p_calcpar->n_level);
			return;
		}
	}else{
		if( p_calcpar->n_class == calc_class_none ){
			--(p_calcpar->n_level);
			return;
		}
	}
	// 不正な演算子または値が存在する
	throw( (int)calc_err_syntax );
	return;
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算子処理（レベル１）
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void calc_exp1( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// 項を取得する
	calc_exp2( p_calcpar );
	for(;;){
		// 取得した値
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// 演算子を評価する
		switch( p_calcpar->n_class ){
		case calc_class_or:
			calc_exp2( p_calcpar );
			p_calcpar->n_result |= n_value;
			break;
		case calc_class_xor:
			calc_exp2( p_calcpar );
			p_calcpar->n_result ^= n_value;
			break;
		default:
			return;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算子処理（レベル２）
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void calc_exp2( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// 項を取得する
	calc_exp3( p_calcpar );
	for(;;){
		// 取得した値
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// 演算子を評価する
		switch( p_calcpar->n_class ){
		case calc_class_and:
			calc_exp3( p_calcpar );
			p_calcpar->n_result &= n_value;
			break;
		default:
			return;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算子処理（レベル３）
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void calc_exp3( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// 項を取得する
	calc_exp4( p_calcpar );
	for(;;){
		// 取得した値
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// 演算子を評価する
		switch( n_class ){
		case calc_class_plus:
			calc_exp4( p_calcpar );
			p_calcpar->n_result += n_value;
			break;
		case calc_class_minus:
			calc_exp4( p_calcpar );
			p_calcpar->n_result = n_value - p_calcpar->n_result;
			break;
		default:
			return;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算子処理（レベル４）
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void calc_exp4( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// 項を取得する
	calc_exp5( p_calcpar );
	for(;;){
		// 取得した値
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// 演算子を評価する
		switch( n_class ){
		case calc_class_mul:
			calc_exp5( p_calcpar );
			p_calcpar->n_result *= n_value;
			break;
		case calc_class_div:
			calc_exp5( p_calcpar );
			if( p_calcpar->n_result == 0 ) throw( (int)calc_err_divided_by_zero );
			p_calcpar->n_result = n_value / p_calcpar->n_result;
			break;
		case calc_class_mod:
			calc_exp5( p_calcpar );
			if( p_calcpar->n_result == 0 ) throw( (int)calc_err_divided_by_zero );
			p_calcpar->n_result = n_value % p_calcpar->n_result;
			break;
		default:
			return;
		}
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算子処理（レベル５）
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static void calc_exp5( t_calc_param *p_calcpar )
{
	int n_class;
	bool b_lpar;
	// 単語を取得する
	calc_getword( p_calcpar );
	// 取得した値
	n_class = p_calcpar->n_class;
	// 演算子を評価する
	switch( n_class ){
	case calc_class_minus:
		p_calcpar->b_number = true;
		calc_exp5( p_calcpar );
		p_calcpar->n_result = -(p_calcpar->n_result);
		break;
	case calc_class_plus:
		p_calcpar->b_number = true;
		calc_exp5( p_calcpar );
		break;
	case calc_class_not:
		p_calcpar->b_number = true;
		calc_exp5( p_calcpar );
		p_calcpar->n_result = ~(p_calcpar->n_result);
		break;
	case calc_class_lpar:
		b_lpar = p_calcpar->b_lpar;
		p_calcpar->b_lpar = true;
		p_calcpar->b_number = true;
		calc_exp0( p_calcpar );
		p_calcpar->b_lpar = b_lpar;
		calc_getword( p_calcpar );
		if( p_calcpar->n_class == calc_class_const ){
			throw( (int)calc_err_syntax );
			return;
		}
		break;
	case calc_class_const:
		p_calcpar->b_number = false;
		calc_getword( p_calcpar );
		if( p_calcpar->n_class == calc_class_const ){
			throw( (int)calc_err_syntax );
			return;
		}
		break;
	case calc_class_none:
		if( p_calcpar->b_number ){
			throw( (int)calc_err_syntax );
			return;
		}
		break;
	}
}

// -------------------------------------------------------------
//	1.	日本語名
//		単語の取得
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		定数ならその値を n_result へ返す、演算子なら n_result は破壊しない
// -------------------------------------------------------------
static void calc_getword( t_calc_param *p_calcpar )
{
	// ホワイトスペースをスキップする
	calc_skipspace( p_calcpar );
	// 演算子かどうか判断する
	if( calc_isexp( p_calcpar ) ) return;
	// 定数かどうか判断する
	if( calc_isnumber( p_calcpar ) ) return;
	// それ以外
	if( p_calcpar->n_class!=calc_class_none &&
		p_calcpar->n_class!=calc_class_const &&
		p_calcpar->n_class!=calc_class_rpar )
		throw( (int)calc_err_syntax );
	p_calcpar->n_class = calc_class_none;
}

// -------------------------------------------------------------
//	1.	日本語名
//		演算子かどうか判断する
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		true	...	演算子
//		false	...	それ以外
//	4.	備考
//		演算子なら p_calcpar->n_class に演算子種類を書き込む
// -------------------------------------------------------------
static bool calc_isexp( t_calc_param *p_calcpar )
{
	int i,j,n_ptr,n_class,l;
	char *s_word;
	n_ptr = p_calcpar->n_ptr;
	for( i=0;i<calc_arraysize( calc_operator );++i ){
		s_word  = calc_operator[i].s_word;
		n_class = calc_operator[i].n_class;
		l = strlen( s_word );
		for( j=0;j<l;j++ ){
			if( (int)s_word[j] != toupper( p_calcpar->s_line[ n_ptr+j ] ) ){
				n_class = calc_class_none;
				break;
			}
		}
		if( n_class != calc_class_none ){
			p_calcpar->n_class = n_class;	// 一致した演算子
			p_calcpar->n_ptr += j;			// 演算子の次の位置
			return true;
		}
	}
	return false;
}

// -------------------------------------------------------------
//	1.	日本語名
//		定数かどうか判断する
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		true	...	定数
//		false	...	それ以外
//	4.	備考
//		定数なら p_calcpar に定数を読み込む
// -------------------------------------------------------------
static bool calc_isnumber( t_calc_param *p_calcpar )
{
	if( calc_ishexnumber( p_calcpar ) ) return true;	// 16進数
	if( calc_isbinnumber( p_calcpar ) ) return true;	//  2進数
	if( calc_isdecnumber( p_calcpar ) ) return true;	// 10進数
	return false;
}

// -------------------------------------------------------------
//	1.	日本語名
//		１６進数かどうか判定する
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		true	...	１６進数
//		false	...	それ以外
//	4.	備考
//		１６進数なら p_calcpar に定数を読み込む
// -------------------------------------------------------------
static bool calc_ishexnumber( t_calc_param *p_calcpar )
{
	char word[ WORD_SIZE ] = "";
	int pword = 0;
	int n_ptr;
	int n_result = 0;
	int n;
	const int n_base = 16;
	// 先頭は必ず数値
	n_ptr = p_calcpar->n_ptr;
	if( !isdigit( p_calcpar->s_line[ n_ptr ] ) ) return false;
	while( isxdigit( p_calcpar->s_line[ n_ptr ] ) && pword < WORD_SIZE-1 ){
		word[ pword++ ] = (char)toupper( p_calcpar->s_line[ n_ptr++ ] );
	}
	word[ pword ] = 0;
	// 16進数修飾子を認識する
	if( (char)toupper( p_calcpar->s_line[ n_ptr ] )!='H' ){
		return false;
	}
	++n_ptr;
	// 次の位置
	p_calcpar->n_ptr = n_ptr;
	// 数値に変換する
	n_ptr = 0;
	while( word[ n_ptr ]!='\0' ){
		if( isdigit( word[ n_ptr ] ) ){
			n = word[ n_ptr ] - '0';
		}else{
			n = word[ n_ptr ] - 'A' + 10;
		}
		n_result = n_result * n_base + n;
		++n_ptr;
	}
	p_calcpar->n_result = n_result;
	p_calcpar->n_class = calc_class_const;
	return true;

}

// -------------------------------------------------------------
//	1.	日本語名
//		１０進数かどうか判定する
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		true	...	１０進数
//		false	...	それ以外
//	4.	備考
//		１０進数なら p_calcpar に定数を読み込む
// -------------------------------------------------------------
static bool calc_isdecnumber( t_calc_param *p_calcpar )
{
	char word[ WORD_SIZE ] = "";
	int pword=0;
	int n_ptr;
	int n_result = 0;
	int n;
	const int n_base = 10;
	// 先頭は必ず数値
	n_ptr = p_calcpar->n_ptr;
	if( !isdigit( p_calcpar->s_line[ n_ptr ] ) ) return false;
	while( isdigit( p_calcpar->s_line[ n_ptr ] ) && pword < WORD_SIZE-1 ){
		word[ pword++ ] = (char)(p_calcpar->s_line[ n_ptr++ ]);
	}
	word[ pword ]=0;
	// 10進数修飾子を認識する（省略可能）
	if( (char)toupper( p_calcpar->s_line[ n_ptr ] )=='D' ){
		++n_ptr;
	}
	// 次の位置
	p_calcpar->n_ptr = n_ptr;
	// 数値に変換する
	n_ptr = 0;
	while( word[ n_ptr ]!='\0' ){
		n = word[ n_ptr ] - '0';
		n_result = n_result * n_base + n;
		++n_ptr;
	}
	p_calcpar->n_result = n_result;
	p_calcpar->n_class = calc_class_const;
	return true;

}

// -------------------------------------------------------------
//	1.	日本語名
//	２進数かどうか判定する
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		true	...	２進数
//		false	...	それ以外
//	4.	備考
//		定数なら p_calcpar に定数を読み込む
// -------------------------------------------------------------
static bool calc_isbinnumber( t_calc_param *p_calcpar )
{
	char word[ WORD_SIZE ] = "";
	int pword=0;
	int n_ptr;
	int n_result = 0;
	int n;
	const int n_base = 2;
	// 先頭は必ず 0 か 1
	n_ptr = p_calcpar->n_ptr;
	if( !isbindigit( p_calcpar->s_line[ n_ptr ] ) ) return false;
	while( isbindigit( p_calcpar->s_line[ n_ptr ] ) && pword < WORD_SIZE-1 ){
		word[ pword++ ] = (char)(p_calcpar->s_line[ n_ptr++ ]);
	}
	word[ pword ] = 0;
	// 2進数修飾子を認識する
	if( (char)toupper( p_calcpar->s_line[ n_ptr ] )!='B' ){
		return false;
	}
	++n_ptr;
	// 次の位置
	p_calcpar->n_ptr = n_ptr;
	// 数値に変換する
	n_ptr = 0;
	while( word[ n_ptr ]!='\0' ){
		n = word[ n_ptr ] - '0';
		n_result = n_result * n_base + n;
		++n_ptr;
	}
	p_calcpar->n_result = n_result;
	p_calcpar->n_class = calc_class_const;
	return true;

}

// -------------------------------------------------------------
//	1.	日本語名
//		２進数の桁かどうか判定する
//	2.	引数
//		c	...	(I)	桁を示す文字
//	3.	返値
//		true	...	２進数の桁
//		false	...	それ以外
//	4.	備考
//		なし
// -------------------------------------------------------------
static inline bool isbindigit( int c )
{
	return( c=='0' || c=='1' );
}

// -------------------------------------------------------------
//	1.	日本語名
//		ホワイトスペースをスキップする
//	2.	引数
//		p_calcpar	...	(I)	演算処理用構造体のポインタ
//	3.	返値
//		なし
//	4.	備考
//		なし
// -------------------------------------------------------------
static inline void calc_skipspace( t_calc_param *p_calcpar )
{
	while( isspace( p_calcpar->s_line[ p_calcpar->n_ptr ] ) ) ++(p_calcpar->n_ptr);
}
