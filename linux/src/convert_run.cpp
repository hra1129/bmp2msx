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
#include "epal_refine.h"
#include "sc2_pair.h"

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
		// Experimental palette selection (epal_refine.h): completely
		// inert unless one of its env vars is set.  keep_epal[i] marks
		// AUTO entries whose 5-bit palette was recorded directly from
		// the (possibly refined) selection, so the final loop below
		// keeps it instead of re-deriving it from the 3-bit palette.
		bool keep_epal[ 16 ] = {};
		epal_maybe_histdump( in, w, h );
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
			// Experimental selection paths (epal_refine.h): completely
			// inert unless one of BMP2MSX_EPAL_REFINE / _ACCENT / _UVBONUS /
			// _HPBONUS / _SQRTHIST is set.  With HPBONUS the histogram
			// the median cut consumes is bonus-weighted first (the
			// author's scheme); with SQRTHIST it is the compressed-
			// domain histogram sqrt(n) + k * saliency mass; accent
			// reservation then displaces the cheapest entries, and
			// the swap refinement runs last.  Fixed-color bins and
			// FourceZero pixels are excluded exactly as in
			// cnvGetPaletteEpalN; fixed entries (and reserved accents)
			// join the nearest-entry assignment but are never removed.
			int pal5[ 16 ][ 3 ];
			int fixed5[ 16 ][ 3 ];
			int accent5[ 32 ][ 3 ];
			int nacc = 0;
			bool have_pal5 = false;
			double* histw = NULL;
			if( epal_sqrthist_enabled() || epal_hp_enabled() || epal_uv_enabled() ) {
				if( epal_sqrthist_enabled() ) {
					histw = epal_build_hist_sqrt555( t, w, h, Mode->FourceZero != FZ_NONE, Mode->FZColor );
				} else {
					double* pixw = epal_uv_enabled() ? epal_uv_pixel_weights( in, w, h )
													  : epal_hp_pixel_weights( in, w, h );
					histw = epal_build_histw555( t, w, h, Mode->FourceZero != FZ_NONE, Mode->FZColor, pixw );
					free( pixw );
				}
				if( histw != NULL ) {
					int i2;
					for( i2 = 0; i2 < pp; ++i2 ) {
						epal_color_to_5bit( Pal[ i2 ], &fixed5[ i2 ][ 0 ], &fixed5[ i2 ][ 1 ], &fixed5[ i2 ][ 2 ] );
						histw[ ( fixed5[ i2 ][ 0 ] << 10 ) | ( fixed5[ i2 ][ 1 ] << 5 ) | fixed5[ i2 ][ 2 ] ] = 0;
					}
					auto_count = epal_select_weighted( histw, pnum - pp, pal5 );
					have_pal5 = true;
					if( !epal_refine_enabled() ) { free( histw ); histw = NULL; }
				}
			}
			if( !have_pal5 ) {
				if( Mode->EpalHGuard ) {
					// Product base selection for EpalHGuard: Wu + k-means.
					cnvGetPaletteEpalWu( t, w, h, AutoPal, &auto_count, pnum, Pal, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
				} else {
					cnvGetPaletteEpalN( t, w, h, AutoPal, &auto_count, pnum, Pal, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
				}
			}
			if( epal_experiment_enabled() && auto_count > 0 ) {
				uint32_t* hist = NULL;
				if( epal_refine_enabled() || epal_accent_enabled() ) {
					hist = epal_build_hist555( t, w, h, Mode->FourceZero != FZ_NONE, Mode->FZColor );
					if( hist != NULL ) {
						int i2;
						for( i2 = 0; i2 < pp; ++i2 ) {
							epal_color_to_5bit( Pal[ i2 ], &fixed5[ i2 ][ 0 ], &fixed5[ i2 ][ 1 ], &fixed5[ i2 ][ 2 ] );
							hist[ ( fixed5[ i2 ][ 0 ] << 10 ) | ( fixed5[ i2 ][ 1 ] << 5 ) | fixed5[ i2 ][ 2 ] ] = 0;
						}
					}
				}
				if( !have_pal5 ) {
					int i2;
					for( i2 = 0; i2 < auto_count; ++i2 ) {
						epal_color_to_5bit( AutoPal[ i2 ], &pal5[ i2 ][ 0 ], &pal5[ i2 ][ 1 ], &pal5[ i2 ][ 2 ] );
					}
				}
				int npal_cur = auto_count;
				if( hist != NULL && epal_accent_enabled() ) {
					int want = epal_accent_k();
					if( want > 32 ) want = 32;
					nacc = epal_accent_reserve( hist, pal5, &npal_cur, (const int (*)[ 3 ]) fixed5, pp, accent5, want, "EPAL-N" );
				}
				if( epal_refine_enabled() && npal_cur > 0 ) {
					int fixedall[ 48 ][ 3 ];
					int nf = 0, i2;
					for( i2 = 0; i2 < pp; ++i2 ) {
						fixedall[ nf ][ 0 ] = fixed5[ i2 ][ 0 ];
						fixedall[ nf ][ 1 ] = fixed5[ i2 ][ 1 ];
						fixedall[ nf ][ 2 ] = fixed5[ i2 ][ 2 ];
						++nf;
					}
					for( i2 = 0; i2 < nacc; ++i2 ) {
						fixedall[ nf ][ 0 ] = accent5[ i2 ][ 0 ];
						fixedall[ nf ][ 1 ] = accent5[ i2 ][ 1 ];
						fixedall[ nf ][ 2 ] = accent5[ i2 ][ 2 ];
						++nf;
					}
					if( histw != NULL ) {
						epal_refine_palette_w( histw, pal5, npal_cur, (const int (*)[ 3 ]) fixedall, nf, "EPAL-N" );
					} else if( hist != NULL ) {
						epal_refine_palette( hist, pal5, npal_cur, (const int (*)[ 3 ]) fixedall, nf, "EPAL-N" );
					}
				}
				{
					int i2, k2 = 0;
					for( i2 = 0; i2 < npal_cur; ++i2 ) {
						AutoPal[ k2++ ] = GET_RGB( cnvEpalTo255( pal5[ i2 ][ 0 ] ), cnvEpalTo255( pal5[ i2 ][ 1 ] ), cnvEpalTo255( pal5[ i2 ][ 2 ] ) );
					}
					for( i2 = 0; i2 < nacc; ++i2 ) {
						AutoPal[ k2++ ] = GET_RGB( cnvEpalTo255( accent5[ i2 ][ 0 ] ), cnvEpalTo255( accent5[ i2 ][ 1 ] ), cnvEpalTo255( accent5[ i2 ][ 2 ] ) );
					}
				}
				free( hist );
			}
			if( Mode->EpalHGuard && auto_count > 0 ) {
				// Product feature-color improvement (Convert.cpp):
				// proposals and ranking in the h domain
				// h(c) = sqrt( n(c) ) + b(c), acceptance by the exact
				// true-SSE guard.  Runs on the median-cut result
				// (after any experimental path above); AUTO entries
				// are then recorded at full 5-bit precision.
				int hgpal5[ 16 ][ 3 ];
				int hgfixed5[ 16 ][ 3 ];
				int i2;
				for( i2 = 0; i2 < pp; ++i2 ) {
					cnvEpalColorTo5bit( Pal[ i2 ], &hgfixed5[ i2 ][ 0 ], &hgfixed5[ i2 ][ 1 ], &hgfixed5[ i2 ][ 2 ] );
				}
				for( i2 = 0; i2 < auto_count; ++i2 ) {
					cnvEpalColorTo5bit( AutoPal[ i2 ], &hgpal5[ i2 ][ 0 ], &hgpal5[ i2 ][ 1 ], &hgpal5[ i2 ][ 2 ] );
				}
				cnvEpalImprovePalette( t, w, h, hgpal5, auto_count, (const int (*)[ 3 ]) hgfixed5, pp, Mode->FourceZero != FZ_NONE, Mode->FZColor );
				for( i2 = 0; i2 < auto_count; ++i2 ) {
					AutoPal[ i2 ] = GET_RGB( cnvEpalTo255( hgpal5[ i2 ][ 0 ] ), cnvEpalTo255( hgpal5[ i2 ][ 1 ] ), cnvEpalTo255( hgpal5[ i2 ][ 2 ] ) );
				}
			}
			free( histw );
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
					// Record the selected color at full 5-bit
					// precision (the final loop honors keep_epal).
					epal_color_to_5bit( Pal[ j ], &Mode->color_palette_epal[ i ].red, &Mode->color_palette_epal[ i ].green, &Mode->color_palette_epal[ i ].blue );
					keep_epal[ i ] = true;
					++j;
				}
			}
		}
		n = pnum;
		for( i = 0; i < pnum; ++i ) {
			n = 256;
			bool user_spec = Mode->fixed_palette || Mode->PalEn[ i ] == PALEN_USE;
			if( !( user_spec && EpalIsEdited( convert7to255_r, Mode->color_palette[ i ].red, Mode->color_palette_epal[ i ].red ) ) && !keep_epal[ i ] ) {
				Mode->color_palette_epal[ i ].red   = EpalDerive5( convert7to255_r, Mode->color_palette[ i ].red );
			}
			if( !( user_spec && EpalIsEdited( convert7to255_g, Mode->color_palette[ i ].green, Mode->color_palette_epal[ i ].green ) ) && !keep_epal[ i ] ) {
				Mode->color_palette_epal[ i ].green = EpalDerive5( convert7to255_g, Mode->color_palette[ i ].green );
			}
			if( !( user_spec && EpalIsEdited( convert7to255_b, Mode->color_palette[ i ].blue, Mode->color_palette_epal[ i ].blue ) ) && !keep_epal[ i ] ) {
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
		// Experimental selection paths (epal_refine.h): completely
		// inert unless one of BMP2MSX_EPAL_REFINE / _ACCENT / _UVBONUS /
		// _HPBONUS / _SQRTHIST is set.  Same structure as the EPAL-N
		// branch; there are no fixed entries in this mode.
		int pal5[ 256 ][ 3 ];
		int accent5[ 32 ][ 3 ];
		int nacc = 0;
		bool have_pal5 = false;
		double* histw = NULL;
		if( epal_sqrthist_enabled() || epal_hp_enabled() || epal_uv_enabled() ) {
			if( epal_sqrthist_enabled() ) {
				histw = epal_build_hist_sqrt555( t, w, h, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			} else {
				double* pixw = epal_uv_enabled() ? epal_uv_pixel_weights( in, w, h )
												  : epal_hp_pixel_weights( in, w, h );
				histw = epal_build_histw555( t, w, h, Mode->FourceZero != FZ_NONE, Mode->FZColor, pixw );
				free( pixw );
			}
			if( histw != NULL ) {
				pal_count = epal_select_weighted( histw, 256, pal5 );
				have_pal5 = true;
				if( !epal_refine_enabled() ) { free( histw ); histw = NULL; }
			}
		}
		if( !have_pal5 ) {
			if( Mode->EpalHGuard ) {
				cnvGetPaletteEpalWu( t, w, h, Pal, &pal_count, 256, NULL, 0, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			} else {
				cnvGetPalette256( t, w, h, Pal, &pal_count, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			}
		}
		epal_maybe_histdump( in, w, h );
		if( epal_experiment_enabled() && pal_count > 0 ) {
			uint32_t* hist = NULL;
			if( epal_refine_enabled() || epal_accent_enabled() ) {
				hist = epal_build_hist555( t, w, h, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			}
			if( !have_pal5 ) {
				int i2;
				for( i2 = 0; i2 < pal_count; ++i2 ) {
					epal_color_to_5bit( Pal[ i2 ], &pal5[ i2 ][ 0 ], &pal5[ i2 ][ 1 ], &pal5[ i2 ][ 2 ] );
				}
			}
			int npal_cur = pal_count;
			if( hist != NULL && epal_accent_enabled() ) {
				int want = epal_accent_k();
				if( want > 32 ) want = 32;
				nacc = epal_accent_reserve( hist, pal5, &npal_cur, NULL, 0, accent5, want, "SC8P" );
			}
			if( epal_refine_enabled() && npal_cur > 0 ) {
				if( histw != NULL ) {
					epal_refine_palette_w( histw, pal5, npal_cur, (const int (*)[ 3 ]) accent5, nacc, "SC8P" );
				} else if( hist != NULL ) {
					epal_refine_palette( hist, pal5, npal_cur, (const int (*)[ 3 ]) accent5, nacc, "SC8P" );
				}
			}
			{
				int i2, k2 = 0;
				for( i2 = 0; i2 < npal_cur; ++i2 ) {
					Pal[ k2++ ] = GET_RGB( cnvEpalTo255( pal5[ i2 ][ 0 ] ), cnvEpalTo255( pal5[ i2 ][ 1 ] ), cnvEpalTo255( pal5[ i2 ][ 2 ] ) );
				}
				for( i2 = 0; i2 < nacc; ++i2 ) {
					Pal[ k2++ ] = GET_RGB( cnvEpalTo255( accent5[ i2 ][ 0 ] ), cnvEpalTo255( accent5[ i2 ][ 1 ] ), cnvEpalTo255( accent5[ i2 ][ 2 ] ) );
				}
			}
			free( hist );
		}
		if( Mode->EpalHGuard && pal_count > 0 ) {
			// Product feature-color improvement (same core as EPAL-N;
			// there are no fixed entries in this mode).
			int hgpal5[ 256 ][ 3 ];
			int i2;
			for( i2 = 0; i2 < pal_count; ++i2 ) {
				cnvEpalColorTo5bit( Pal[ i2 ], &hgpal5[ i2 ][ 0 ], &hgpal5[ i2 ][ 1 ], &hgpal5[ i2 ][ 2 ] );
			}
			cnvEpalImprovePalette( t, w, h, hgpal5, pal_count, NULL, 0, Mode->FourceZero != FZ_NONE, Mode->FZColor );
			for( i2 = 0; i2 < pal_count; ++i2 ) {
				Pal[ i2 ] = GET_RGB( cnvEpalTo255( hgpal5[ i2 ][ 0 ] ), cnvEpalTo255( hgpal5[ i2 ][ 1 ] ), cnvEpalTo255( hgpal5[ i2 ][ 2 ] ) );
			}
		}
		free( histw );
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
	case MD_SC2: case MD_SC2P: return '2';
	case MD_SC3: case MD_SC3P: return '3';
	case MD_SC5: case MD_SC6: case MD_SC5_256L: case MD_SC6_256L:
	case MD_SC5P: case MD_SC6P: case MD_SC5P_256L: case MD_SC6P_256L: return '5';
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
		// SCREEN2 line-pair prototype (sc2_pair.h): completely inert
		// unless BMP2MSX_SC2_PAIR and/or BMP2MSX_SC2_DUMP is set.  The
		// pair path replaces cnvRecolor() for MD_SC2 / MD_SC2P at
		// 256x192 without Tile or interlace; anything else (and every
		// non-SC2 mode) takes the product path unchanged.
		bool is_sc2 = ( Mode.screen_mode == MD_SC2 || Mode.screen_mode == MD_SC2P );
		bool pair_path = is_sc2 && sc2_pair_env_set() && !Mode.Tile && !Mode.interlace
						 && owidth == 256 && oheight == 192;
		if( is_sc2 && sc2_dump_env_set() ) {
			sc2_dump_before( Mode, out, owidth, oheight, Pal );
		}
		if( pair_path ) {
			if( !sc2_pair_convert( out, owidth, oheight, bmp, &Mode, Pal, err ) ) {
				goto done;
			}
		} else {
			cnvRecolor( out, owidth, oheight, bmp, &Mode, prog_quiet, Pal, tail, tailcnt );
		}
		if( is_sc2 && sc2_dump_env_set() ) {
			sc2_dump_after( Mode, bmp, Pal );
		}
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
		"SCREEN12 (256 lines)", "SCREEN8 EPAL (V9968)", "SCREEN8 EPAL (V9968) (256 lines)",
		"SCREEN2/4 EPAL (V9968)", "SCREEN3 EPAL (V9968)", "SCREEN5 EPAL (V9968)",
		"SCREEN6 EPAL (V9968)", "SCREEN7 EPAL (V9968)", "SCREEN5 EPAL (V9968) (256 lines)",
		"SCREEN6 EPAL (V9968) (256 lines)", "SCREEN7 EPAL (V9968) (256 lines)"
	};
	char buf[ 1024 ];
	const char* mode_name = ( m.screen_mode < MD_MAX ) ? mode_names[ m.screen_mode ] : "?";
	snprintf( buf, sizeof( buf ),
		"screen_mode=%s diffusion=%s coef=%.3f x_weight=%.3f cuterr=%u interlace=%s "
		"fixed_palette=%s resize=%s resample=%d size_mode=%d selcol=%u pltmode=%u basic=%s "
		"algo=%u jk_rc=%s dither=%u dither_add=%u seido=%u nonzero=%s force_zero=%u tile=%s "
		"sort=%d lines192=%s epal_hguard=%s sc2_pair=%s",
		mode_name,
		m.diffusion_error_enable ? "on" : "off", m.diffusion_error_coef, m.diffusion_error_x_weight,
		m.err, m.interlace ? "on" : "off", m.fixed_palette ? "on" : "off",
		m.resize_enable ? "on" : "off", m.Resample, m.SizeMode, m.SelCol, m.PltMode,
		m.output_basic_enable ? "on" : "off", m.AlgoMode, m.JKrc ? "on" : "off",
		m.ErrAlgo, m.ErrAdd, m.Seido, m.NonZero ? "on" : "off", m.FourceZero,
		m.Tile ? "on" : "off", m.SortMode, m.b192 ? "on" : "off",
		m.EpalHGuard ? "on" : "off", m.Sc2Pair ? "on" : "off" );
	return buf;
}
