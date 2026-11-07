// See script.h.  The tokenizer, command dispatch and argument parsing
// follow ScriptProc.cpp function by function (scrGetWord / scrGetString /
// scrGetNumber / scrSearch and the scr* command handlers), with Win32
// calls replaced by their CLI equivalents noted in script.h.
#include "script.h"

#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

#include <iconv.h>

#include "calc.h"
#include "utils.h"
#include "convert_run.h"

namespace {

const int WORD_MAX = 512;

struct ScrWord {
	const char* word;
	int id;
};

struct Script {
	std::string text;		// whole script, UTF-8, NUL-terminated by std::string
	size_t ptr = 0;
	SETTING set;			// scrset in ScriptProc.cpp
	std::string target;		// szTarget in ScriptProc.cpp
	std::string output_dir;
	std::string error;
	bool failed = false;

	explicit Script( const SETTING& initial ) : set( initial ) {}
};

bool is_space( char c ) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

void skip_space( Script& s ) {
	while( s.ptr < s.text.size() && is_space( s.text[ s.ptr ] ) ) ++s.ptr;
}

// ScriptProc.cpp: scrGetWord (a word ends at space, '=' or ',')
std::string get_word( Script& s ) {
	skip_space( s );
	std::string word;
	while( s.ptr < s.text.size() ) {
		char c = s.text[ s.ptr ];
		if( c == '=' || c == ',' || is_space( c ) ) break;
		word += c;
		++s.ptr;
		if( word.size() >= WORD_MAX - 1 ) break;
	}
	return word;
}

bool iequals( const std::string& a, const char* b ) {
	size_t n = strlen( b );
	if( a.size() != n ) return false;
	for( size_t i = 0; i < n; ++i ) {
		char ca = a[ i ], cb = b[ i ];
		if( ca >= 'a' && ca <= 'z' ) ca = (char)( ca - 'a' + 'A' );
		if( cb >= 'a' && cb <= 'z' ) cb = (char)( cb - 'a' + 'A' );
		if( ca != cb ) return false;
	}
	return true;
}

int search( const std::string& word, const ScrWord* words, size_t count ) {
	for( size_t i = 0; i < count; ++i ) {
		if( iequals( word, words[ i ].word ) ) return words[ i ].id;
	}
	return -1;
}

// ScriptProc.cpp: scrGetString (a "..." quoted string)
bool get_string( Script& s, std::string& out ) {
	skip_space( s );
	if( s.ptr >= s.text.size() || s.text[ s.ptr ] != '"' ) {
		s.error = "string expected";
		return false;
	}
	++s.ptr;
	out.clear();
	// The original copies bytes >= ' ' (its Shift_JIS bytes are all
	// >= 0x20 as signed char there); with UTF-8, multi-byte sequences
	// are >= 0x80 and must also be accepted, so compare unsigned.
	while( s.ptr < s.text.size() && (unsigned char) s.text[ s.ptr ] >= ' ' && s.text[ s.ptr ] != '"' ) {
		out += s.text[ s.ptr ];
		++s.ptr;
	}
	if( s.ptr >= s.text.size() || s.text[ s.ptr ] != '"' ) {
		s.error = "unterminated string";
		return false;
	}
	++s.ptr;
	return true;
}

// ScriptProc.cpp: scrGetNumber (expression over the rest of the text,
/// evaluated by the real calc.cpp, starting at index 0 of the substring)
bool get_number( Script& s, int* num ) {
	skip_space( s );
	int p = 0;
	if( !calc_exp( num, s.text.c_str() + s.ptr, &p ) ) return false;
	s.ptr += (size_t) p;
	return true;
}

// Expression at the current position of the full text (scrScreen style:
// calc_exp receives the running index).
bool get_number_here( Script& s, int* num ) {
	int p = (int) s.ptr;
	if( !calc_exp( num, s.text.c_str(), &p ) ) return false;
	s.ptr = (size_t) p;
	return true;
}

bool fail( Script& s, const char* msg ) {
	s.error = msg;
	return false;
}

// ---------------------------------------------------------- commands
enum {
	SW_LOADSETTING = 0, SW_CONVERT, SW_SETFILENAME, SW_COMMENT, SW_SCREEN,
	SW_MESSAGE, SW_SETFLAG, SW_SETTING, SW_PALETTE, SW_PALMODE, SW_CUTERR,
	SW_GOSAVAL, SW_SELCOL, SW_DTHADD, SW_ALGOMODE
};

const ScrWord scrword[] = {
	{ "LOADSETTING", SW_LOADSETTING }, { "設定読込", SW_LOADSETTING },
	{ "CONVERT", SW_CONVERT }, { "変換処理", SW_CONVERT },
	{ "SETFILENAME", SW_SETFILENAME }, { "ファイル名", SW_SETFILENAME },
	{ ";", SW_COMMENT },
	{ "SCREEN", SW_SCREEN }, { "画面モード", SW_SCREEN },
	{ "MSG", SW_MESSAGE }, { "メッセージ", SW_MESSAGE },
	{ "SETFLAG", SW_SETFLAG }, { "フラグ", SW_SETFLAG },
	{ "SETTING", SW_SETTING }, { "設定画面表示", SW_SETTING },
	{ "PALETTE", SW_PALETTE }, { "パレット", SW_PALETTE },
	{ "PALMODE", SW_PALMODE }, { "パレット動作", SW_PALMODE },
	{ "CUTERR", SW_CUTERR }, { "切り捨て誤差", SW_CUTERR },
	{ "GOSAVAL", SW_GOSAVAL }, { "誤差拡散係数", SW_GOSAVAL },
	{ "SELCOLMODE", SW_SELCOL }, { "選色モード", SW_SELCOL },
	{ "DITHERADD", SW_DTHADD }, { "ディザ加算", SW_DTHADD },
	{ "ALGOMODE", SW_ALGOMODE }, { "自然画生成", SW_ALGOMODE },
};

#define OFFSET_OF( m ) ( (int) offsetof( SETTING, m ) )
const ScrWord flgword[] = {
	{ "誤差拡散", OFFSET_OF( diffusion_error_enable ) }, { "GOSA", OFFSET_OF( diffusion_error_enable ) },
	{ "インターレース", OFFSET_OF( interlace ) }, { "INTERACE", OFFSET_OF( interlace ) },
	{ "固定パレット", OFFSET_OF( fixed_palette ) }, { "PALNOCHG", OFFSET_OF( fixed_palette ) },
	{ "サイズ調節", OFFSET_OF( resize_enable ) }, { "RESIZE", OFFSET_OF( resize_enable ) },
	{ "自動ファイル名", OFFSET_OF( AutoName ) }, { "AUTONAME", OFFSET_OF( AutoName ) },
	{ "色差再計算", OFFSET_OF( JKrc ) }, { "COLORRECALC", OFFSET_OF( JKrc ) },
	{ "ゼロ使用不可", OFFSET_OF( NonZero ) }, { "ZERONOUSE", OFFSET_OF( NonZero ) },
	{ "タイル", OFFSET_OF( Tile ) }, { "TILE", OFFSET_OF( Tile ) },
};

const ScrWord flgbool[] = {
	{ "TRUE", 1 }, { "FALSE", 0 }, { "ON", 1 }, { "OFF", 0 },
};
const ScrWord flgpmode[] = {
	{ "NOUSE", PALEN_NONE }, { "AUTO", PALEN_AUTO }, { "CUSTOM", PALEN_USE },
	{ "未使用", PALEN_NONE }, { "自動", PALEN_AUTO }, { "指定", PALEN_USE },
};
const ScrWord flgselmode[] = {
	{ "SCATTER", SELCOL_BUNSAN }, { "MAX", SELCOL_TABUNPU },
	{ "分散", SELCOL_BUNSAN }, { "多分布", SELCOL_TABUNPU },
};
const ScrWord flgdthadd[] = {
	{ "ROTATE", EADD_ROTATE }, { "NONE", EADD_NONE }, { "X", EADD_X }, { "Y", EADD_Y },
	{ "回転", EADD_ROTATE }, { "同位相", EADD_NONE }, { "Ｘずらし", EADD_X }, { "Ｙずらし", EADD_Y },
};
const ScrWord flgalgo[] = {
	{ "無し", ALGO_NONE }, { "再計算", ALGO_RECALC }, { "パワフル", ALGO_POWERFUL },
	{ "ＮＴＳＣ改", ALGO_NTSC2 }, { "色合い", ALGO_RECALC2 },
	{ "NONE", ALGO_NONE }, { "RECALC", ALGO_RECALC }, { "POWERFUL", ALGO_POWERFUL },
	{ "NTSC2", ALGO_NTSC2 }, { "COLOR", ALGO_RECALC2 },
};

template <size_t N>
int search_in( const std::string& word, const ScrWord ( &words )[ N ] ) {
	return search( word, words, N );
}

bool cmd_loadsetting( Script& s ) {
	std::string filename;
	if( !get_string( s, filename ) ) return false;
	if( !GetCfgFile( &s.set, filename.c_str() ) ) return fail( s, "cannot load setting file" );
	return true;
}

bool cmd_convert( Script& s ) {
	if( s.target.empty() ) return fail( s, "no file name set (SETFILENAME)" );
	std::string err;
	if( !cli_convert_file( s.set, s.target, s.output_dir, &err ) ) {
		fprintf( stderr, "bmp2msx-cli: %s\n", err.c_str() );
		return fail( s, "conversion failed" );
	}
	return true;
}

bool cmd_setfilename( Script& s ) {
	return get_string( s, s.target );
}

bool cmd_comment( Script& s ) {
	while( s.ptr < s.text.size() && s.text[ s.ptr ] != '\n' && s.text[ s.ptr ] != '\r' ) ++s.ptr;
	return true;
}

bool cmd_screen( Script& s ) {
	// CLI extension: SCREEN 8P / SCREEN 8P256 (Phase 3 EPAL modes) and
	// SCREEN 2P/3P/5P/6P/7P (+256) for the SCREEN2-7 EPAL modes
	{
		size_t save = s.ptr;
		std::string w = get_word( s );
		if( iequals( w, "8P" ) ) { s.set.screen_mode = MD_SC8P; return true; }
		if( iequals( w, "8P256" ) ) { s.set.screen_mode = MD_SC8P_256L; return true; }
		if( iequals( w, "2P" ) ) { s.set.screen_mode = MD_SC2P; return true; }
		if( iequals( w, "3P" ) ) { s.set.screen_mode = MD_SC3P; return true; }
		if( iequals( w, "5P" ) ) { s.set.screen_mode = MD_SC5P; return true; }
		if( iequals( w, "6P" ) ) { s.set.screen_mode = MD_SC6P; return true; }
		if( iequals( w, "7P" ) ) { s.set.screen_mode = MD_SC7P; return true; }
		if( iequals( w, "5P256" ) ) { s.set.screen_mode = MD_SC5P_256L; return true; }
		if( iequals( w, "6P256" ) ) { s.set.screen_mode = MD_SC6P_256L; return true; }
		if( iequals( w, "7P256" ) ) { s.set.screen_mode = MD_SC7P_256L; return true; }
		s.ptr = save;
	}
	int m;
	if( !get_number_here( s, &m ) ) return fail( s, "invalid screen mode" );
	switch( m ) {
	case 2: m = MD_SC2; break;
	case 3: m = MD_SC3; break;
	case 4: m = MD_SC2; break;
	case 5: m = MD_SC5; break;
	case 6: m = MD_SC6; break;
	case 7: m = MD_SC7; break;
	case 8: m = MD_SC8; break;
	case 10: m = MD_SC10; break;
	case 11: m = MD_SC10; break;
	case 12: m = MD_SC12; break;
	default: return fail( s, "invalid screen mode" );
	}
	s.set.screen_mode = m;
	return true;
}

bool cmd_message( Script& s ) {
	std::string word;
	if( !get_string( s, word ) ) return false;
	printf( "%s\n", word.c_str() );
	fflush( stdout );
	return true;
}

bool cmd_setflag( Script& s ) {
	std::string word = get_word( s );
	if( word.empty() ) return fail( s, "flag name expected" );
	int p = search_in( word, flgword );
	if( p < 0 ) return fail( s, "unknown flag name" );
	skip_space( s );
	if( s.ptr < s.text.size() ) ++s.ptr;	// skip '=' (as the original does)
	skip_space( s );
	word = get_word( s );
	if( word.empty() ) return fail( s, "flag value expected" );
	int b = search_in( word, flgbool );
	if( b < 0 ) return fail( s, "unknown flag value" );
	*( (bool*)( ( (char*) &s.set ) + p ) ) = ( b != 0 );
	return true;
}

bool cmd_setting( Script& s ) {
	// Headless equivalent of opening the setting dialog: show the
	// settings that subsequent CONVERT commands will use.
	printf( "%s\n", cli_describe_setting( s.set ).c_str() );
	fflush( stdout );
	return true;
}

bool cmd_palette( Script& s ) {
	int p, r, g, b;
	if( !get_number( s, &p ) ) return fail( s, "invalid palette" );
	++s.ptr;
	if( !get_number( s, &r ) ) return fail( s, "invalid palette" );
	++s.ptr;
	if( !get_number( s, &g ) ) return fail( s, "invalid palette" );
	++s.ptr;
	if( !get_number( s, &b ) ) return fail( s, "invalid palette" );
	if( p < 0 || p > 15 || r < 0 || r > 7 || g < 0 || g > 7 || b < 0 || b > 7 ) return fail( s, "palette value out of range" );
	s.set.color_palette[ p ].red = r;
	s.set.color_palette[ p ].green = g;
	s.set.color_palette[ p ].blue = b;
	return true;
}

bool cmd_palmode( Script& s ) {
	int p;
	if( !get_number( s, &p ) ) return fail( s, "invalid palette mode" );
	++s.ptr;
	std::string word = get_word( s );
	if( word.empty() ) return fail( s, "palette mode value expected" );
	int b = search_in( word, flgpmode );
	if( b < 0 ) return fail( s, "unknown palette mode value" );
	if( p < 0 || p > 15 ) return fail( s, "palette number out of range" );
	s.set.PalEn[ p ] = (uint8_t) b;
	return true;
}

bool cmd_cuterr( Script& s ) {
	int r;
	if( !get_number( s, &r ) ) return fail( s, "invalid cut-off error value" );
	if( r < 0 || r > 255 ) return fail( s, "cut-off error value out of range" );
	s.set.err = r;
	return true;
}

bool cmd_gosaval( Script& s ) {
	int r;
	if( !get_number( s, &r ) ) return fail( s, "invalid diffusion coefficient" );
	if( r < 0 || r > 500 ) return fail( s, "diffusion coefficient out of range" );
	s.set.diffusion_error_coef = (float) r / 1000.0f;
	return true;
}

bool cmd_selcol( Script& s ) {
	std::string word = get_word( s );
	if( word.empty() ) return fail( s, "select-color mode expected" );
	int b = search_in( word, flgselmode );
	if( b < 0 ) return fail( s, "unknown select-color mode" );
	s.set.SelCol = b;
	return true;
}

bool cmd_dthadd( Script& s ) {
	std::string word = get_word( s );
	if( word.empty() ) return fail( s, "dither-add mode expected" );
	int b = search_in( word, flgdthadd );
	if( b < 0 ) return fail( s, "unknown dither-add mode" );
	s.set.ErrAdd = b;
	return true;
}

bool cmd_algomode( Script& s ) {
	std::string word = get_word( s );
	if( word.empty() ) return fail( s, "algorithm mode expected" );
	int b = search_in( word, flgalgo );
	if( b < 0 ) return fail( s, "unknown algorithm mode" );
	s.set.AlgoMode = b;
	return true;
}

bool dispatch( Script& s, int cmd ) {
	switch( cmd ) {
	case SW_LOADSETTING: return cmd_loadsetting( s );
	case SW_CONVERT: return cmd_convert( s );
	case SW_SETFILENAME: return cmd_setfilename( s );
	case SW_COMMENT: return cmd_comment( s );
	case SW_SCREEN: return cmd_screen( s );
	case SW_MESSAGE: return cmd_message( s );
	case SW_SETFLAG: return cmd_setflag( s );
	case SW_SETTING: return cmd_setting( s );
	case SW_PALETTE: return cmd_palette( s );
	case SW_PALMODE: return cmd_palmode( s );
	case SW_CUTERR: return cmd_cuterr( s );
	case SW_GOSAVAL: return cmd_gosaval( s );
	case SW_SELCOL: return cmd_selcol( s );
	case SW_DTHADD: return cmd_dthadd( s );
	case SW_ALGOMODE: return cmd_algomode( s );
	}
	return fail( s, "unknown command" );
}

// ---------------------------------------------------------- file loading
bool valid_utf8( const std::string& s ) {
	const unsigned char* p = (const unsigned char*) s.data();
	size_t i = 0, n = s.size();
	while( i < n ) {
		unsigned char c = p[ i ];
		if( c < 0x80 ) { ++i; continue; }
		int extra;
		uint32_t cp;
		if( ( c & 0xE0 ) == 0xC0 ) { extra = 1; cp = c & 0x1F; if( cp == 0 ) return false; }
		else if( ( c & 0xF0 ) == 0xE0 ) { extra = 2; cp = c & 0x0F; }
		else if( ( c & 0xF8 ) == 0xF0 ) { extra = 3; cp = c & 0x07; if( cp > 4 ) return false; }
		else return false;
		if( i + extra >= n ) return false;
		for( int k = 1; k <= extra; ++k ) {
			if( ( p[ i + k ] & 0xC0 ) != 0x80 ) return false;
			cp = ( cp << 6 ) | ( p[ i + k ] & 0x3F );
		}
		if( extra == 2 && cp < 0x800 ) return false;
		if( extra == 3 && ( cp < 0x10000 || cp > 0x10FFFF ) ) return false;
		i += extra + 1;
	}
	return true;
}

bool sjis_to_utf8( const std::string& in, std::string& out ) {
	iconv_t cd = iconv_open( "UTF-8", "SHIFT_JIS" );
	if( cd == (iconv_t) -1 ) return false;
	out.clear();
	out.reserve( in.size() * 2 );
	char* inbuf = const_cast<char*>( in.data() );
	size_t inleft = in.size();
	char chunk[ 8192 ];
	while( inleft > 0 ) {
		char* outptr = chunk;
		size_t outleft = sizeof( chunk );
		size_t r = iconv( cd, &inbuf, &inleft, &outptr, &outleft );
		out.append( chunk, sizeof( chunk ) - outleft );
		if( r == (size_t) -1 ) { iconv_close( cd ); return false; }
	}
	iconv_close( cd );
	return true;
}

} // namespace

int cli_run_script( const char* script_path, const SETTING& initial,
					const std::string& initial_target, const std::string& output_dir ) {
	FILE* f = fopen( script_path, "rb" );
	if( !f ) {
		fprintf( stderr, "bmp2msx-cli: cannot open script: %s\n", script_path );
		return 1;
	}
	fseek( f, 0, SEEK_END );
	long size = ftell( f );
	fseek( f, 0, SEEK_SET );
	std::string raw;
	if( size > 0 ) {
		raw.resize( (size_t) size );
		if( fread( raw.data(), 1, raw.size(), f ) != raw.size() ) {
			fclose( f );
			fprintf( stderr, "bmp2msx-cli: cannot read script: %s\n", script_path );
			return 1;
		}
	}
	fclose( f );
	// Strip a UTF-8 BOM if present.
	if( raw.size() >= 3 && (unsigned char) raw[ 0 ] == 0xEF && (unsigned char) raw[ 1 ] == 0xBB && (unsigned char) raw[ 2 ] == 0xBF ) {
		raw.erase( 0, 3 );
	}

	Script s( initial );
	s.output_dir = output_dir;
	s.target = initial_target;
	if( valid_utf8( raw ) ) {
		s.text = raw;
	} else if( !sjis_to_utf8( raw, s.text ) ) {
		fprintf( stderr, "bmp2msx-cli: script is neither UTF-8 nor Shift_JIS: %s\n", script_path );
		return 1;
	}

	// ScriptProc.cpp: scrPlayScript main loop
	while( s.ptr < s.text.size() ) {
		std::string word = get_word( s );
		if( word.empty() ) break;
		int cmd = search_in( word, scrword );
		if( cmd < 0 ) {
			fprintf( stderr, "bmp2msx-cli: script syntax error at byte %zu: %s\n", s.ptr, word.c_str() );
			return 1;
		}
		if( !dispatch( s, cmd ) ) {
			fprintf( stderr, "bmp2msx-cli: script error at byte %zu: %s\n", s.ptr, s.error.c_str() );
			return 1;
		}
		if( s.ptr >= s.text.size() ) break;
	}
	return 0;
}
