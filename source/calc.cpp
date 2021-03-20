// -------------------------------------------------------------
// �l�����Z����
//
//												(C)2000 HRA!
// -------------------------------------------------------------

#include "calc.h"
#include "string.h"
#include <ctype.h>

// -------------------------------------------------------------
// �ݒ�

static const int calc_level_max = 50;		// �J�b�R�̃l�X�g���x���ő�
#define WORD_SIZE	128						// �P��̍ő�T�C�Y

// -------------------------------------------------------------
// �O���[�o���ϐ�

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
// �v���g�^�C�v�錾

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
//	1.	���{�ꖼ
//		���Z�����J�n
//	2.	����
//		p_calcpar	...	(I/O)	���Z�����p�\���̂̃|�C���^�i���g��
//								����������܂��j
//		s_line		...	(I)		���Z������������������
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool calc_exp( int *n_result,const char *s_line,int *ptr )
{
	t_calc_param p_calcpar;
	// �\���̂̏�����
	p_calcpar.n_class		= calc_class_none;
	p_calcpar.s_line		= s_line;
	p_calcpar.n_ptr			= 0;
	p_calcpar.n_result		= 0;
	p_calcpar.b_lpar		= false;
	p_calcpar.n_level		= 0;
	p_calcpar.b_number		= true;
	// ���Z�����J�n
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
//	1.	���{�ꖼ
//		���Z����
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void calc_exp0( t_calc_param *p_calcpar )
{
	// �J�b�R�l�X�g���x���𑝉�
	++(p_calcpar->n_level);
	if( p_calcpar->n_level > calc_level_max ){
		throw( (int)calc_err_level );
		return;
	}
	// �����擾����
	calc_exp1( p_calcpar );
	// ���𔻒肷��
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
	// �s���ȉ��Z�q�܂��͒l�����݂���
	throw( (int)calc_err_syntax );
	return;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���Z�q�����i���x���P�j
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void calc_exp1( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// �����擾����
	calc_exp2( p_calcpar );
	for(;;){
		// �擾�����l
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// ���Z�q��]������
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
//	1.	���{�ꖼ
//		���Z�q�����i���x���Q�j
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void calc_exp2( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// �����擾����
	calc_exp3( p_calcpar );
	for(;;){
		// �擾�����l
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// ���Z�q��]������
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
//	1.	���{�ꖼ
//		���Z�q�����i���x���R�j
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void calc_exp3( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// �����擾����
	calc_exp4( p_calcpar );
	for(;;){
		// �擾�����l
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// ���Z�q��]������
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
//	1.	���{�ꖼ
//		���Z�q�����i���x���S�j
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void calc_exp4( t_calc_param *p_calcpar )
{
	int n_value;
	int n_class;
	// �����擾����
	calc_exp5( p_calcpar );
	for(;;){
		// �擾�����l
		n_value = p_calcpar->n_result;
		n_class = p_calcpar->n_class;
		// ���Z�q��]������
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
//	1.	���{�ꖼ
//		���Z�q�����i���x���T�j
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void calc_exp5( t_calc_param *p_calcpar )
{
	int n_class;
	bool b_lpar;
	// �P����擾����
	calc_getword( p_calcpar );
	// �擾�����l
	n_class = p_calcpar->n_class;
	// ���Z�q��]������
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
//	1.	���{�ꖼ
//		�P��̎擾
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�萔�Ȃ炻�̒l�� n_result �֕Ԃ��A���Z�q�Ȃ� n_result �͔j�󂵂Ȃ�
// -------------------------------------------------------------
static void calc_getword( t_calc_param *p_calcpar )
{
	// �z���C�g�X�y�[�X���X�L�b�v����
	calc_skipspace( p_calcpar );
	// ���Z�q���ǂ������f����
	if( calc_isexp( p_calcpar ) ) return;
	// �萔���ǂ������f����
	if( calc_isnumber( p_calcpar ) ) return;
	// ����ȊO
	if( p_calcpar->n_class!=calc_class_none &&
		p_calcpar->n_class!=calc_class_const &&
		p_calcpar->n_class!=calc_class_rpar )
		throw( (int)calc_err_syntax );
	p_calcpar->n_class = calc_class_none;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���Z�q���ǂ������f����
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		true	...	���Z�q
//		false	...	����ȊO
//	4.	���l
//		���Z�q�Ȃ� p_calcpar->n_class �ɉ��Z�q��ނ���������
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
			p_calcpar->n_class = n_class;	// ��v�������Z�q
			p_calcpar->n_ptr += j;			// ���Z�q�̎��̈ʒu
			return true;
		}
	}
	return false;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�萔���ǂ������f����
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		true	...	�萔
//		false	...	����ȊO
//	4.	���l
//		�萔�Ȃ� p_calcpar �ɒ萔��ǂݍ���
// -------------------------------------------------------------
static bool calc_isnumber( t_calc_param *p_calcpar )
{
	if( calc_ishexnumber( p_calcpar ) ) return true;	// 16�i��
	if( calc_isbinnumber( p_calcpar ) ) return true;	//  2�i��
	if( calc_isdecnumber( p_calcpar ) ) return true;	// 10�i��
	return false;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�P�U�i�����ǂ������肷��
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		true	...	�P�U�i��
//		false	...	����ȊO
//	4.	���l
//		�P�U�i���Ȃ� p_calcpar �ɒ萔��ǂݍ���
// -------------------------------------------------------------
static bool calc_ishexnumber( t_calc_param *p_calcpar )
{
	char word[ WORD_SIZE ] = "";
	int pword = 0;
	int n_ptr;
	int n_result = 0;
	int n;
	const int n_base = 16;
	// �擪�͕K�����l
	n_ptr = p_calcpar->n_ptr;
	if( !isdigit( p_calcpar->s_line[ n_ptr ] ) ) return false;
	while( isxdigit( p_calcpar->s_line[ n_ptr ] ) && pword < WORD_SIZE-1 ){
		word[ pword++ ] = (char)toupper( p_calcpar->s_line[ n_ptr++ ] );
	}
	word[ pword ] = 0;
	// 16�i���C���q��F������
	if( (char)toupper( p_calcpar->s_line[ n_ptr ] )!='H' ){
		return false;
	}
	++n_ptr;
	// ���̈ʒu
	p_calcpar->n_ptr = n_ptr;
	// ���l�ɕϊ�����
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
//	1.	���{�ꖼ
//		�P�O�i�����ǂ������肷��
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		true	...	�P�O�i��
//		false	...	����ȊO
//	4.	���l
//		�P�O�i���Ȃ� p_calcpar �ɒ萔��ǂݍ���
// -------------------------------------------------------------
static bool calc_isdecnumber( t_calc_param *p_calcpar )
{
	char word[ WORD_SIZE ] = "";
	int pword=0;
	int n_ptr;
	int n_result = 0;
	int n;
	const int n_base = 10;
	// �擪�͕K�����l
	n_ptr = p_calcpar->n_ptr;
	if( !isdigit( p_calcpar->s_line[ n_ptr ] ) ) return false;
	while( isdigit( p_calcpar->s_line[ n_ptr ] ) && pword < WORD_SIZE-1 ){
		word[ pword++ ] = (char)(p_calcpar->s_line[ n_ptr++ ]);
	}
	word[ pword ]=0;
	// 10�i���C���q��F������i�ȗ��\�j
	if( (char)toupper( p_calcpar->s_line[ n_ptr ] )=='D' ){
		++n_ptr;
	}
	// ���̈ʒu
	p_calcpar->n_ptr = n_ptr;
	// ���l�ɕϊ�����
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
//	1.	���{�ꖼ
//	�Q�i�����ǂ������肷��
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		true	...	�Q�i��
//		false	...	����ȊO
//	4.	���l
//		�萔�Ȃ� p_calcpar �ɒ萔��ǂݍ���
// -------------------------------------------------------------
static bool calc_isbinnumber( t_calc_param *p_calcpar )
{
	char word[ WORD_SIZE ] = "";
	int pword=0;
	int n_ptr;
	int n_result = 0;
	int n;
	const int n_base = 2;
	// �擪�͕K�� 0 �� 1
	n_ptr = p_calcpar->n_ptr;
	if( !isbindigit( p_calcpar->s_line[ n_ptr ] ) ) return false;
	while( isbindigit( p_calcpar->s_line[ n_ptr ] ) && pword < WORD_SIZE-1 ){
		word[ pword++ ] = (char)(p_calcpar->s_line[ n_ptr++ ]);
	}
	word[ pword ] = 0;
	// 2�i���C���q��F������
	if( (char)toupper( p_calcpar->s_line[ n_ptr ] )!='B' ){
		return false;
	}
	++n_ptr;
	// ���̈ʒu
	p_calcpar->n_ptr = n_ptr;
	// ���l�ɕϊ�����
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
//	1.	���{�ꖼ
//		�Q�i���̌����ǂ������肷��
//	2.	����
//		c	...	(I)	������������
//	3.	�Ԓl
//		true	...	�Q�i���̌�
//		false	...	����ȊO
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static inline bool isbindigit( int c )
{
	return( c=='0' || c=='1' );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�z���C�g�X�y�[�X���X�L�b�v����
//	2.	����
//		p_calcpar	...	(I)	���Z�����p�\���̂̃|�C���^
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static inline void calc_skipspace( t_calc_param *p_calcpar )
{
	while( isspace( p_calcpar->s_line[ p_calcpar->n_ptr ] ) ) ++(p_calcpar->n_ptr);
}
