// ---------------------------------------------------------------------------
// bmp2msx-cli: headless BMP2MSX for Linux.
//
// Two modes:
//   bmp2msx-cli [options] <image>          convert one image directly
//   bmp2msx-cli --script <file> [--input <image>] [options]
//                                          run a BMP2MSX script (the same
//                                          language as the GUI script
//                                          window; see src/script.h)
//
// Settings start from the application's own defaults (utils.cpp
// GetDefCfg) — the CLI deliberately does not read the raw BMP2MSX.B2M
// config file automatically; use --load-setting or a script's
// LOADSETTING to apply one explicitly.  If BMP2MSX_CUSTOM.BIN exists in
// the current directory it is loaded (calibration tables), exactly as
// the Windows application does at startup.
// ---------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "Convert.h"
#include "utils.h"
#include "convert_run.h"
#include "script.h"

static bool parse_onoff( const char* v, bool& out ) {
	if( !strcmp( v, "on" ) || !strcmp( v, "1" ) || !strcmp( v, "true" ) || !strcmp( v, "yes" ) ) { out = true; return true; }
	if( !strcmp( v, "off" ) || !strcmp( v, "0" ) || !strcmp( v, "false" ) || !strcmp( v, "no" ) ) { out = false; return true; }
	return false;
}

static bool parse_int( const char* v, int& out ) {
	char* end = nullptr;
	long n = strtol( v, &end, 0 );
	if( !end || *end ) return false;
	out = (int) n;
	return true;
}

static bool parse_rgb( const char* v, int max, int& r, int& g, int& b ) {
	return sscanf( v, "%d,%d,%d", &r, &g, &b ) == 3 && r >= 0 && r <= max && g >= 0 && g <= max && b >= 0 && b <= max;
}

static int screen_mode_from_name( const std::string& name ) {
	struct { const char* name; int mode; } table[] = {
		{ "2", MD_SC2 }, { "3", MD_SC3 }, { "4", MD_SC2 }, { "5", MD_SC5 },
		{ "6", MD_SC6 }, { "7", MD_SC7 }, { "8", MD_SC8 }, { "10", MD_SC10 },
		{ "11", MD_SC10 }, { "12", MD_SC12 },
		{ "5-256", MD_SC5_256L }, { "6-256", MD_SC6_256L }, { "7-256", MD_SC7_256L },
		{ "8-256", MD_SC8_256L }, { "10-256", MD_SC10_256L }, { "12-256", MD_SC12_256L },
		{ "8p", MD_SC8P }, { "8P", MD_SC8P },
		{ "8p-256", MD_SC8P_256L }, { "8P-256", MD_SC8P_256L }, { "8p256", MD_SC8P_256L },
		{ "2p", MD_SC2P }, { "2P", MD_SC2P }, { "3p", MD_SC3P }, { "3P", MD_SC3P },
		{ "5p", MD_SC5P }, { "5P", MD_SC5P }, { "6p", MD_SC6P }, { "6P", MD_SC6P },
		{ "7p", MD_SC7P }, { "7P", MD_SC7P },
		{ "5p-256", MD_SC5P_256L }, { "5P-256", MD_SC5P_256L }, { "5p256", MD_SC5P_256L },
		{ "6p-256", MD_SC6P_256L }, { "6P-256", MD_SC6P_256L }, { "6p256", MD_SC6P_256L },
		{ "7p-256", MD_SC7P_256L }, { "7P-256", MD_SC7P_256L }, { "7p256", MD_SC7P_256L },
	};
	for( auto& e : table ) if( name == e.name ) return e.mode;
	return -1;
}

static void usage( const char* argv0 ) {
	printf(
		"Usage:\n"
		"  %s [options] <image>\n"
		"  %s --script <file.txt> [--input <image>] [options]\n"
		"\n"
		"Options (conversion settings; defaults are the application's own):\n"
		"  --screen <mode>        2,3,4,5,6,7,8,10,11,12, *-256 for 256-line\n"
		"                         variants, 8p / 8p-256 for SCREEN8 EPAL (V9968),\n"
		"                         2p/3p/5p/6p/7p (and *-256) for SCREEN2-7 EPAL\n"
		"  --output-dir <dir>     write outputs here (default: beside the input)\n"
		"  --load-setting <file>  load a .B2M setting file before converting\n"
		"  --palette-output <bsave|pl|csv|none>\n"
		"  --basic / --no-basic   also write the .BAS loader (default: off)\n"
		"  --interlace <on|off>\n"
		"  --lines-192 <on|off>   192-line mode for SCREEN5 and above\n"
		"  --resize <on|off>      resize the image to the screen size\n"
		"  --resample <normal|antialias|copy>\n"
		"  --size-mode <force|msx>  aspect handling when resizing\n"
		"  --diffusion <on|off>   error diffusion\n"
		"  --diffusion-coef <0..500>      coefficient x1000 (GUI: GOSAVAL)\n"
		"  --diffusion-x-weight <0.0..1.0>\n"
		"  --dither <none|dith1|dith2|...|dith8>\n"
		"  --dither-add <rotate|none|x|y>\n"
		"  --cuterr <0..255>      cut-off error threshold\n"
		"  --selcol <scatter|max> palette selection algorithm\n"
		"  --algo <none|recalc|powerful|ntsc2|color>  SCREEN10/12 algorithm\n"
		"  --color-recalc <on|off>\n"
		"  --accuracy <24|21|18|15|12>    color reproduction accuracy (bits)\n"
		"  --fixed-palette <on|off>\n"
		"  --epal-hguard <on|off> EPAL improved selection (Wu + k-means base, h-domain feature proposals, true-SSE guard)\n"
		"  --sc2-pair <on|off>    SCREEN2/4 line-pair quantization (blend flat areas over 2 lines; SC2/SC2P only)\n"
		"  --palette-preset <msx1|msx2>\n"
		"  --palette <n>=<r>,<g>,<b>      palette entry n (values 0..7)\n"
		"  --palette-mode <n>=<auto|nouse|custom>\n"
		"  --nonzero <on|off>     do not use palette/color 0\n"
		"  --tile <on|off>        use 50:50 tiles (SCREEN2/3/5/6/7)\n"
		"  --sort <ignore|inc-auto|dec-auto|inc|dec>\n"
		"  --force-zero <none|color|matrix>\n"
		"  --fz-color <r>,<g>,<b>         color forced to 0 (values 0..255)\n"
		"  --fz-pos <x>,<y>               matrix position for --force-zero matrix\n"
		"  --bg <r>,<g>,<b>               background color (values 0..255)\n"
		"  --show-settings        print the effective settings and exit\n"
		"  --help                 this text\n",
		argv0, argv0 );
}

int main( int argc, char** argv ) {
	// Color tables: calibration file if present, factory tables otherwise
	// (this is exactly the Windows startup sequence).
	(void) GetCustomFile( "BMP2MSX_CUSTOM.BIN" );

	SETTING set;
	GetDefCfg( &set );
	set.PreView = 1;	// file output only

	std::string script_path, input_path, output_dir;
	bool show_settings = false;

	// --load-setting is applied first (pre-scan), so that explicit
	// command-line options override the file, not the other way round.
	for( int i = 1; i + 1 < argc; ++i ) {
		if( !strcmp( argv[ i ], "--load-setting" ) ) {
			if( !GetCfgFile( &set, argv[ i + 1 ] ) ) {
				fprintf( stderr, "bmp2msx-cli: cannot load setting file: %s\n", argv[ i + 1 ] );
				return 1;
			}
			set.PreView = 1;
		}
	}

	for( int i = 1; i < argc; ++i ) {
		std::string arg = argv[ i ];
		auto need_value = [ & ]( const char* opt, const char*& value ) -> bool {
			if( i + 1 >= argc ) {
				fprintf( stderr, "bmp2msx-cli: option %s needs a value\n", opt );
				exit( 1 );
			}
			value = argv[ ++i ];
			return true;
		};
		const char* v = nullptr;
		bool onoff = false;
		int num = 0, r = 0, g = 0, b = 0;

		if( arg == "--help" || arg == "-h" ) { usage( argv[ 0 ] ); return 0; }
		else if( arg == "--script" ) { need_value( "--script", v ); script_path = v; }
		else if( arg == "--input" ) { need_value( "--input", v ); input_path = v; }
		else if( arg == "--output-dir" ) { need_value( "--output-dir", v ); output_dir = v; }
		else if( arg == "--load-setting" ) { need_value( "--load-setting", v ); /* applied in the pre-scan */ }
		else if( arg == "--show-settings" ) { show_settings = true; }
		else if( arg == "--screen" ) {
			need_value( "--screen", v );
			int m = screen_mode_from_name( v );
			if( m < 0 ) { fprintf( stderr, "bmp2msx-cli: unknown screen mode: %s\n", v ); return 1; }
			set.screen_mode = (uint32_t) m;
		}
		else if( arg == "--palette-output" ) {
			need_value( "--palette-output", v );
			if( !strcmp( v, "bsave" ) ) set.PltMode = PLT_BSAVE;
			else if( !strcmp( v, "pl" ) ) set.PltMode = PLT_PL;
			else if( !strcmp( v, "csv" ) ) set.PltMode = PLT_CSV;
			else if( !strcmp( v, "none" ) ) set.PltMode = PLT_NONE;
			else { fprintf( stderr, "bmp2msx-cli: unknown palette output: %s\n", v ); return 1; }
		}
		else if( arg == "--basic" ) { set.output_basic_enable = true; }
		else if( arg == "--no-basic" ) { set.output_basic_enable = false; }
		else if( arg == "--interlace" ) { need_value( "--interlace", v ); if( !parse_onoff( v, onoff ) ) return 1; set.interlace = onoff; }
		else if( arg == "--lines-192" ) { need_value( "--lines-192", v ); if( !parse_onoff( v, onoff ) ) return 1; set.b192 = onoff; }
		else if( arg == "--resize" ) { need_value( "--resize", v ); if( !parse_onoff( v, onoff ) ) return 1; set.resize_enable = onoff; }
		else if( arg == "--resample" ) {
			need_value( "--resample", v );
			if( !strcmp( v, "normal" ) ) set.Resample = RS_NORMAL;
			else if( !strcmp( v, "antialias" ) ) set.Resample = RS_ANTIALIAS;
			else if( !strcmp( v, "copy" ) ) set.Resample = 2;	// any other value takes the copy path, as in the GUI switch
			else { fprintf( stderr, "bmp2msx-cli: unknown resample: %s\n", v ); return 1; }
		}
		else if( arg == "--size-mode" ) {
			need_value( "--size-mode", v );
			if( !strcmp( v, "force" ) ) set.SizeMode = RM_FORCE;
			else if( !strcmp( v, "msx" ) ) set.SizeMode = RM_MSX;
			else { fprintf( stderr, "bmp2msx-cli: unknown size mode: %s\n", v ); return 1; }
		}
		else if( arg == "--diffusion" ) { need_value( "--diffusion", v ); if( !parse_onoff( v, onoff ) ) return 1; set.diffusion_error_enable = onoff; }
		else if( arg == "--diffusion-coef" ) {
			need_value( "--diffusion-coef", v );
			if( !parse_int( v, num ) || num < 0 || num > 500 ) { fprintf( stderr, "bmp2msx-cli: bad --diffusion-coef\n" ); return 1; }
			set.diffusion_error_coef = (float) num / 1000.0f;
		}
		else if( arg == "--diffusion-x-weight" ) {
			need_value( "--diffusion-x-weight", v );
			double d = strtod( v, nullptr );
			if( d < 0.0 || d > 1.0 ) { fprintf( stderr, "bmp2msx-cli: bad --diffusion-x-weight\n" ); return 1; }
			set.diffusion_error_x_weight = (float) d;
		}
		else if( arg == "--dither" ) {
			need_value( "--dither", v );
			if( !strcmp( v, "none" ) ) set.ErrAlgo = EALGO_NONE;
			else if( v[ 0 ] == 'd' && !strncmp( v, "dith", 4 ) && v[ 4 ] >= '1' && v[ 4 ] <= '8' && !v[ 5 ] ) set.ErrAlgo = (uint32_t)( v[ 4 ] - '0' );
			else { fprintf( stderr, "bmp2msx-cli: unknown dither: %s\n", v ); return 1; }
		}
		else if( arg == "--dither-add" ) {
			need_value( "--dither-add", v );
			if( !strcmp( v, "rotate" ) ) set.ErrAdd = EADD_ROTATE;
			else if( !strcmp( v, "none" ) ) set.ErrAdd = EADD_NONE;
			else if( !strcmp( v, "x" ) ) set.ErrAdd = EADD_X;
			else if( !strcmp( v, "y" ) ) set.ErrAdd = EADD_Y;
			else { fprintf( stderr, "bmp2msx-cli: unknown dither-add: %s\n", v ); return 1; }
		}
		else if( arg == "--cuterr" ) {
			need_value( "--cuterr", v );
			if( !parse_int( v, num ) || num < 0 || num > 255 ) { fprintf( stderr, "bmp2msx-cli: bad --cuterr\n" ); return 1; }
			set.err = (uint32_t) num;
		}
		else if( arg == "--selcol" ) {
			need_value( "--selcol", v );
			if( !strcmp( v, "scatter" ) ) set.SelCol = SELCOL_BUNSAN;
			else if( !strcmp( v, "max" ) ) set.SelCol = SELCOL_TABUNPU;
			else { fprintf( stderr, "bmp2msx-cli: unknown selcol: %s\n", v ); return 1; }
		}
		else if( arg == "--algo" ) {
			need_value( "--algo", v );
			if( !strcmp( v, "none" ) ) set.AlgoMode = ALGO_NONE;
			else if( !strcmp( v, "recalc" ) ) set.AlgoMode = ALGO_RECALC;
			else if( !strcmp( v, "powerful" ) ) set.AlgoMode = ALGO_POWERFUL;
			else if( !strcmp( v, "ntsc2" ) ) set.AlgoMode = ALGO_NTSC2;
			else if( !strcmp( v, "color" ) ) set.AlgoMode = ALGO_RECALC2;
			else { fprintf( stderr, "bmp2msx-cli: unknown algo: %s\n", v ); return 1; }
		}
		else if( arg == "--color-recalc" ) { need_value( "--color-recalc", v ); if( !parse_onoff( v, onoff ) ) return 1; set.JKrc = onoff; }
		else if( arg == "--accuracy" ) {
			need_value( "--accuracy", v );
			if( !parse_int( v, num ) ) return 1;
			switch( num ) {
			case 24: set.Seido = SEIDO_24; break;
			case 21: set.Seido = SEIDO_21; break;
			case 18: set.Seido = SEIDO_18; break;
			case 15: set.Seido = SEIDO_15; break;
			case 12: set.Seido = SEIDO_12; break;
			default: fprintf( stderr, "bmp2msx-cli: bad --accuracy\n" ); return 1;
			}
		}
		else if( arg == "--fixed-palette" ) { need_value( "--fixed-palette", v ); if( !parse_onoff( v, onoff ) ) return 1; set.fixed_palette = onoff; }
		else if( arg == "--epal-hguard" ) { need_value( "--epal-hguard", v ); if( !parse_onoff( v, onoff ) ) return 1; set.EpalHGuard = onoff; }
		else if( arg == "--sc2-pair" ) { need_value( "--sc2-pair", v ); if( !parse_onoff( v, onoff ) ) return 1; set.Sc2Pair = onoff; }
		else if( arg == "--palette-preset" ) {
			need_value( "--palette-preset", v );
			if( !strcmp( v, "msx1" ) ) set_msx1_palette( set.color_palette );
			else if( !strcmp( v, "msx2" ) ) set_msx2_palette( set.color_palette );
			else { fprintf( stderr, "bmp2msx-cli: unknown palette preset: %s\n", v ); return 1; }
		}
		else if( arg == "--palette" ) {
			need_value( "--palette", v );
			int idx;
			if( sscanf( v, "%d=%d,%d,%d", &idx, &r, &g, &b ) != 4 || idx < 0 || idx > 15 || r < 0 || r > 7 || g < 0 || g > 7 || b < 0 || b > 7 ) {
				fprintf( stderr, "bmp2msx-cli: bad --palette (want n=r,g,b with values 0..7)\n" );
				return 1;
			}
			set.color_palette[ idx ].red = r;
			set.color_palette[ idx ].green = g;
			set.color_palette[ idx ].blue = b;
		}
		else if( arg == "--palette-mode" ) {
			need_value( "--palette-mode", v );
			int idx; char mode_name[ 16 ];
			if( sscanf( v, "%d=%15s", &idx, mode_name ) != 2 || idx < 0 || idx > 15 ) {
				fprintf( stderr, "bmp2msx-cli: bad --palette-mode\n" );
				return 1;
			}
			if( !strcmp( mode_name, "auto" ) ) set.PalEn[ idx ] = PALEN_AUTO;
			else if( !strcmp( mode_name, "nouse" ) ) set.PalEn[ idx ] = PALEN_NONE;
			else if( !strcmp( mode_name, "custom" ) ) set.PalEn[ idx ] = PALEN_USE;
			else { fprintf( stderr, "bmp2msx-cli: bad --palette-mode value\n" ); return 1; }
		}
		else if( arg == "--nonzero" ) { need_value( "--nonzero", v ); if( !parse_onoff( v, onoff ) ) return 1; set.NonZero = onoff; }
		else if( arg == "--tile" ) { need_value( "--tile", v ); if( !parse_onoff( v, onoff ) ) return 1; set.Tile = onoff; }
		else if( arg == "--sort" ) {
			need_value( "--sort", v );
			if( !strcmp( v, "ignore" ) ) set.SortMode = SM_IGNORE;
			else if( !strcmp( v, "inc-auto" ) ) set.SortMode = SM_INCAUTO;
			else if( !strcmp( v, "dec-auto" ) ) set.SortMode = SM_DECAUTO;
			else if( !strcmp( v, "inc" ) ) set.SortMode = SM_INC;
			else if( !strcmp( v, "dec" ) ) set.SortMode = SM_DEC;
			else { fprintf( stderr, "bmp2msx-cli: unknown sort: %s\n", v ); return 1; }
		}
		else if( arg == "--force-zero" ) {
			need_value( "--force-zero", v );
			if( !strcmp( v, "none" ) ) set.FourceZero = FZ_NONE;
			else if( !strcmp( v, "color" ) ) set.FourceZero = FZ_COLOR;
			else if( !strcmp( v, "matrix" ) ) set.FourceZero = FZ_MATRIX;
			else { fprintf( stderr, "bmp2msx-cli: unknown force-zero: %s\n", v ); return 1; }
		}
		else if( arg == "--fz-color" ) {
			need_value( "--fz-color", v );
			if( !parse_rgb( v, 255, r, g, b ) ) { fprintf( stderr, "bmp2msx-cli: bad --fz-color\n" ); return 1; }
			set.FZColor = GET_RGB( r, g, b );
		}
		else if( arg == "--fz-pos" ) {
			need_value( "--fz-pos", v );
			int x, y;
			if( sscanf( v, "%d,%d", &x, &y ) != 2 || x < 0 || y < 0 ) { fprintf( stderr, "bmp2msx-cli: bad --fz-pos\n" ); return 1; }
			set.FZX = (uint32_t) x;
			set.FZY = (uint32_t) y;
		}
		else if( arg == "--bg" ) {
			need_value( "--bg", v );
			if( !parse_rgb( v, 255, r, g, b ) ) { fprintf( stderr, "bmp2msx-cli: bad --bg\n" ); return 1; }
			set.FCColor = GET_RGB( r, g, b );
		}
		else if( arg.size() && arg[ 0 ] == '-' ) {
			fprintf( stderr, "bmp2msx-cli: unknown option: %s\n", arg.c_str() );
			return 1;
		}
		else {
			if( !input_path.empty() ) {
				fprintf( stderr, "bmp2msx-cli: multiple input files\n" );
				return 1;
			}
			input_path = arg;
		}
	}

	if( show_settings ) {
		printf( "%s\n", cli_describe_setting( set ).c_str() );
		return 0;
	}

	if( !script_path.empty() ) {
		return cli_run_script( script_path.c_str(), set, input_path, output_dir );
	}

	if( input_path.empty() ) {
		usage( argv[ 0 ] );
		return 1;
	}
	std::string err;
	if( !cli_convert_file( set, input_path, output_dir, &err ) ) {
		fprintf( stderr, "bmp2msx-cli: %s\n", err.c_str() );
		return 2;
	}
	return 0;
}
