// See gui_convert.h.  CreatePal()/GetPix() and the resize/recolor flow
// are transcribed from WndProc.cpp exactly as in ../src/convert_run.cpp
// (which see for the line-by-line correspondence); the GUI parts that
// convert_run.cpp drops (preview storage) are kept here instead.
#include "gui_convert.h"

#include <cstdlib>
#include <cstring>

#include "bsave.h"
#include "utils.h"
#include "image_load.h"

// platform_linux.cpp: queue the name GetSaveFileName() should return.
void cli_set_save_filename( const char* filename );

static bool prog_quiet( int ) { return true; }

// WndProc.cpp: GetPix (verbatim; used only for FourceZero == FZ_MATRIX)
static inline C_COLOR GetPix( C_COLOR* in, int width, int height, int x, int y ) {
	if( x < 0 ) x = 0;
	if( y < 0 ) y = 0;
	if( x >= width ) x = width - 1;
	if( y >= height ) y = height - 1;
	return( *( in + x + ( height - y ) * width ) );
}

// ---------------------------------------------------------------------------
// EPAL (SCREEN2/3/5/6/7) palette conversion helpers.
// The 3-bit (color_palette) <-> 5-bit (color_palette_epal) relationship is
// the one CreatePal has always maintained:
//   3-bit to 5-bit: ( convert7to255[v] * 31 + 127 ) / 255
//   5-bit to 3-bit: convert_rgb_to_palette over the cnvEpalTo255 expansion
// ---------------------------------------------------------------------------
static int EpalDerive5( const int *tbl, int v3 ) {
	return ( tbl[ v3 & 7 ] * 31 + 127 ) / 255;
}

static int EpalReflect3( const int *tbl, int v5 ) {
	return convert_rgb_to_palette( tbl, 8, cnvEpalTo255( v5 ) );
}

// True when a channel is 5-bit authoritative, i.e. it was edited at 5-bit
// precision in the settings dialog: its 5-bit value differs from the value
// derived from the 3-bit side, while the 3-bit side equals the reflection
// of the 5-bit value.  Settings produced by earlier builds always derive
// the 5-bit side from the 3-bit side, so this is never true for them and
// their conversion results do not change.
static bool EpalIsEdited( const int *tbl, int v3, int v5 ) {
	return ( v5 & 31 ) != EpalDerive5( tbl, v3 ) && ( v3 & 7 ) == EpalReflect3( tbl, v5 );
}

// 8-bit value of a fixed palette color: edited channels expand their
// 5-bit value, everything else expands the 3-bit value as before.
static int EpalFixedPal8( const int *tbl, int v3, int v5 ) {
	if( EpalIsEdited( tbl, v3, v5 ) ) return cnvEpalTo255( v5 );
	return tbl[ v3 & 7 ];
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
	} else if( cnvIsEpalMode( Mode->screen_mode ) ) {
		// SCREEN2/3/5/6/7 EPAL (V9968): same palette structure as the
		// 16-color branch, but generated at RGB555 precision.
		C_COLOR AutoPal[ 16 ];
		int auto_count = 0;
		int k;
		bool keep_epal[ 16 ] = {};
		j = 0;
		pp = 0;
		pnum = cnvEpalColorCount( Mode->screen_mode );
		for( i = 0; i < pnum; ++i ) {
			if( Mode->NonZero && i == 0 ) continue;
			if( Mode->PalEn[ i ] == PALEN_NONE ) continue;
			++n;
			if( Mode->PalEn[ i ] == PALEN_USE ) {
				Pal[ pp ] = GET_RGB( EpalFixedPal8( convert7to255_r, Mode->color_palette[ i ].red,   Mode->color_palette_epal[ i ].red ),
									 EpalFixedPal8( convert7to255_g, Mode->color_palette[ i ].green, Mode->color_palette_epal[ i ].green ),
									 EpalFixedPal8( convert7to255_b, Mode->color_palette[ i ].blue,  Mode->color_palette_epal[ i ].blue ) );
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
			if( Mode->EpalHGuard ) {
				// Base selection is Wu + k-means (Convert.cpp); the
				// feature-color improvement runs below.
				cnvGetPaletteEpalWu( t, w, h, AutoPal, &auto_count, pnum, Pal, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			} else {
				cnvGetPaletteEpalN( t, w, h, AutoPal, &auto_count, pnum, Pal, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			}
			if( Mode->EpalHGuard && auto_count > 0 ) {
				// Feature-color improvement: proposals in the h domain,
				// acceptance by the true-SSE guard (Convert.cpp).
				int pal5[ 16 ][ 3 ];
				int fixed5[ 16 ][ 3 ];
				int i2;
				for( i2 = 0; i2 < pp; ++i2 ) {
					cnvEpalColorTo5bit( Pal[ i2 ], &fixed5[ i2 ][ 0 ], &fixed5[ i2 ][ 1 ], &fixed5[ i2 ][ 2 ] );
				}
				for( i2 = 0; i2 < auto_count; ++i2 ) {
					cnvEpalColorTo5bit( AutoPal[ i2 ], &pal5[ i2 ][ 0 ], &pal5[ i2 ][ 1 ], &pal5[ i2 ][ 2 ] );
				}
				cnvEpalImprovePalette( t, w, h, pal5, auto_count, (const int (*)[ 3 ])fixed5, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
				for( i2 = 0; i2 < auto_count; ++i2 ) {
					AutoPal[ i2 ] = GET_RGB( cnvEpalTo255( pal5[ i2 ][ 0 ] ), cnvEpalTo255( pal5[ i2 ][ 1 ] ), cnvEpalTo255( pal5[ i2 ][ 2 ] ) );
				}
			}
			free( t );
			k = pp;
			for( i = 0; i < auto_count; ++i ) {
				Pal[ k++ ] = AutoPal[ i ];
			}
			j = pp;
			for( i = 0; i < pnum; ++i ) {
				if( Mode->NonZero && i == 0 ) continue;
				if( Mode->PalEn[ i ] == PALEN_AUTO ) {
					Mode->color_palette[ i ].red   = convert_rgb_to_palette( convert7to255_r, 8, GET_RED( Pal[ j ] ) );
					Mode->color_palette[ i ].green = convert_rgb_to_palette( convert7to255_g, 8, GET_GREEN( Pal[ j ] ) );
					Mode->color_palette[ i ].blue  = convert_rgb_to_palette( convert7to255_b, 8, GET_BLUE( Pal[ j ] ) );
					// EPAL records AUTO entries at full 5-bit precision
					// (author ruling 2026-11-07: snap to 5-bit, not 3-bit;
					// the final loop honors keep_epal).
					Mode->color_palette_epal[ i ].red   = ( GET_RED(   Pal[ j ] ) * 31 + 127 ) / 255;
					Mode->color_palette_epal[ i ].green = ( GET_GREEN( Pal[ j ] ) * 31 + 127 ) / 255;
					Mode->color_palette_epal[ i ].blue  = ( GET_BLUE(  Pal[ j ] ) * 31 + 127 ) / 255;
					keep_epal[ i ] = true;
					++j;
				}
			}
		}
		n = pnum;
		for( i = 0; i < pnum; ++i ) {
			n = 256;
			bool user_spec = Mode->fixed_palette || Mode->PalEn[ i ] == PALEN_USE;
			if( !keep_epal[ i ] && !( user_spec && EpalIsEdited( convert7to255_r, Mode->color_palette[ i ].red, Mode->color_palette_epal[ i ].red ) ) ) {
				Mode->color_palette_epal[ i ].red   = EpalDerive5( convert7to255_r, Mode->color_palette[ i ].red );
			}
			if( !keep_epal[ i ] && !( user_spec && EpalIsEdited( convert7to255_g, Mode->color_palette[ i ].green, Mode->color_palette_epal[ i ].green ) ) ) {
				Mode->color_palette_epal[ i ].green = EpalDerive5( convert7to255_g, Mode->color_palette[ i ].green );
			}
			if( !keep_epal[ i ] && !( user_spec && EpalIsEdited( convert7to255_b, Mode->color_palette[ i ].blue, Mode->color_palette_epal[ i ].blue ) ) ) {
				Mode->color_palette_epal[ i ].blue  = EpalDerive5( convert7to255_b, Mode->color_palette[ i ].blue );
			}
			Pal[ i ] = GET_RGB( cnvEpalTo255( Mode->color_palette_epal[ i ].red ),
								cnvEpalTo255( Mode->color_palette_epal[ i ].green ),
								cnvEpalTo255( Mode->color_palette_epal[ i ].blue ) );
		}
	} else if( Mode->screen_mode == MD_SC8P || Mode->screen_mode == MD_SC8P_256L ) {
		int pal_count = 0;
		t = (C_COLOR*) malloc( sizeof( C_COLOR ) * w * h );
		if( t == NULL ) {
			if( err ) *err = "out of memory";
			return false;
		}
		memcpy( t, in, w * h * sizeof( C_COLOR ) );
		if( Mode->EpalHGuard ) {
			// Base selection is Wu + k-means (Convert.cpp).
			cnvGetPaletteEpalWu( t, w, h, Pal, &pal_count, 256, NULL, 0, Mode->FourceZero != FZ_NONE, Mode->FZColor );
		} else {
			cnvGetPalette256( t, w, h, Pal, &pal_count, Mode->FourceZero != FZ_NONE, Mode->FZColor );
		}
		if( Mode->EpalHGuard && pal_count > 0 ) {
			// Feature-color improvement (same core as EPAL-N; no fixed colors).
			int pal5[ 256 ][ 3 ];
			int i2;
			for( i2 = 0; i2 < pal_count; ++i2 ) {
				cnvEpalColorTo5bit( Pal[ i2 ], &pal5[ i2 ][ 0 ], &pal5[ i2 ][ 1 ], &pal5[ i2 ][ 2 ] );
			}
			cnvEpalImprovePalette( t, w, h, pal5, pal_count, NULL, 0, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			for( i2 = 0; i2 < pal_count; ++i2 ) {
				Pal[ i2 ] = GET_RGB( cnvEpalTo255( pal5[ i2 ][ 0 ] ), cnvEpalTo255( pal5[ i2 ][ 1 ] ), cnvEpalTo255( pal5[ i2 ][ 2 ] ) );
			}
		}
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

bool gui_convert_image( const SETTING& mode_in, const std::string& input_path,
						GuiPreview& result, std::string* err ) {
	SETTING Mode = mode_in;

	static C_COLOR Pal[ 256 ];
	static C_TILE_PATTERN tail[ TAILMAX ];
	C_COLOR *in = nullptr, *out = nullptr;
	LPBYTE bmp = nullptr;
	int tailcnt = 0;
	int width = 0, height = 0, owidth = 0, oheight = 0, wwidth = 0, wheight = 0, n = 0;
	bool ok = false;

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
	free( dib );

	result.original.assign( in, in + (size_t) width * height );
	result.orig_w = width;
	result.orig_h = height;

	if( ( width < 8 || height < 8 ) && Mode.resize_enable ) {
		if( err ) *err = "image is smaller than 8x8";
		goto done;
	}
	if( Mode.FourceZero == FZ_MATRIX ) {
		Mode.FZColor = GetPix( in, width, height, Mode.FZX, Mode.FZY );
	}
	owidth = screen_size[ Mode.screen_mode ].w;
	if( ( screen_mode[ Mode.screen_mode ] >= 5 ) && Mode.b192 ) {
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
			if( ( Mode.screen_mode == MD_SC6 ) || ( Mode.screen_mode == MD_SC7 ) || ( Mode.screen_mode == MD_SC6_256L ) || ( Mode.screen_mode == MD_SC7_256L ) || ( Mode.screen_mode == MD_SC6P ) || ( Mode.screen_mode == MD_SC7P ) || ( Mode.screen_mode == MD_SC6P_256L ) || ( Mode.screen_mode == MD_SC7P_256L ) ) {
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
			if( Mode.interlace && Mode.screen_mode != MD_SC6 && Mode.screen_mode != MD_SC7 && Mode.screen_mode != MD_SC6_256L && Mode.screen_mode != MD_SC7_256L && Mode.screen_mode != MD_SC6P && Mode.screen_mode != MD_SC7P && Mode.screen_mode != MD_SC6P_256L && Mode.screen_mode != MD_SC7P_256L ) {
				cnvCopyInter( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
			} else {
				cnvCopy( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
			}
		}
	} else {
		if( Mode.interlace && Mode.screen_mode != MD_SC6 && Mode.screen_mode != MD_SC7 && Mode.screen_mode != MD_SC6_256L && Mode.screen_mode != MD_SC7_256L && Mode.screen_mode != MD_SC6P && Mode.screen_mode != MD_SC7P && Mode.screen_mode != MD_SC6P_256L && Mode.screen_mode != MD_SC7P_256L ) {
			cnvCopyInter( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
		} else {
			cnvCopy( in, width, height, out, owidth, oheight, prog_quiet, Mode.Seido );
		}
	}
	free( in );
	in = nullptr;

	result.source.assign( out, out + (size_t) owidth * oheight );
	result.source_w = owidth;
	result.source_h = oheight;

	if( !CreatePal( n, &Mode, out, owidth, oheight, Pal, err ) ) {
		goto done;
	}
	if( Mode.screen_mode < MD_SC8 || ( Mode.screen_mode >= MD_SC5_256L && Mode.screen_mode < MD_SC8_256L ) || cnvIsEpalMode( Mode.screen_mode ) ) {
		if( !( Mode.fixed_palette && ( Mode.SortMode == SM_INCAUTO ) ) ) {
			cnvSortPalette( &Mode, Pal );
		}
	}
	if( Mode.Tile ) {
		switch( Mode.screen_mode ) {
		case MD_SC2: case MD_SC3: case MD_SC5: case MD_SC6: case MD_SC7:
		case MD_SC5_256L: case MD_SC6_256L: case MD_SC7_256L:
		case MD_SC2P: case MD_SC3P: case MD_SC5P: case MD_SC6P: case MD_SC7P:
		case MD_SC5P_256L: case MD_SC6P_256L: case MD_SC7P_256L:
			tailcnt = cnvCreateTail4( Mode.color_palette, Mode.PalEn, !Mode.NonZero, tail, Mode.screen_mode );
			break;
		}
	}
	bmp = (LPBYTE) calloc( 1, 0x20000 );	// 128KB VRAM image (zero-filled)
	if( bmp == NULL ) {
		if( err ) *err = "out of memory";
		goto done;
	}
	if( Mode.screen_mode < MD_SC10 || ( Mode.screen_mode >= MD_SC5_256L && Mode.screen_mode < MD_SC10_256L ) || Mode.screen_mode == MD_SC8P || Mode.screen_mode == MD_SC8P_256L || cnvIsEpalMode( Mode.screen_mode ) ) {
		cnvRecolor( out, owidth, oheight, bmp, &Mode, prog_quiet, Pal, tail, tailcnt );
	} else {
		cnvNtcolor( out, owidth, oheight, bmp, &Mode, prog_quiet );
	}
	free( out );
	out = nullptr;

	result.vram.assign( bmp, bmp + 0x20000 );
	if( n == 16 ) owidth /= 2; else if( n == 4 ) owidth /= 4;
	result.save_w = owidth;
	result.save_h = oheight;
	result.mode = Mode;
	result.input_path = input_path;
	ok = true;
done:
	free( in );
	free( out );
	free( bmp );
	return ok;
}

// Palette file digit, following the same GraphSaurus convention as
// ../src/convert_run.cpp.
static char palette_digit( uint32_t screen_mode ) {
	switch( screen_mode ) {
	case MD_SC2: case MD_SC2P: return '2';
	case MD_SC3: case MD_SC3P: return '3';
	case MD_SC5: case MD_SC6: case MD_SC5_256L: case MD_SC6_256L:
	case MD_SC5P: case MD_SC6P: case MD_SC5P_256L: case MD_SC6P_256L: return '5';
	default: return '7';
	}
}

static std::string path_stem( const std::string& path ) {
	size_t slash = path.find_last_of( '/' );
	std::string base = slash == std::string::npos ? path : path.substr( slash + 1 );
	size_t dot = base.find_last_of( '.' );
	if( dot != std::string::npos && dot != 0 ) base = base.substr( 0, dot );
	return base;
}

static std::string path_with_stem( const std::string& path ) {
	size_t slash = path.find_last_of( '/' );
	return slash == std::string::npos ? std::string() : path.substr( 0, slash + 1 );
}

std::string gui_derived_palette_name( const SETTING& mode,
									  const std::string& name_source ) {
	std::string base = path_with_stem( name_source ) + path_stem( name_source );
	if( mode.PltMode == PLT_PL ) {
		return base + ".PL" + palette_digit( mode.screen_mode );
	}
	if( mode.PltMode == PLT_CSV ) {
		return base + ".CSV";
	}
	return std::string();
}

int gui_save_preview_files( const GuiPreview& pv, const SETTING& mode_in,
							const std::string& name_source,
							const std::string& palette_file ) {
	SETTING Mode = mode_in;
	if( ( Mode.PltMode == PLT_PL || Mode.PltMode == PLT_CSV ) && !palette_file.empty() ) {
		cli_set_save_filename( palette_file.c_str() );
	} else {
		// No palette prompt answer queued: GetSaveFileName() reports
		// "cancelled" and bsvSaveBmp() skips the palette file.
		cli_set_save_filename( nullptr );
	}
	LPBYTE bmp = (LPBYTE) pv.vram.data();	// bsvSaveBmp does not write through it
	int ecode = bsvSaveBmp( nullptr, name_source.c_str(), bmp, pv.save_w, pv.save_h, &Mode );
	cli_set_save_filename( nullptr );
	return ecode;
}
