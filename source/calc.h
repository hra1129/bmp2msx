// -------------------------------------------------------------
// �l�����Z����
//
//												2000 Hara
// -------------------------------------------------------------

#ifndef _CALC_H_
#define _CALC_H_

// -------------------------------------------------------------
// ���Z�����p�����[�^�\����

typedef struct {
	const char	*s_line;		// �������̕�����
	int			n_ptr;			// s_line ���̌��݂̒��ڈʒu
	int			n_class;		// ���o�����P��̎��
	int			n_result;		// �v�Z����
	bool		b_lpar;			// ( ����n�܂鉉�Z�������������ۂ�
	int			n_level;		// ���̊K�w���x���i�J�b�R�̃l�X�g���x���j
	bool		b_number;		// �K���l��Ԃ��Ȃ���΂Ȃ�Ȃ����ۂ�
} t_calc_param;

// -------------------------------------------------------------
// n_class �̒l

enum {
	calc_class_none,			// ����`
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
	calc_class_const,			// �萔
};

// -------------------------------------------------------------
// �G���[�R�[�h

enum {
	calc_err_noerr,				// �G���[����
	calc_err_syntax,			// ���@�G���[
	calc_err_level,				// �ߏ�ȃJ�b�R�̃l�X�g
	calc_err_unknown_symbol,	// �]���ł��Ȃ��V���{�������݂��܂�
	calc_err_divided_by_zero,	// 0 ���Z�G���[
};

#define calc_arraysize( a ) ( sizeof(a)/sizeof((a)[0]) )

// -------------------------------------------------------------
// �֐�

bool calc_exp( int *n_result,const char *s_line,int *ptr );

#endif
