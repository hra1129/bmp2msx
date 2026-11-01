// See convert_run.h.  Function bodies follow WndProc.cpp (Converter,
// CreatePal, GetPix) statement by statement; comments mark the spots
// where GUI-only behavior was replaced by its headless equivalent.
#include "convert_run.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "bsave.h"
#include "utils.h"
#include "image_load.h"

// platform_linux.cpp: queue the name GetSaveFileName() should return.
void cli_set_save_filename( const char* filename );

static bool prog_quiet( int ) { return true; }

// WndProc.cpp: GetPix (transcribed verbatim, including its coordinate
// convention; used only for FourceZero == FZ_MATRIX)
static inline C_COLOR GetPix( C_COLOR* in, int width, int height, int x, int y ) {
	if( x < 0 ) x = 0;
	if( y < 0 ) y = 0;
	if( x >= width ) x = width - 1;
	if( y >= height ) y = height - 1;
	return( *( in + x + ( height - y ) * width ) );
}

// WndProc.cpp: CreatePal (MessageBox error reports replaced by `err`)
static bool CreatePal( int& n, SETTING* Mode, C_COLOR* in, int w, int h, C_COLOR* Pal, std::string* err ) {
	C_COLOR* t;
	int i, j, pp, pnum;

	n = 0;
	if( Mode->screen_mode < MD_SC8 || ( Mode->screen_mode >= MD_SC5_256L && Mode->screen_mode < MD_SC8_256L ) ) {
		j = 0;
		pp = 0;
		if( Mode->screen_mode == MD_SC6 || Mode->screen_mode == MD_SC6_256L ) pnum = 4; else pnum = 16;
		for( i = 0; i < pnum; ++i ) {
			if( Mode->NonZero && i == 0 ) continue;
			if( Mode->PalEn[ i ] == PALEN_NONE ) continue;
			++n;
			if( Mode->PalEn[ i ] == PALEN_USE ) {
				Pal[ pp ] = GET_RGB( convert7to255_r[ Mode->color_palette[ i ].red ],
									 convert7to255_g[ Mode->color_palette[ i ].green ],
									 convert7to255_b[ Mode->color_palette[ i ].blue ] );
				++pp;
			}
		}
		if( n < 2 ) {
			if( err ) *err = "fewer than 2 usable palette entries";
			return false;
		}
		if( !Mode->fixed_palette ) {
			t = (C_COLOR*) malloc( sizeof( C_COLOR ) * w * h );
			if( t == NULL ) {
				if( err ) *err = "out of memory";
				return false;
			}
			memcpy( t, in, w * h * sizeof( C_COLOR ) );
			cnvGetPalette( t, w, h, Pal, Mode->SelCol, n, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			free( t );
			j = pp;
			for( i = 0; i < pnum; ++i ) {
				if( Mode->NonZero && i == 0 ) continue;
				if( Mode->PalEn[ i ] == PALEN_AUTO ) {
					Mode->color_palette[ i ].red   = convert_rgb_to_palette( convert7to255_r, 8, GET_RED( Pal[ j ] ) );
					Mode->color_palette[ i ].green = convert_rgb_to_palette( convert7to255_g, 8, GET_GREEN( Pal[ j ] ) );
					Mode->color_palette[ i ].blue  = convert_rgb_to_palette( convert7to255_b, 8, GET_BLUE( Pal[ j ] ) );
					++j;
				}
			}
		}
		n = pnum;
		for( i = 0; i < pnum; ++i ) {
			n = 256;
			Pal[ i ] = GET_RGB( convert7to255_r[ Mode->color_palette[ i ].red ],
								convert7to255_g[ Mode->color_palette[ i ].green ],
								convert7to255_b[ Mode->color_palette[ i ].blue ] );
		}
	} else if( Mode->screen_mode == MD_SC8P || Mode->screen_mode == MD_SC8P_256L ) {
		int pal_count = 0;
		t = (C_COLOR*) malloc( sizeof( C_COLOR ) * w * h );
		if( t == NULL ) {
			if( err ) *err = "out of memory";
			return false;
		}
		memcpy( t, in, w * h * sizeof( C_COLOR ) );
		cnvGetPalette256( t, w, h, Pal, &pal_count, Mode->FourceZero != FZ_NONE, Mode->FZColor );
		free( t );
		for( i = pal_count; i < 256; ++i ) {
			Pal[ i ] = 0;
		}
		for( i = 0; i < 256; ++i ) {
			Mode->color_palette_epal[ i ].red   = ( GET_RED(   Pal[ i ] ) * 31 + 127 ) / 255;
			Mode->color_palette_epal[ i ].green = ( GET_GREEN( Pal[ i ] ) * 31 + 127 ) / 255;
			Mode->color_palette_epal[ i ].blue  = ( GET_BLUE(  Pal[ i ] ) * 31 + 127 ) / 255;
		}
	} else if( Mode->screen_mode == MD_SC8 || Mode->screen_mode == MD_SC8_256L ) {
		cnvGetPaletteS8( Pal );
	}
	return true;
}

static std::string path_dir( const std::string& path ) {
	size_t pos = path.find_last_of( '/' );
	return pos == std::string::npos ? std::string() : path.substr( 0, pos + 1 );
}

static std::string path_stem( const std::string& path ) {
	size_t slash = path.find_last_of( '/' );
	std::string base = slash == std::string::npos ? path : path.substr( slash + 1 );
	size_t dot = base.find_last_of( '.' );
	if( dot != std::string::npos && dot != 0 ) base = base.substr( 0, dot );
	return base;
}

// Palette file name the GUI would have asked for; the digit follows the
// GraphSaurus convention (PL5 for the SCREEN5/6 family, PL7 for the
// SCREEN7/10 family, PL2/PL3 for SCREEN2/3).
static char palette_digit( uint32_t screen_mode ) {
	switch( screen_mode ) {
	case MD_SC2: return '2';
	case MD_SC3: return '3';
	case MD_SC5: case MD_SC6: case MD_SC5_256L: case MD_SC6_256L: return '5';
	default: return '7';
	}
}

bool cli_convert_file( const SETTING& mode_in, const std::string& input_path,
					   const std::string& output_dir, std::string* err ) {
	SETTING Mode = mode_in;
	Mode.PreView = 1;	// file output only (no preview exists headless)
	Mode.AutoName = true;	// names are always derived; there is no save dialog

	static C_COLOR Pal[ 256 ];
	static C_TILE_PATTERN tail[ TAILMAX ];
	C_COLOR *in = nullptr, *out = nullptr;
	LPBYTE bmp = nullptr;
	int tailcnt = 0;
	int width = 0, height = 0, owidth = 0, oheight = 0, wwidth = 0, wheight = 0, n = 0;
	int ecode = BSV_NOERR;
	bool ok = false;

	// Load the image (counterpart of bdAddFile + GetBmp in the GUI)
	char* dib = load_image_dib( input_path.c_str() );
	if( !dib ) {
		if( err ) *err = "cannot load image: " + input_path;
		return false;
	}
	{
		char* ptr = dib;
		if( !GetBmp( &in, &width, &height, &ptr ) ) {
			if( err ) *err = "unsupported image format: " + input_path;
			free( dib );
			return false;
		}
	}
	// GetBmp() allocated `in` with LocalAlloc (= malloc here); the pixel
	// buffer `dib` itself is no longer needed once decoded, except that
	// GetBmp*() read from it during the call above.
	free( dib );

	if( ( width < 8 || height < 8 ) && Mode.resize_enable ) {
		if( err ) *err = "image is smaller than 8x8";
		goto done;
	}
	if( Mode.FourceZero == FZ_MATRIX ) {
		Mode.FZColor = GetPix( in, width, height, Mode.FZX, Mode.FZY );
	}
	owidth = screen_size[ Mode.screen_mode ].w;
	if( ( Mode.screen_mode >= MD_SC5 ) && Mode.b192 ) {
		oheight = 192;
	} else {
		oheight = screen_size[ Mode.screen_mode ].h;
	}
	oheight = oheight * ( Mode.interlace ? 2 : 1 );
	out = (C_COLOR*) malloc( sizeof( C_COLOR ) * owidth * oheight );
	if( out == NULL ) {
		if( err ) *err = "out of memory";
		goto done;
	}
	for( int i = 0; i < owidth * oheight; ++i ) {
		out[ i ] = Mode.FCColor;
	}
	if( Mode.resize_enable ) {
		switch( Mode.SizeMode ) {
		case RM_MSX:
			if( ( Mode.screen_mode == MD_SC6 ) || ( Mode.screen_mode == MD_SC7 ) || ( Mode.screen_mode == MD_SC6_256L ) || ( Mode.screen_mode == MD_SC7_256L ) ) {
				wwidth  = width * 5 / 3;
				wheight = height;
			} else {
				wwidth  = width;
				wheight = height * 6 / 5;
			}
			if( Mode.interlace ) {
				wheight = wheight * 2;
			}
			if( wwidth > owidth ) {
				wheight = wheight * owidth / wwidth;
				wwidth  = owidth;
			}
			if( wheight > oheight ) {
				wwidth  = wwidth * oheight / wheight;
				wheight = oheight;
			}
			break;
		default:
			wwidth  = owidth;
			wheight = oheight;
			break;
		}
		switch( Mode.Resample ) {
		case RS_NORMAL:
			cnvNormResize( in, width, height, out, owidth, oheight, wwidth, wheight, prog_quiet, Mode.Seido );
			break;
		case RS_ANTIALIAS:
			cnvAntiResize( in, width, height, out, owidth, oheight, wwidth, wheight, prog_quiet, Mode.Seido );
			break;
		default:
			if( Mode.interlace && Mode.screen_mode != MD_SC6 && Mode.screen_mode != MD_SC7 && Mode.screen_mode != MD_SC6_256L && Mode.screen_mode != MD_SC7_256L ) {
				cnvCopyInter( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
			} else {
				cnvCopy( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
			}
		}
	} else {
		if( Mode.interlace && Mode.screen_mode != MD_SC6 && Mode.screen_mode != MD_SC7 && Mode.screen_mode != MD_SC6_256L && Mode.screen_mode != MD_SC7_256L ) {
			cnvCopyInter( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
		} else {
			cnvCopy( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
		}
	}
	free( in );
	in = nullptr;

	if( !CreatePal( n, &Mode, out, owidth, oheight, Pal, err ) ) {
		goto done;
	}
	if( Mode.screen_mode < MD_SC8 || ( Mode.screen_mode >= MD_SC5_256L && Mode.screen_mode < MD_SC8_256L ) ) {
		if( !( Mode.fixed_palette && ( Mode.SortMode == SM_INCAUTO ) ) ) {
			cnvSortPalette( &Mode, Pal );
		}
	}
	if( Mode.Tile ) {
		switch( Mode.screen_mode ) {
		case MD_SC2: case MD_SC3: case MD_SC5: case MD_SC6: case MD_SC7:
		case MD_SC5_256L: case MD_SC6_256L: case MD_SC7_256L:
			tailcnt = cnvCreateTail4( Mode.color_palette, Mode.PalEn, !Mode.NonZero, tail, Mode.screen_mode );
			break;
		}
	}
	bmp = (LPBYTE) calloc( 1, 0x20000 );	// 128KB VRAM image (zero-filled)
	if( bmp == NULL ) {
		if( err ) *err = "out of memory";
		goto done;
	}
	if( Mode.screen_mode < MD_SC10 || ( Mode.screen_mode >= MD_SC5_256L && Mode.screen_mode < MD_SC10_256L ) || Mode.screen_mode == MD_SC8P || Mode.screen_mode == MD_SC8P_256L ) {
		cnvRecolor( out, owidth, oheight, bmp, &Mode, prog_quiet, Pal, tail, tailcnt );
	} else {
		cnvNtcolor( out, owidth, oheight, bmp, &Mode, prog_quiet );
	}
	free( out );
	out = nullptr;

	if( n == 16 ) owidth /= 2; else if( n == 4 ) owidth /= 4;
	{
		// The name bsvSaveBmp() derives output names from, and the
		// palette file name queued for its PL/CSV prompt.
		std::string out_base = ( output_dir.empty() ? path_dir( input_path ) : ( output_dir + "/" ) ) + path_stem( input_path );
		std::string name_source = out_base + ".bmp";
		if( Mode.PltMode == PLT_PL ) {
			cli_set_save_filename( ( out_base + ".PL" + palette_digit( Mode.screen_mode ) ).c_str() );
		} else if( Mode.PltMode == PLT_CSV ) {
			cli_set_save_filename( ( out_base + ".CSV" ).c_str() );
		} else {
			cli_set_save_filename( nullptr );
		}
		ecode = bsvSaveBmp( nullptr, name_source.c_str(), bmp, owidth, oheight, &Mode );
		cli_set_save_filename( nullptr );
	}
	if( ecode != BSV_NOERR ) {
		if( err ) *err = "failed to write output files";
		goto done;
	}
	ok = true;
done:
	free( in );
	free( out );
	free( bmp );
	return ok;
}

std::string cli_describe_setting( const SETTING& m ) {
	static const char* mode_names[] = {
		"SCREEN2", "SCREEN3", "SCREEN5", "SCREEN6", "SCREEN7", "SCREEN8",
		"SCREEN10", "SCREEN12", "SCREEN5 (256 lines)", "SCREEN6 (256 lines)",
		"SCREEN7 (256 lines)", "SCREEN8 (256 lines)", "SCREEN10 (256 lines)",
		"SCREEN12 (256 lines)", "SCREEN8 EPAL (V9968)", "SCREEN8 EPAL (V9968) (256 lines)"
	};
	char buf[ 1024 ];
	const char* mode_name = ( m.screen_mode < MD_MAX ) ? mode_names[ m.screen_mode ] : "?";
	snprintf( buf, sizeof( buf ),
		"screen_mode=%s diffusion=%s coef=%.3f x_weight=%.3f cuterr=%u interlace=%s "
		"fixed_palette=%s resize=%s resample=%d size_mode=%d selcol=%u pltmode=%u basic=%s "
		"algo=%u jk_rc=%s dither=%u dither_add=%u seido=%u nonzero=%s force_zero=%u tile=%s "
		"sort=%d lines192=%s",
		mode_name,
		m.diffusion_error_enable ? "on" : "off", m.diffusion_error_coef, m.diffusion_error_x_weight,
		m.err, m.interlace ? "on" : "off", m.fixed_palette ? "on" : "off",
		m.resize_enable ? "on" : "off", m.Resample, m.SizeMode, m.SelCol, m.PltMode,
		m.output_basic_enable ? "on" : "off", m.AlgoMode, m.JKrc ? "on" : "off",
		m.ErrAlgo, m.ErrAdd, m.Seido, m.NonZero ? "on" : "off", m.FourceZero,
		m.Tile ? "on" : "off", m.SortMode, m.b192 ? "on" : "off" );
	return buf;
}
