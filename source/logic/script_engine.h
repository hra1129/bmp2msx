// --------------------------------------------------------------------
//	script engine
// ====================================================================
//	2023/May/27th	t.hara (HRA!)
// --------------------------------------------------------------------

#ifndef __SCRIPT_ENGINE_H__

#include <string>
#include <map>

// --------------------------------------------------------------------
class C_SCRIPT_ERROR {
public:
	std::string s_error_message;

	void set_error( const char *p_message, int line_no );

	C_SCRIPT_ERROR(): s_error_message("") {
	}
};

// --------------------------------------------------------------------
class C_SCRIPT_WORD {
public:
	enum C_TYPE {
		NONE,
		INTEGER_TYPE,
		STRING_TYPE,
		SYMBOL_TYPE,
		NAME_TYPE,
	};

	std::string s_value = "";
	int n_value = 0;
	C_TYPE n_type = C_TYPE::NONE;
};

// --------------------------------------------------------------------
class C_SCRIPT_ENGINE {
private:
	const char *p_script;		//	スクリプトテキスト
	int pos;					//	スクリプトテキストの現在の参照位置
	int line_no;				//	スクリプトテキストの現在の行番号

	// --------------------------------------------------------------------
	//	コマンド処理関数
	// --------------------------------------------------------------------
	C_SCRIPT_ERROR command_load_config( void );
	C_SCRIPT_ERROR command_convert( void );
	C_SCRIPT_ERROR command_message( void );
	C_SCRIPT_ERROR command_set_property( void );
	C_SCRIPT_ERROR command_show_config_dialog( void );
	C_SCRIPT_ERROR command_palette( void );

	// --------------------------------------------------------------------
	//	現在の参照位置から単語を取得
	//	input)
	//		none
	//	output)
	//		C_SCRIPT_WORD による単語データ
	// --------------------------------------------------------------------
	C_SCRIPT_WORD get_word( void );

	// --------------------------------------------------------------------
	//	ホワイトスペース/コメントを読み飛ばす
	//	input)
	//		none
	//	output)
	//		C_SCRIPT_WORD による単語データ
	// --------------------------------------------------------------------
	void skip_white( void );

public:
	// --------------------------------------------------------------------
	//	スクリプト実行
	//	input)
	//		p_script .... スクリプト文字列
	//	output)
	//		C_ERROR_CODE による終了コード
	// --------------------------------------------------------------------
	C_SCRIPT_ERROR run( const char *p_script );
};

#endif	//	__SCRIPT_ENGINE_H__
