// -------------------------------------------------------------
// 四則演算処理
//
//												2000 Hara
// -------------------------------------------------------------

#ifndef _CALC_H_
#define _CALC_H_

// -------------------------------------------------------------
// 演算処理パラメータ構造体

typedef struct {
	const char	*s_line;		// 処理中の文字列
	int			n_ptr;			// s_line 中の現在の着目位置
	int			n_class;		// 取り出した単語の種類
	int			n_result;		// 計算結果
	bool		b_lpar;			// ( から始まる演算処理だったか否か
	int			n_level;		// 式の階層レベル（カッコのネストレベル）
	bool		b_number;		// 必ず値を返さなければならないか否か
} t_calc_param;

// -------------------------------------------------------------
// n_class の値

enum {
	calc_class_none,			// 未定義
	calc_class_lpar,			// '('
	calc_class_rpar,			// ')'
	calc_class_plus,			// '+'
	calc_class_minus,			// '-'
	calc_class_mul,				// '*'
	calc_class_div,				// '/'
	calc_class_mod,				// 'MOD'
	calc_class_and,				// 'AND'
	calc_class_or,				// 'OR'
	calc_class_xor,				// 'XOR'
	calc_class_not,				// 'NOT'
	calc_class_const,			// 定数
};

// -------------------------------------------------------------
// エラーコード

enum {
	calc_err_noerr,				// エラー無し
	calc_err_syntax,			// 文法エラー
	calc_err_level,				// 過剰なカッコのネスト
	calc_err_unknown_symbol,	// 評価できないシンボルが存在します
	calc_err_divided_by_zero,	// 0 除算エラー
};

#define calc_arraysize( a ) ( sizeof(a)/sizeof((a)[0]) )

// -------------------------------------------------------------
// 関数

bool calc_exp( int *n_result,const char *s_line,int *ptr );

#endif
