// --------------------------------------------------------------------
//	script engine
// ====================================================================
//	2023/May/27th	t.hara (HRA!)
// --------------------------------------------------------------------

#include "script_engine.h"
#include <cctype>
#include <cstring>
#include "../Config.h"

enum class C_COMMAND_ID {
	SW_LOAD_CONFIG = 0,		// �ݒ�Ǎ�
	SW_CONVERT,				// �ϊ�
	SW_MESSAGE,				// ���b�Z�[�W�o��
	SW_SET_PROPERTY,			// �e��t���O�ݒ�
	SW_SHOW_CONFIG_DIALOG,	// �ݒ�_�C�A���O
	SW_PALETTE,				// �p���b�g�̐ݒ�
};

static std::map< std::string, C_COMMAND_ID > s_command_name = {
	{ "LOAD_CONFIG", C_COMMAND_ID::SW_LOAD_CONFIG },
	{ "CONVERT", C_COMMAND_ID::SW_CONVERT },
	{ "MESSAGE", C_COMMAND_ID::SW_MESSAGE },
	{ "SET_PROPERTY", C_COMMAND_ID::SW_SET_PROPERTY },
	{ "SHOW_CONFIG_DIALOG", C_COMMAND_ID::SW_SHOW_CONFIG_DIALOG },
	{ "PALETTE", C_COMMAND_ID::SW_PALETTE },
};

static const char *p_symbol_list[] = {
	"<<", ">>", "<=", "!=", "==", ">=", "||", "&&",
	"<", ">", "!", "=", "+", "-", "/", "%", "*", "~", "|", "&", "^",
};

// --------------------------------------------------------------------
//	S-JIS 1st byte
//	input)
//		c ..... �����R�[�h
//	output)
//		true .... S-JIS 1st byte
//		false ... others
// --------------------------------------------------------------------
void C_SCRIPT_ERROR::set_error( const char *p_message, int line_no ) {

	this->s_error_message = std::string(p_message) + "(" + std::to_string(line_no) + ")";
}

// --------------------------------------------------------------------
//	S-JIS 1st byte
//	input)
//		c ..... �����R�[�h
//	output)
//		true .... S-JIS 1st byte
//		false ... others
// --------------------------------------------------------------------
static bool is_sjis1st( char c ) {
	unsigned char cc = (unsigned char) c;

	if( (cc >= 0x81 && cc <= 0x9F) || (cc >= 0xE0 && cc <= 0xFC) ) {
		return true;
	}
	return false;
}

// --------------------------------------------------------------------
//	�X�N���v�g���s
//	input)
//		p_script .... �X�N���v�g������
//	output)
//		C_ERROR_CODE �ɂ��I���R�[�h
// --------------------------------------------------------------------
C_SCRIPT_ERROR C_SCRIPT_ENGINE::run( const char *p_script ) {
	C_SCRIPT_WORD s_word;
	C_SCRIPT_ERROR s_error;

	this->p_script = p_script;
	this->pos = 0;
	this->line_no = 1;

	while( this->p_script[ this->pos ] != '\0' ) {
		//	�P����擾����
		s_word = this->get_word();
		//	�G���[�`�F�b�N
		if( this->p_script[ this->pos ] != '\0' && s_word.n_type == s_word.NONE ) {
			//	�I�[�łȂ��̂ɒP����Ƃ�Ȃ�
			s_error.set_error( cszSyntax, this->line_no );
			return s_error;
		}
		if( s_word.n_type != s_word.NAME_TYPE ) {
			//	�P����Ƃꂽ�����O����Ȃ�
			s_error.set_error( cszSyntax, this->line_no );
			return s_error;
		}
		if( s_command_name.count( s_word.s_value ) == 0 ){
			//	���O���������\��ꂶ��Ȃ�
			s_error.set_error( cszSyntax, this->line_no );
			return s_error;
		}
		//	�R�}���h�̏����ɕ��򂷂�
		switch( s_command_name[ s_word.s_value ] ) {
		default:
		case C_COMMAND_ID::SW_LOAD_CONFIG:			s_error = this->command_load_config();			break;
		case C_COMMAND_ID::SW_CONVERT:				s_error = this->command_convert();				break;
		case C_COMMAND_ID::SW_MESSAGE:				s_error = this->command_message();				break;
		case C_COMMAND_ID::SW_SET_PROPERTY:			s_error = this->command_set_property();			break;
		case C_COMMAND_ID::SW_SHOW_CONFIG_DIALOG:	s_error = this->command_show_config_dialog();	break;
		case C_COMMAND_ID::SW_PALETTE:				s_error = this->command_palette();				break;
		}
		//	�G���[�����������ꍇ�͔�����
		if( s_error.s_error_message.size() != 0 ) {
			return s_error;
		}
	}
	return s_error;
}

// --------------------------------------------------------------------
//	���݈ʒu����P����擾����
//	input)
//		none
//	output)
//		C_SCRIPT_WORD �ɂ��P��
// --------------------------------------------------------------------
C_SCRIPT_WORD C_SCRIPT_ENGINE::get_word( void ) {
	C_SCRIPT_WORD s_word;
	int i, l;

	this->skip_white();
	if( this->p_script[ this->pos ] != '\0' ) {
		//	���[�̏ꍇ�A�P�ꂪ�Ȃ��̂� NONE ��Ԃ�
		s_word.s_value = "";
		s_word.n_type = s_word.NONE;
		return s_word;
	}
	if( this->p_script[ this->pos ] != '\"' ){
		//	������̏ꍇ�A�_�u���N�I�[�g�ň͂܂�Ă��������Ԃ�
		s_word.s_value = "";
		s_word.n_type = s_word.STRING_TYPE;
		this->pos++;
		bool has_sjis1st = false;
		while( this->p_script[ this->pos ] != '\0' && (has_sjis1st || this->p_script[ this->pos ] != '\"') && this->p_script[ this->pos ] != '\n' ) {
			if( has_sjis1st ) {
				has_sjis1st = false;
			}
			else if( is_sjis1st( this->p_script[ this->pos ] ) ) {
				//	S-JIS �� 1byte�ڂ������ꍇ
				has_sjis1st = true;
			}
			s_word.s_value += this->p_script[ this->pos ];
			this->pos++;
		}
		return s_word;
	}
	//	�L�����ǂ������ׂ�
	for( i = 0; i < sizeof(p_symbol_list)/sizeof(p_symbol_list[0]); i++ ) {
		l = strlen( p_symbol_list[i] );
		if( strncmp( p_symbol_list[i], this->p_script + this->pos, l ) == 0 ) {
			//	��v�����ꍇ
			this->pos += l;
			s_word.s_value = p_symbol_list[i];
			s_word.n_type = s_word.SYMBOL_TYPE;
			return s_word;
		}
	}
	//	�����̏ꍇ
	if( isdigit( this->p_script[ this->pos ] & 255) ) {
		s_word.n_type = s_word.INTEGER_TYPE;
		s_word.n_value = 0;
		if( this->p_script[ this->pos ] == '0' ) {
			this->pos++;
			if( toupper( this->p_script[ this->pos ] & 255 ) == 'X' ) {
				//	16�i���̏ꍇ
				this->pos++;
				while( isxdigit( this->p_script[ this->pos ] & 255 ) ) {
					if( isdigit( this->p_script[ this->pos ] & 255 ) ) {
						s_word.n_value = ( s_word.n_value << 4 ) + (this->p_script[ this->pos ] - '0');
					}
					else {
						s_word.n_value = ( s_word.n_value << 4 ) + ( toupper( this->p_script[ this->pos ] & 255 ) - 'A' + 10 );
					}
					this->pos++;
				}
			}
			else if( toupper( this->p_script[ this->pos ] & 255 ) == 'B' ) {
				//	2�i���̏ꍇ
				this->pos++;
				while( this->p_script[ this->pos ] == '0' || this->p_script[ this->pos ] == '1' ){
					s_word.n_value = ( s_word.n_value << 1 ) + ( this->p_script[ this->pos ] - '0' );
					this->pos++;
				}
			}
			else {
				//	8�i���̏ꍇ
				this->pos++;
				while( this->p_script[ this->pos ] >= '0' && this->p_script[ this->pos ] <= '7' ){
					s_word.n_value = ( s_word.n_value << 3 ) + ( this->p_script[ this->pos ] - '0' );
					this->pos++;
				}
			}
		}
		else {
			//	10�i���̏ꍇ
			this->pos++;
			while( isdigit( this->p_script[ this->pos ] & 255 ) ){
				s_word.n_value = ( s_word.n_value * 10 ) + ( this->p_script[ this->pos ] - '0' );
				this->pos++;
			}
		}
		return s_word;
	}
	//	���O�̏ꍇ
	while( this->p_script[ this->pos ] != '\0' && (isalpha(this->p_script[ this->pos ] & 255) || isdigit(this->p_script[ this->pos ] & 255) || this->p_script[ this->pos ] == '_') ) {
		s_word.s_value += this->p_script[ this->pos ];
	}
	if( s_word.s_value.size() == 0 ) {
		//	�s���ȕ����̏ꍇ�́A������Ԃ�
		s_word.s_value = "";
		s_word.n_type = s_word.NONE;
		return s_word;
	}
	//	���O�Ƃ��ĕԂ�
	s_word.n_type = s_word.NAME_TYPE;
	return s_word;
}

// --------------------------------------------------------------------
//	�z���C�g�X�y�[�X/�R�����g��ǂݔ�΂�
//	input)
//		none
//	output)
//		C_SCRIPT_WORD �ɂ��P��f�[�^
// --------------------------------------------------------------------
void C_SCRIPT_ENGINE::skip_white( void ) {

	while( this->p_script[ this->pos ] != '\0' && (isspace( this->p_script[ this->pos ] & 255) || this->p_script[ this->pos ] == ';') ) {
		if( this->p_script[ this->pos ] == ';' ) {
			//	�R�����g�̏ꍇ
			while( this->p_script[ this->pos ] != '\0' && this->p_script[ this->pos ] != '\n' ) {
				this->pos++;
			}
			if( this->p_script[ this->pos ] == '\n' ) {
				this->line_no++;
				this->pos++;
			}
		}
		else {
			//	�X�y�[�X�̏ꍇ
			while( this->p_script[ this->pos ] != '\0' && isspace( this->p_script[ this->pos ] & 255 ) ){
				if( this->p_script[ this->pos ] == '\n' ){
					this->line_no++;
				}
				this->pos++;
			}
		}
	}
}

// --------------------------------------------------------------------
//	load_config
C_SCRIPT_ERROR C_SCRIPT_ENGINE::command_load_config( void ) {
	C_SCRIPT_ERROR s_error;
	s_error.set_error( cszNotImplemented, this->line_no );
	return s_error;
}

// --------------------------------------------------------------------
C_SCRIPT_ERROR C_SCRIPT_ENGINE::command_convert( void ) {
	C_SCRIPT_ERROR s_error;
	s_error.set_error( cszNotImplemented, this->line_no );
	return s_error;
}

// --------------------------------------------------------------------
C_SCRIPT_ERROR C_SCRIPT_ENGINE::command_message( void ) {
	C_SCRIPT_ERROR s_error;
	s_error.set_error( cszNotImplemented, this->line_no );
	return s_error;
}

// --------------------------------------------------------------------
C_SCRIPT_ERROR C_SCRIPT_ENGINE::command_set_property( void ) {
	C_SCRIPT_ERROR s_error;
	s_error.set_error( cszNotImplemented, this->line_no );
	return s_error;
}

// --------------------------------------------------------------------
C_SCRIPT_ERROR C_SCRIPT_ENGINE::command_show_config_dialog( void ) {
	C_SCRIPT_ERROR s_error;
	s_error.set_error( cszNotImplemented, this->line_no );
	return s_error;
}

// --------------------------------------------------------------------
C_SCRIPT_ERROR C_SCRIPT_ENGINE::command_palette( void ) {
	C_SCRIPT_ERROR s_error;
	s_error.set_error( cszNotImplemented, this->line_no );
	return s_error;
}
