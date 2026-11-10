// ---------------------------------------------------------------------------
// sc2_pair.cpp — see sc2_pair.h.  Everything here is a prototype:
// transcriptions of Convert.cpp statics (_rgb_to_itp / _distance_itp,
// cnvRecolor5, cnvSC5toSC2) are marked as such and exist so the
// prototype can be validated byte-for-byte against the product.
// ---------------------------------------------------------------------------
#include "sc2_pair.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Environment
// ---------------------------------------------------------------------------
static bool env_present( const char* name ) {
	const char* v = getenv( name );
	return v != NULL && *v != '\0';
}

bool sc2_pair_env_set() { return env_present( "BMP2MSX_SC2_PAIR" ); }
bool sc2_dump_env_set() { return env_present( "BMP2MSX_SC2_DUMP" ); }

static double pair_w() {
	const char* v = getenv( "BMP2MSX_SC2_PAIR" );
	return v ? strtod( v, NULL ) : 0.0;
}

static bool fit_is_pair() {
	const char* v = getenv( "BMP2MSX_SC2_FIT" );
	return v != NULL && !strcmp( v, "pair" );
}

static bool edge_enabled() {
	const char* v = getenv( "BMP2MSX_SC2_EDGE" );
	return v != NULL && strcmp( v, "0" ) != 0 && *v != '\0';
}

static double edge_pct() {
	const char* v = getenv( "BMP2MSX_SC2_EDGE_PCT" );
	double p = v ? strtod( v, NULL ) : 98.0;
	if( p <= 0.0 ) p = 98.0;
	if( p > 100.0 ) p = 100.0;
	return p;
}

static int edge_dilate() {
	const char* v = getenv( "BMP2MSX_SC2_EDGE_DILATE" );
	int n = v ? atoi( v ) : 1;
	if( n < 0 ) n = 0;
	if( n > 4 ) n = 4;
	return n;
}

// Dead zone: normalized e below this is treated as 0 (flat), above it
// is rescaled to [0,1].  Soft antialias ramps land in the mid range;
// without a dead zone they lose much of the pair term even though
// pair-averaging was serving them well.
static double edge_lo() {
	const char* v = getenv( "BMP2MSX_SC2_EDGE_LO" );
	double lo = v ? strtod( v, NULL ) : 0.0;
	if( lo < 0.0 ) lo = 0.0;
	if( lo > 0.95 ) lo = 0.95;
	return lo;
}

// ---------------------------------------------------------------------------
// v3 (flat-region selective mode): BMP2MSX_SC2_FLAT=1 restricts the
// pair machinery to detected flat-fill / gentle-gradient regions.
// ---------------------------------------------------------------------------
static bool flat_enabled() {
	const char* v = getenv( "BMP2MSX_SC2_FLAT" );
	return v != NULL && strcmp( v, "0" ) != 0 && *v != '\0';
}

// Local-variation threshold for the flat mask, in 8-bit channel units
// (robust P90-P10 range over a 5x5 window, max over channels).
// Negative forces an empty mask, >1000 a full mask (verification aids).
static double flat_range() {
	const char* v = getenv( "BMP2MSX_SC2_FLAT_RANGE" );
	return v ? strtod( v, NULL ) : 32.0;
}

// Outside-mask quantization source: "shadow" (the product's own
// quantizer incl. error diffusion, via the validated transcription) or
// "nearest" (per-dot nearest, no diffusion).
static bool flat_out_shadow() {
	const char* v = getenv( "BMP2MSX_SC2_FLAT_OUT" );
	return !( v != NULL && !strcmp( v, "nearest" ) );
}

// Boundary-block arbitration override for measurements: "arb"
// (default; true-error comparison), "line" (conventional fit always),
// "pair" (pair fit always).
static int flat_bnd_mode() {
	const char* v = getenv( "BMP2MSX_SC2_FLAT_BND" );
	if( v != NULL && !strcmp( v, "line" ) ) return 1;
	if( v != NULL && !strcmp( v, "pair" ) ) return 0;
	return -1;
}

static std::string dump_dir() {
	const char* v = getenv( "BMP2MSX_SC2_DUMP" );
	return v ? std::string( v ) : std::string();
}

static std::string run_label() {
	const char* v = getenv( "BMP2MSX_SC2_LABEL" );
	return v ? std::string( v ) : std::string( "unlabeled" );
}

// ---------------------------------------------------------------------------
// ICtCp distance — transcription of Convert.cpp _rgb_to_itp (doubled so
// sums of two colors can be transformed too) and _distance_itp.
// d2() is the squared distance; the product's _distance_itp returns
// (int)( sqrt( d2 ) * 65536. ), so orderings and ties match.
// ---------------------------------------------------------------------------
static void rgb_to_itp( double r, double g, double b, double* p_i, double* p_t, double* p_p ) {
	double l, m, s;
	l = ( 1688. * r + 2146. * g +  262. * b ) / ( 4096. * 255. );
	m = (  683. * r + 2951. * g +  462. * b ) / ( 4096. * 255. );
	s = (   99. * r +  309. * g + 3688. * b ) / ( 4096. * 255. );
	l = 0.5 * sqrt( l );
	m = 0.5 * sqrt( m );
	s = 0.5 * sqrt( s );
	*p_i = ( 2048. * l + 2048. * m             ) / 4096.;
	*p_t = ( 3625. * l - 7465. * m + 3840. * s ) / 4096.;
	*p_p = ( 9500. * l - 9212. * m -  288. * s ) / 4096.;
}

struct ITP { double i, t, p; };

static ITP itp_of( double r, double g, double b ) {
	ITP v;
	rgb_to_itp( r, g, b, &v.i, &v.t, &v.p );
	return v;
}

static double d2_itp( const ITP& a, const ITP& b ) {
	double i = a.i - b.i, t = a.t - b.t, p = a.p - b.p;
	return i * i + t * t + p * p;
}

// The product's integer distance (for exact tie behavior in transcriptions).
static int distance_itp_int( int r1, int g1, int b1, int r2, int g2, int b2 ) {
	ITP a = itp_of( r1, g1, b1 ), b = itp_of( r2, g2, b2 );
	return (int)( sqrt( d2_itp( a, b ) ) * 65536. );
}

// Squared product distance: _distance_itp's integer result, squared.
// Using the product's own (truncated) distance for every term of the
// pair objective makes w = 0 reproduce the product's per-pixel choices
// exactly, ties included.
static double dist2_product( const ITP& a, const ITP& b ) {
	int d = (int)( sqrt( d2_itp( a, b ) ) * 65536. );
	return (double) d * d;
}

static int range_limiter_i( int n, int mn, int mx ) {
	if( n < mn ) return mn;
	if( n > mx ) return mx;
	return n;
}

static int palette_luminance( C_COLOR c ) {
	return 77 * (int) GET_RED( c ) + 150 * (int) GET_GREEN( c ) + 29 * (int) GET_BLUE( c );
}

// ---------------------------------------------------------------------------
// Candidate palette (mirrors the search set of cnvRecolor5: indices
// 0..15, PALEN_NONE skipped, index 0 skipped when NonZero is set)
// ---------------------------------------------------------------------------
struct Cand {
	int idx;
	int r, g, b;
	ITP itp;
};

static std::vector< Cand > build_candidates( const SETTING* Mode, C_COLOR* pal ) {
	std::vector< Cand > out;
	int z0 = Mode->NonZero ? 1 : 0;
	for( int z = z0; z < 16; ++z ) {
		if( Mode->PalEn[ z ] == PALEN_NONE ) continue;
		Cand c;
		c.idx = z;
		c.r = GET_RED( pal[ z ] );
		c.g = GET_GREEN( pal[ z ] );
		c.b = GET_BLUE( pal[ z ] );
		c.itp = itp_of( c.r, c.g, c.b );
		out.push_back( c );
	}
	return out;
}

static C_COLOR bit_depth_mask( int seido ) {
	C_COLOR mask = ~( ( 1 << seido ) - 1 );
	mask = mask & ( ( mask << 8 ) | 0xFF ) & ( ( mask << 16 ) | 0xFFFF );
	return mask;
}

// ---------------------------------------------------------------------------
// Edge map (v2).  Gradient magnitude = Sobel per channel, max over the
// three channels: a luminance-only gradient would miss isoluminant hue
// boundaries, and those are exactly where pair-averaging would smear
// two different hues together.  Sobel (rather than central differences)
// for its built-in smoothing on antialiased / resampled targets.
// The magnitude is normalized by a high percentile of its own
// distribution (robust against a few extreme pixels), clamped to
// [0,1], then spread by 3x3 max-dilation passes so the edge zone
// covers the whole transition band (antialias fringe pixels beside a
// hard edge, whose own gradient peaks off-center); dilation also makes
// e_pair's max over the two pixels cover their immediate horizontal
// neighbours, so no separate neighbour aggregation is needed.
// ---------------------------------------------------------------------------
static std::vector< float > compute_edge_map( const C_COLOR* in, int w, int h,
											  double pct, int dilate, double lo, float* scale_out ) {
	std::vector< float > mag( (size_t) w * h, 0.0f );
	auto chan = [ & ]( const C_COLOR* p, int c ) -> double {
		return c == 0 ? GET_RED( *p ) : c == 1 ? GET_GREEN( *p ) : GET_BLUE( *p );
	};
	for( int y = 0; y < h; ++y ) {
		int ym = y > 0 ? y - 1 : 0, yp = y + 1 < h ? y + 1 : h - 1;
		for( int x = 0; x < w; ++x ) {
			int xm = x > 0 ? x - 1 : 0, xp = x + 1 < w ? x + 1 : w - 1;
			double best = 0.0;
			for( int c = 0; c < 3; ++c ) {
				double gx = ( chan( &in[ (size_t) ym * w + xp ], c ) + 2 * chan( &in[ (size_t) y * w + xp ], c ) + chan( &in[ (size_t) yp * w + xp ], c ) )
						  - ( chan( &in[ (size_t) ym * w + xm ], c ) + 2 * chan( &in[ (size_t) y * w + xm ], c ) + chan( &in[ (size_t) yp * w + xm ], c ) );
				double gy = ( chan( &in[ (size_t) yp * w + xm ], c ) + 2 * chan( &in[ (size_t) yp * w + x ], c ) + chan( &in[ (size_t) yp * w + xp ], c ) )
						  - ( chan( &in[ (size_t) ym * w + xm ], c ) + 2 * chan( &in[ (size_t) ym * w + x ], c ) + chan( &in[ (size_t) ym * w + xp ], c ) );
				double m = sqrt( gx * gx + gy * gy );
				if( m > best ) best = m;
			}
			mag[ (size_t) y * w + x ] = (float) best;
		}
	}
	// percentile scale
	std::vector< float > sorted = mag;
	std::sort( sorted.begin(), sorted.end() );
	size_t rank = (size_t) ceil( pct / 100.0 * (double) sorted.size() );
	if( rank == 0 ) rank = 1;
	if( rank > sorted.size() ) rank = sorted.size();
	float scale = sorted[ rank - 1 ];
	if( scale_out ) *scale_out = scale;
	std::vector< float > e( (size_t) w * h, 0.0f );
	if( scale > 0.0f ) {
		for( size_t i = 0; i < e.size(); ++i ) {
			double v = (double) mag[ i ] / scale;
			if( v > 1.0 ) v = 1.0;
			v = ( v - lo ) / ( 1.0 - lo );
			e[ i ] = v > 0.0 ? (float) v : 0.0f;
		}
	}
	// 3x3 max dilation passes
	for( int pass = 0; pass < dilate; ++pass ) {
		std::vector< float > src = e;
		for( int y = 0; y < h; ++y ) {
			int ym = y > 0 ? y - 1 : 0, yp = y + 1 < h ? y + 1 : h - 1;
			for( int x = 0; x < w; ++x ) {
				int xm = x > 0 ? x - 1 : 0, xp = x + 1 < w ? x + 1 : w - 1;
				float m = 0.0f;
				for( int yy = ym; yy <= yp; ++yy )
					for( int xx = xm; xx <= xp; ++xx )
						if( src[ (size_t) yy * w + xx ] > m ) m = src[ (size_t) yy * w + xx ];
				e[ (size_t) y * w + x ] = m;
			}
		}
	}
	return e;
}

// Effective pair weight at (x, pair starting at line y): w * (1 - e_pair).
static double pair_weight_at( const float* emap, int w, int y, int x, bool have2, double weight ) {
	if( !emap ) return weight;
	float ep = emap[ (size_t) y * w + x ];
	if( have2 ) {
		float e2 = emap[ (size_t) ( y + 1 ) * w + x ];
		if( e2 > ep ) ep = e2;
	}
	return weight * ( 1.0 - (double) ep );
}

// ---------------------------------------------------------------------------
// Pair quantization: fill idx[] (one palette index per pixel).
// ---------------------------------------------------------------------------
static void pair_quantize( const C_COLOR* in, int w, int h, const SETTING* Mode,
						   C_COLOR* pal, double weight, const float* emap,
						   std::vector< unsigned char >& idx ) {
	std::vector< Cand > cand = build_candidates( Mode, pal );
	int nc = (int) cand.size();
	idx.assign( (size_t) w * h, 0 );

	// ITP of every palette pair sum, for the w term.
	std::vector< ITP > pairsum( (size_t) nc * nc );
	for( int a = 0; a < nc; ++a )
		for( int b = 0; b < nc; ++b )
			pairsum[ (size_t) a * nc + b ] = itp_of( cand[ a ].r + cand[ b ].r,
													 cand[ a ].g + cand[ b ].g,
													 cand[ a ].b + cand[ b ].b );

	C_COLOR fz = Mode->FourceZero ? ( Mode->FZColor & bit_depth_mask( Mode->Seido ) ) : (C_COLOR) -1;

	for( int y = 0; y < h; y += 2 ) {
		bool have2 = ( y + 1 < h );
		for( int x = 0; x < w; ++x ) {
			C_COLOR c1px = in[ (size_t) y * w + x ];
			C_COLOR c2px = have2 ? in[ (size_t) ( y + 1 ) * w + x ] : c1px;
			ITP t1 = itp_of( GET_RED( c1px ), GET_GREEN( c1px ), GET_BLUE( c1px ) );
			ITP t2 = itp_of( GET_RED( c2px ), GET_GREEN( c2px ), GET_BLUE( c2px ) );
			ITP ts = itp_of( (double) GET_RED( c1px ) + GET_RED( c2px ),
							 (double) GET_GREEN( c1px ) + GET_GREEN( c2px ),
							 (double) GET_BLUE( c1px ) + GET_BLUE( c2px ) );
			double weff = pair_weight_at( emap, w, y, x, have2, weight );
			double best = 0;
			int ba = 0, bb = 0;
			bool first = true;
			for( int a = 0; a < nc; ++a ) {
				double e1 = dist2_product( t1, cand[ a ].itp );
				for( int b = 0; b < nc; ++b ) {
					double e = e1 + dist2_product( t2, cand[ b ].itp );
					if( weff != 0.0 ) e += weff * dist2_product( ts, pairsum[ (size_t) a * nc + b ] );
					if( first || e < best ) {
						best = e; ba = a; bb = b; first = false;
					}
				}
			}
			int i1 = cand[ ba ].idx, i2 = cand[ bb ].idx;
			if( Mode->FourceZero ) {
				if( pal[ i1 ] == fz ) i1 = 0;
				if( pal[ i2 ] == fz ) i2 = 0;
			}
			idx[ (size_t) y * w + x ] = (unsigned char) i1;
			if( have2 ) idx[ (size_t) ( y + 1 ) * w + x ] = (unsigned char) i2;
		}
	}
}

// ---------------------------------------------------------------------------
// Cell emission — the byte-level conventions of cnvSC5toSC2's tail:
// color byte = (fg << 4) | bg, pattern bit 1 selects the fg (high
// nibble) color, and the brighter color is normalized into fg.
// `first` / `second` are palette indices, bits has bit (7-j) set when
// pixel j takes `first` (before normalization).
// ---------------------------------------------------------------------------
static void emit_cell( unsigned char* vram, int dadr, C_COLOR* pal, int first, int second, int bits ) {
	int c, p;
	if( first == second ) {
		c = first;
		p = 0x00;
		int lum1 = palette_luminance( pal[ c & 0x0F ] );
		if( lum1 >= ( 128 << 8 ) ) {
			p = p ^ 255;
			c = ( ( c << 4 ) & 0xF0 ) | ( ( c >> 4 ) & 0x0F );
		}
	} else {
		c = ( first << 4 ) | second;
		p = bits;
		int lum1 = palette_luminance( pal[ c & 0x0F ] );
		int lum2 = palette_luminance( pal[ ( c >> 4 ) & 0x0F ] );
		if( ( lum1 == lum2 && ( c & 0x0F ) > ( ( c >> 4 ) & 0x0F ) ) || ( lum1 > lum2 ) ) {
			p = p ^ 255;
			c = ( ( c << 4 ) & 0xF0 ) | ( ( c >> 4 ) & 0x0F );
		}
	}
	vram[ dadr ] = (unsigned char) p;
	vram[ dadr + 0x2000 ] = (unsigned char) c;
}

// ---------------------------------------------------------------------------
// Variant (b) enforcement — faithful transcription of cnvSC5toSC2's
// selection logic, reading the per-pixel index image instead of the
// packed nibble bitmap (identical contents).
// ---------------------------------------------------------------------------
static void enforce_line_product( const std::vector< unsigned char >& idx, int w, int h,
								  C_COLOR* pal, unsigned char* vram ) {
	memset( vram, 0, 0x4000 );
	for( int y = 0; y < 192; ++y ) {
		for( int cx = 0; cx < 32; ++cx ) {
			int cnt = 0, idx1 = 0, idx2 = 0;
			int ccnt[ 8 ] = { 0 };
			int ccol[ 8 ] = { 0 };
			for( int j = 0; j < 8; ++j ) {
				int cc = idx[ (size_t) y * w + cx * 8 + j ];
				int i;
				for( i = 0; i < cnt; ++i ) {
					if( cc == ccol[ i ] ) {
						ccnt[ i ]++;
						if( ccnt[ i ] > ccnt[ idx1 ] ) idx1 = i;
						break;
					}
				}
				if( i == cnt ) {
					ccol[ cnt ] = cc;
					ccnt[ cnt ] = 1;
					cnt++;
				}
			}
			int dadr = ( y & 7 ) + ( cx * 8 ) + ( y >> 3 ) * 256;
			if( cnt < 2 ) {
				emit_cell( vram, dadr, pal, ccol[ 0 ], ccol[ 0 ], 0 );
			} else {
				C_COLOR c1 = pal[ ccol[ idx1 ] ];
				int len = 0;
				for( int i = 0; i < cnt; ++i ) {
					if( i == idx1 ) continue;
					C_COLOR pc = pal[ ccol[ i ] ];
					int l = distance_itp_int( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
											  GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ) );
					if( l >= len ) {
						len = l;
						idx2 = i;
					}
				}
				C_COLOR c2 = pal[ ccol[ idx2 ] ];
				int bits = 0;
				for( int j = 0; j < 8; ++j ) {
					int cc = idx[ (size_t) y * w + cx * 8 + j ];
					C_COLOR pc = pal[ cc ];
					bits <<= 1;
					if( distance_itp_int( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
										  GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ) )
						< distance_itp_int( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
											GET_RED( c2 ), GET_GREEN( c2 ), GET_BLUE( c2 ) ) ) {
						bits |= 1;
					}
				}
				emit_cell( vram, dadr, pal, ccol[ idx1 ], ccol[ idx2 ], bits );
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Variant (c): pair-aware fitting.  Per 8x2 block, shortlist candidate
// colors per line (from the unconstrained pair-quantized indices plus
// per-pixel nearest colors), then choose the even pair and the odd
// pair minimizing the block total of the pair objective, assigning
// each pixel column jointly over the four chosen colors.
// ---------------------------------------------------------------------------
static void enforce_pair_aware( const C_COLOR* in, const std::vector< unsigned char >& idx,
								int w, int h, const SETTING* Mode, C_COLOR* pal,
								double weight, const float* emap, unsigned char* vram ) {
	std::vector< Cand > cand = build_candidates( Mode, pal );
	memset( vram, 0, 0x4000 );

	auto cand_of = [ & ]( int palidx ) -> const Cand* {
		for( auto& c : cand ) if( c.idx == palidx ) return &c;
		return nullptr;
	};

	for( int y = 0; y + 1 < h; y += 2 ) {
		for( int cx = 0; cx < 32; ++cx ) {
			// --- shortlists per line (palette indices, cap 5) ---
			std::vector< int > list[ 2 ];
			for( int ln = 0; ln < 2; ++ln ) {
				int yy = y + ln;
				int order[ 16 ], freq[ 16 ], no = 0;
				for( int j = 0; j < 8; ++j ) {
					int cc = idx[ (size_t) yy * w + cx * 8 + j ];
					int i;
					for( i = 0; i < no; ++i ) if( order[ i ] == cc ) { freq[ i ]++; break; }
					if( i == no ) { order[ no ] = cc; freq[ no ] = 1; no++; }
				}
				// stable sort by frequency desc
				for( int a = 0; a < no; ++a )
					for( int b = a + 1; b < no; ++b )
						if( freq[ b ] > freq[ a ] ) {
							int t1 = order[ a ]; order[ a ] = order[ b ]; order[ b ] = t1;
							int t2 = freq[ a ]; freq[ a ] = freq[ b ]; freq[ b ] = t2;
						}
				for( int i = 0; i < no && (int) list[ ln ].size() < 5; ++i ) list[ ln ].push_back( order[ i ] );
				// add per-pixel nearest colors not already present
				for( int j = 0; j < 8 && (int) list[ ln ].size() < 5; ++j ) {
					C_COLOR px = in[ (size_t) yy * w + cx * 8 + j ];
					ITP tp = itp_of( GET_RED( px ), GET_GREEN( px ), GET_BLUE( px ) );
					int besti = -1;
					double bestd = 0;
					for( int ci = 0; ci < (int) cand.size(); ++ci ) {
						double d = dist2_product( tp, cand[ ci ].itp );
						if( besti < 0 || d < bestd ) { bestd = d; besti = cand[ ci ].idx; }
					}
					bool have = false;
					for( int v : list[ ln ] ) if( v == besti ) have = true;
					if( !have && besti >= 0 ) list[ ln ].push_back( besti );
				}
				if( list[ ln ].empty() ) list[ ln ].push_back( cand[ 0 ].idx );
			}

			// --- target ITP values for the block ---
			ITP t1[ 8 ], t2[ 8 ], ts[ 8 ];
			for( int j = 0; j < 8; ++j ) {
				C_COLOR a = in[ (size_t) y * w + cx * 8 + j ];
				C_COLOR b = in[ (size_t) ( y + 1 ) * w + cx * 8 + j ];
				t1[ j ] = itp_of( GET_RED( a ), GET_GREEN( a ), GET_BLUE( a ) );
				t2[ j ] = itp_of( GET_RED( b ), GET_GREEN( b ), GET_BLUE( b ) );
				ts[ j ] = itp_of( (double) GET_RED( a ) + GET_RED( b ),
								  (double) GET_GREEN( a ) + GET_GREEN( b ),
								  (double) GET_BLUE( a ) + GET_BLUE( b ) );
			}

			// --- search line pairs ---
			double best_total = 0;
			bool first = true;
			int best_assign[ 2 ][ 8 ];
			int best_pair[ 2 ][ 2 ];
			int assign[ 2 ][ 8 ];
			for( int e1 = 0; e1 < (int) list[ 0 ].size(); ++e1 )
			for( int e2 = e1; e2 < (int) list[ 0 ].size(); ++e2 )
			for( int o1 = 0; o1 < (int) list[ 1 ].size(); ++o1 )
			for( int o2 = o1; o2 < (int) list[ 1 ].size(); ++o2 ) {
				const Cand* ep[ 2 ] = { cand_of( list[ 0 ][ e1 ] ), cand_of( list[ 0 ][ e2 ] ) };
				const Cand* op[ 2 ] = { cand_of( list[ 1 ][ o1 ] ), cand_of( list[ 1 ][ o2 ] ) };
				if( !ep[ 0 ] || !ep[ 1 ] || !op[ 0 ] || !op[ 1 ] ) continue;
				double total = 0;
				for( int j = 0; j < 8; ++j ) {
					double weff = pair_weight_at( emap, w, y, cx * 8 + j, true, weight );
					double bj = 0;
					int ba = 0, bb = 0;
					bool fj = true;
					for( int a = 0; a < 2; ++a ) {
						for( int b = 0; b < 2; ++b ) {
							ITP sum = itp_of( ep[ a ]->r + op[ b ]->r, ep[ a ]->g + op[ b ]->g, ep[ a ]->b + op[ b ]->b );
							double e = dist2_product( t1[ j ], ep[ a ]->itp ) + dist2_product( t2[ j ], op[ b ]->itp );
							if( weff != 0.0 ) e += weff * dist2_product( ts[ j ], sum );
							if( fj || e < bj ) { bj = e; ba = a; bb = b; fj = false; }
						}
					}
					total += bj;
					assign[ 0 ][ j ] = ep[ ba ]->idx;
					assign[ 1 ][ j ] = op[ bb ]->idx;
				}
				if( first || total < best_total ) {
					best_total = total;
					first = false;
					memcpy( best_assign, assign, sizeof( assign ) );
					best_pair[ 0 ][ 0 ] = list[ 0 ][ e1 ];
					best_pair[ 0 ][ 1 ] = list[ 0 ][ e2 ];
					best_pair[ 1 ][ 0 ] = list[ 1 ][ o1 ];
					best_pair[ 1 ][ 1 ] = list[ 1 ][ o2 ];
				}
			}

			// --- emit both lines ---
			for( int ln = 0; ln < 2; ++ln ) {
				int yy = y + ln;
				int first_c = best_pair[ ln ][ 0 ], second_c = best_pair[ ln ][ 1 ];
				int bits = 0;
				for( int j = 0; j < 8; ++j ) {
					bits <<= 1;
					if( best_assign[ ln ][ j ] == first_c ) bits |= 1;
				}
				int dadr = ( yy & 7 ) + ( cx * 8 ) + ( yy >> 3 ) * 256;
				emit_cell( vram, dadr, pal, first_c, second_c, bits );
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Dump helpers
// ---------------------------------------------------------------------------
static void write_ppm( const std::string& path, const unsigned char* rgb, int w, int h ) {
	FILE* f = fopen( path.c_str(), "wb" );
	if( !f ) return;
	fprintf( f, "P6\n%d %d\n255\n", w, h );
	fwrite( rgb, 1, (size_t) w * h * 3, f );
	fclose( f );
}

static void target_rgb( const C_COLOR* in, int w, int h, std::vector< unsigned char >& rgb ) {
	rgb.assign( (size_t) w * h * 3, 0 );
	for( int i = 0; i < w * h; ++i ) {
		rgb[ (size_t) i * 3 + 0 ] = (unsigned char) GET_RED( in[ i ] );
		rgb[ (size_t) i * 3 + 1 ] = (unsigned char) GET_GREEN( in[ i ] );
		rgb[ (size_t) i * 3 + 2 ] = (unsigned char) GET_BLUE( in[ i ] );
	}
}

static void decode_vram( const unsigned char* vram, C_COLOR* pal, int w, int h,
						 std::vector< unsigned char >& rgb, std::vector< unsigned char >& ridx ) {
	rgb.assign( (size_t) w * h * 3, 0 );
	ridx.assign( (size_t) w * h, 0 );
	for( int y = 0; y < 192 && y < h; ++y ) {
		for( int cx = 0; cx < 32; ++cx ) {
			int dadr = ( y & 7 ) + ( cx * 8 ) + ( y >> 3 ) * 256;
			int p = vram[ dadr ];
			int c = vram[ dadr + 0x2000 ];
			int fg = ( c >> 4 ) & 0x0F, bg = c & 0x0F;
			for( int j = 0; j < 8; ++j ) {
				int bit = ( p >> ( 7 - j ) ) & 1;
				int id = bit ? fg : bg;
				size_t o = (size_t) y * w + cx * 8 + j;
				ridx[ o ] = (unsigned char) id;
				rgb[ o * 3 + 0 ] = (unsigned char) GET_RED( pal[ id ] );
				rgb[ o * 3 + 1 ] = (unsigned char) GET_GREEN( pal[ id ] );
				rgb[ o * 3 + 2 ] = (unsigned char) GET_BLUE( pal[ id ] );
			}
		}
	}
}

struct DemandStats {
	int cells_eq1 = 0, cells_eq2 = 0, cells_ge3 = 0, excess = 0;
};

static DemandStats demand_stats( const std::vector< unsigned char >& idx, int w, int h ) {
	DemandStats s;
	for( int y = 0; y < h; ++y ) {
		for( int cx = 0; cx < w / 8; ++cx ) {
			bool seen[ 16 ] = {};
			int distinct = 0;
			for( int j = 0; j < 8; ++j ) {
				int id = idx[ (size_t) y * w + cx * 8 + j ];
				if( !seen[ id ] ) { seen[ id ] = true; distinct++; }
			}
			if( distinct <= 1 ) s.cells_eq1++;
			else if( distinct == 2 ) s.cells_eq2++;
			else { s.cells_ge3++; s.excess += distinct - 2; }
		}
	}
	return s;
}

static void write_index_bin( const std::string& path, const std::vector< unsigned char >& idx, int w, int h ) {
	FILE* f = fopen( path.c_str(), "wb" );
	if( !f ) return;
	unsigned int dims[ 2 ] = { (unsigned int) w, (unsigned int) h };
	fwrite( dims, 4, 2, f );
	fwrite( idx.data(), 1, idx.size(), f );
	fclose( f );
}

// ---------------------------------------------------------------------------
// Shadow of the product quantizer (cnvRecolor5), used ONLY to obtain
// the unconstrained index image of a product (control) run for stats.
// Valid only when dither is EALGO_NONE and Tile is off; the caller
// compares the shadow's enforced VRAM against the real output and
// records the match in stats.txt.
// ---------------------------------------------------------------------------
static bool shadow_quantize( const C_COLOR* in, int w, int h, const SETTING* Mode, C_COLOR* pal,
							 std::vector< unsigned char >& idx ) {
	if( Mode->ErrAlgo != EALGO_NONE || Mode->Tile ) return false;
	int coef = 0, xcoef = 0, ycoef = 0;
	if( Mode->diffusion_error_enable ) {
		coef = (int)( Mode->diffusion_error_coef * 1024 );
		xcoef = (int)( Mode->diffusion_error_x_weight * 256. );
		ycoef = 256 - xcoef;
	}
	std::vector< signed short > errbuf[ 2 ];
	for( int i = 0; i < 2; ++i ) errbuf[ i ].assign( (size_t)( w + 5 ) * 4, 0 );
	C_COLOR fz = Mode->FourceZero ? ( Mode->FZColor & bit_depth_mask( Mode->Seido ) ) : (C_COLOR) -1;
	idx.assign( (size_t) w * h, 0 );

	for( int y = 0; y < h; ++y ) {
		signed short* px = errbuf[ y & 1 ].data();
		signed short* py = errbuf[ 1 - ( y & 1 ) ].data();
		for( int x = 0; x < w; ++x ) {
			C_COLOR c = in[ (size_t) y * w + x ];
			int cr = GET_RED( c ), cg = GET_GREEN( c ), cb = GET_BLUE( c );
			if( coef ) {
				cr += px[ x * 4 + 0 ];
				cg += px[ x * 4 + 1 ];
				cb += px[ x * 4 + 2 ];
			}
			int pr = range_limiter_i( cr, 0, 255 );
			int pg = range_limiter_i( cg, 0, 255 );
			int pb = range_limiter_i( cb, 0, 255 );
			int nr = 0x7FFFFFFF, n = 0;
			C_COLOR cc = c;
			for( int z = Mode->NonZero ? 1 : 0; z < 16; ++z ) {
				if( Mode->PalEn[ z ] == PALEN_NONE ) continue;
				C_COLOR pc = pal[ z ];
				int r = distance_itp_int( pr, pg, pb, GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ) );
				if( r < nr ) {
					nr = r;
					n = z;
					cc = pc;
					if( r == 0 ) break;
				}
			}
			if( coef ) {
				int er = range_limiter_i( ( cr - (int) GET_RED( cc ) ) * coef / 1024, -32768, 32767 );
				int eg = range_limiter_i( ( cg - (int) GET_GREEN( cc ) ) * coef / 1024, -32768, 32767 );
				int eb = range_limiter_i( ( cb - (int) GET_BLUE( cc ) ) * coef / 1024, -32768, 32767 );
				if( abs( er ) < (signed) Mode->err ) er = 0;
				if( abs( eg ) < (signed) Mode->err ) eg = 0;
				if( abs( eb ) < (signed) Mode->err ) eb = 0;
				px[ ( x + 1 ) * 4 + 0 ] = (signed short)( px[ ( x + 1 ) * 4 + 0 ] + (signed short)( ( er * xcoef ) >> 8 ) );
				px[ ( x + 1 ) * 4 + 1 ] = (signed short)( px[ ( x + 1 ) * 4 + 1 ] + (signed short)( ( eg * xcoef ) >> 8 ) );
				px[ ( x + 1 ) * 4 + 2 ] = (signed short)( px[ ( x + 1 ) * 4 + 2 ] + (signed short)( ( eb * xcoef ) >> 8 ) );
				py[ x * 4 + 0 ] = (signed short)( ( er * ycoef ) >> 8 );
				py[ x * 4 + 1 ] = (signed short)( ( eg * ycoef ) >> 8 );
				py[ x * 4 + 2 ] = (signed short)( ( eb * ycoef ) >> 8 );
			}
			if( Mode->FourceZero && cc == fz ) n = 0;
			idx[ (size_t) y * w + x ] = (unsigned char) n;
		}
	}
	return true;
}

// ---------------------------------------------------------------------------
// v3: flat / gentle-gradient region mask.
//
// Flatness is judged per dot from the target at conversion size by a
// ROBUST local range: for each channel, the P90-P10 spread of the 25
// samples in the 5x5 window (sorted indices 21 and 2), maximized over
// channels.  A plain max-min range is useless on real material (one
// JPEG-grain pixel sets it), and a blur-then-range statistic is worse
// (a 1px checkerboard blurs to uniform gray and would be judged
// "flat").  The percentile spread keeps bimodal content (edges,
// checkerboards, line art) firmly outside while unimodal spread --
// sensor/JPEG grain and the consistent small slope of a gentle ramp --
// stays under the threshold.  The binary mask is then cleaned by one
// 5x5 majority pass (>= 13 of 25), which drops isolated specks and
// fills pinholes so regions are coherent.
// ---------------------------------------------------------------------------
static std::vector< char > compute_flat_mask( const C_COLOR* in, int w, int h,
											  double range_thr, double* coverage_out ) {
	std::vector< char > mask( (size_t) w * h, 0 );
	if( coverage_out ) *coverage_out = 0.0;
	if( range_thr < 0.0 ) return mask;
	if( range_thr > 1000.0 ) {
		std::fill( mask.begin(), mask.end(), (char) 1 );
		if( coverage_out ) *coverage_out = 1.0;
		return mask;
	}
	std::vector< char > raw( (size_t) w * h, 0 );
	for( int y = 0; y < h; ++y ) {
		for( int x = 0; x < w; ++x ) {
			double worst = 0.0;
			for( int c = 0; c < 3; ++c ) {
				int v[ 25 ];
				int n = 0;
				for( int dy = -2; dy <= 2; ++dy ) {
					int yy = y + dy; if( yy < 0 ) yy = 0; if( yy >= h ) yy = h - 1;
					for( int dx = -2; dx <= 2; ++dx ) {
						int xx = x + dx; if( xx < 0 ) xx = 0; if( xx >= w ) xx = w - 1;
						C_COLOR px = in[ (size_t) yy * w + xx ];
						v[ n++ ] = c == 0 ? GET_RED( px ) : c == 1 ? GET_GREEN( px ) : GET_BLUE( px );
					}
				}
				std::sort( v, v + 25 );
				double rng = (double) ( v[ 21 ] - v[ 2 ] );
				if( rng > worst ) worst = rng;
			}
			raw[ (size_t) y * w + x ] = worst <= range_thr ? 1 : 0;
		}
	}
	// majority cleanup (5x5, >= 13 of 25, clamped borders)
	for( int y = 0; y < h; ++y ) {
		for( int x = 0; x < w; ++x ) {
			int cnt = 0;
			for( int dy = -2; dy <= 2; ++dy ) {
				int yy = y + dy; if( yy < 0 ) yy = 0; if( yy >= h ) yy = h - 1;
				for( int dx = -2; dx <= 2; ++dx ) {
					int xx = x + dx; if( xx < 0 ) xx = 0; if( xx >= w ) xx = w - 1;
					cnt += raw[ (size_t) yy * w + xx ];
				}
			}
			mask[ (size_t) y * w + x ] = cnt >= 13 ? 1 : 0;
		}
	}
	if( coverage_out ) {
		size_t on = 0;
		for( char v : mask ) on += v;
		*coverage_out = mask.empty() ? 0.0 : (double) on / (double) mask.size();
	}
	return mask;
}

// ---------------------------------------------------------------------------
// v3 quantization: pair-mode columns (both pixels of the vertical pair
// inside the mask) take the joint pair quantization at full w; every
// other pixel takes the conventional per-dot result -- the product's
// own quantizer (with its error diffusion) via the validated shadow
// transcription when available, else per-dot nearest.
// NOTE the structural caveat: the shadow's diffusion stream was
// computed as if every pixel took the shadow's choice; pair-mode
// pixels take different colors, so the error that "would have"
// propagated out of them differs.  A product implementation would
// need one coupled stream; the prototype keeps the two streams
// separate outside the mask.
// ---------------------------------------------------------------------------
static void quantize_v3( const C_COLOR* in, int w, int h, const SETTING* Mode, C_COLOR* pal,
						 double weight, const std::vector< char >& mask,
						 std::vector< char >& pairmode_out,
						 std::vector< unsigned char >& idx, bool* used_shadow_out ) {
	std::vector< unsigned char > idx_pair, idx_conv;
	pair_quantize( in, w, h, Mode, pal, weight, nullptr, idx_pair );
	bool have_shadow = false;
	if( flat_out_shadow() ) have_shadow = shadow_quantize( in, w, h, Mode, pal, idx_conv );
	if( !have_shadow ) pair_quantize( in, w, h, Mode, pal, 0.0, nullptr, idx_conv );
	if( used_shadow_out ) *used_shadow_out = have_shadow;
	idx.assign( (size_t) w * h, 0 );
	pairmode_out.assign( (size_t) ( h / 2 ) * w, 0 );
	for( int y = 0; y + 1 < h; y += 2 ) {
		for( int x = 0; x < w; ++x ) {
			bool pm = mask[ (size_t) y * w + x ] && mask[ (size_t) ( y + 1 ) * w + x ];
			pairmode_out[ (size_t) ( y / 2 ) * w + x ] = pm ? 1 : 0;
			idx[ (size_t) y * w + x ]         = pm ? idx_pair[ (size_t) y * w + x ]         : idx_conv[ (size_t) y * w + x ];
			idx[ (size_t) ( y + 1 ) * w + x ] = pm ? idx_pair[ (size_t) ( y + 1 ) * w + x ] : idx_conv[ (size_t) ( y + 1 ) * w + x ];
		}
	}
	if( h % 2 ) {
		for( int x = 0; x < w; ++x ) idx[ (size_t) ( h - 1 ) * w + x ] = idx_conv[ (size_t) ( h - 1 ) * w + x ];
	}
}

// ---------------------------------------------------------------------------
// v3 cell fitting candidates.  Both return, per line, the two chosen
// palette indices (first/second) plus the rendered palette index of
// every pixel, so a boundary block can compare candidates by true
// error and emit the winner through the shared emit_cell().
// ---------------------------------------------------------------------------
// Conventional per-line fit for one cell: an exact copy of
// enforce_line_product's selection logic (kept separate so the v1/v2
// paths stay untouched), extended with the per-pixel assignment.
static void fit_line_cell( const std::vector< unsigned char >& idx, int w, int y, int cx,
						   C_COLOR* pal, int* first, int* second, int assign[ 8 ] ) {
	int cnt = 0, idx1 = 0, idx2 = 0;
	int ccnt[ 8 ] = { 0 };
	int ccol[ 8 ] = { 0 };
	for( int j = 0; j < 8; ++j ) {
		int cc = idx[ (size_t) y * w + cx * 8 + j ];
		int i;
		for( i = 0; i < cnt; ++i ) {
			if( cc == ccol[ i ] ) {
				ccnt[ i ]++;
				if( ccnt[ i ] > ccnt[ idx1 ] ) idx1 = i;
				break;
			}
		}
		if( i == cnt ) {
			ccol[ cnt ] = cc;
			ccnt[ cnt ] = 1;
			cnt++;
		}
	}
	if( cnt < 2 ) {
		*first = *second = ccol[ 0 ];
		for( int j = 0; j < 8; ++j ) assign[ j ] = ccol[ 0 ];
		return;
	}
	C_COLOR c1 = pal[ ccol[ idx1 ] ];
	int len = 0;
	for( int i = 0; i < cnt; ++i ) {
		if( i == idx1 ) continue;
		C_COLOR pc = pal[ ccol[ i ] ];
		int l = distance_itp_int( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
								  GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ) );
		if( l >= len ) {
			len = l;
			idx2 = i;
		}
	}
	C_COLOR c2 = pal[ ccol[ idx2 ] ];
	*first = ccol[ idx1 ];
	*second = ccol[ idx2 ];
	for( int j = 0; j < 8; ++j ) {
		int cc = idx[ (size_t) y * w + cx * 8 + j ];
		C_COLOR pc = pal[ cc ];
		assign[ j ] = ( distance_itp_int( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
										  GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ) )
					  < distance_itp_int( GET_RED( pc ), GET_GREEN( pc ), GET_BLUE( pc ),
										  GET_RED( c2 ), GET_GREEN( c2 ), GET_BLUE( c2 ) ) )
						? ccol[ idx1 ] : ccol[ idx2 ];
	}
}

// Pair-aware fit for one 8x2 block: an exact copy of
// enforce_pair_aware's per-block search, except the pair weight is
// given per column (colw[j]) instead of through the edge map, so a
// boundary block's pair candidate is itself mask-selective.
static void fit_pair_block( const C_COLOR* in, const std::vector< unsigned char >& idx,
							int w, int y, int cx, const SETTING* Mode, C_COLOR* pal,
							const double colw[ 8 ], int best_pair[ 2 ][ 2 ], int best_assign[ 2 ][ 8 ] ) {
	std::vector< Cand > cand = build_candidates( Mode, pal );
	auto cand_of = [ & ]( int palidx ) -> const Cand* {
		for( auto& c : cand ) if( c.idx == palidx ) return &c;
		return nullptr;
	};
	// --- shortlists per line (palette indices, cap 5) ---
	std::vector< int > list[ 2 ];
	for( int ln = 0; ln < 2; ++ln ) {
		int yy = y + ln;
		int order[ 16 ], freq[ 16 ], no = 0;
		for( int j = 0; j < 8; ++j ) {
			int cc = idx[ (size_t) yy * w + cx * 8 + j ];
			int i;
			for( i = 0; i < no; ++i ) if( order[ i ] == cc ) { freq[ i ]++; break; }
			if( i == no ) { order[ no ] = cc; freq[ no ] = 1; no++; }
		}
		for( int a = 0; a < no; ++a )
			for( int b = a + 1; b < no; ++b )
				if( freq[ b ] > freq[ a ] ) {
					int t1 = order[ a ]; order[ a ] = order[ b ]; order[ b ] = t1;
					int t2 = freq[ a ]; freq[ a ] = freq[ b ]; freq[ b ] = t2;
				}
		for( int i = 0; i < no && (int) list[ ln ].size() < 5; ++i ) list[ ln ].push_back( order[ i ] );
		for( int j = 0; j < 8 && (int) list[ ln ].size() < 5; ++j ) {
			C_COLOR px = in[ (size_t) yy * w + cx * 8 + j ];
			ITP tp = itp_of( GET_RED( px ), GET_GREEN( px ), GET_BLUE( px ) );
			int besti = -1;
			double bestd = 0;
			for( int ci = 0; ci < (int) cand.size(); ++ci ) {
				double d = dist2_product( tp, cand[ ci ].itp );
				if( besti < 0 || d < bestd ) { bestd = d; besti = cand[ ci ].idx; }
			}
			bool have = false;
			for( int v : list[ ln ] ) if( v == besti ) have = true;
			if( !have && besti >= 0 ) list[ ln ].push_back( besti );
		}
		if( list[ ln ].empty() ) list[ ln ].push_back( cand[ 0 ].idx );
	}
	// --- target ITP values for the block ---
	ITP t1[ 8 ], t2[ 8 ], ts[ 8 ];
	for( int j = 0; j < 8; ++j ) {
		C_COLOR a = in[ (size_t) y * w + cx * 8 + j ];
		C_COLOR b = in[ (size_t) ( y + 1 ) * w + cx * 8 + j ];
		t1[ j ] = itp_of( GET_RED( a ), GET_GREEN( a ), GET_BLUE( a ) );
		t2[ j ] = itp_of( GET_RED( b ), GET_GREEN( b ), GET_BLUE( b ) );
		ts[ j ] = itp_of( (double) GET_RED( a ) + GET_RED( b ),
						  (double) GET_GREEN( a ) + GET_GREEN( b ),
						  (double) GET_BLUE( a ) + GET_BLUE( b ) );
	}
	// --- search line pairs ---
	double best_total = 0;
	bool first = true;
	int assign[ 2 ][ 8 ];
	for( int e1 = 0; e1 < (int) list[ 0 ].size(); ++e1 )
	for( int e2 = e1; e2 < (int) list[ 0 ].size(); ++e2 )
	for( int o1 = 0; o1 < (int) list[ 1 ].size(); ++o1 )
	for( int o2 = o1; o2 < (int) list[ 1 ].size(); ++o2 ) {
		const Cand* ep[ 2 ] = { cand_of( list[ 0 ][ e1 ] ), cand_of( list[ 0 ][ e2 ] ) };
		const Cand* op[ 2 ] = { cand_of( list[ 1 ][ o1 ] ), cand_of( list[ 1 ][ o2 ] ) };
		if( !ep[ 0 ] || !ep[ 1 ] || !op[ 0 ] || !op[ 1 ] ) continue;
		double total = 0;
		for( int j = 0; j < 8; ++j ) {
			double weff = colw[ j ];
			double bj = 0;
			int ba = 0, bb = 0;
			bool fj = true;
			for( int a = 0; a < 2; ++a ) {
				for( int b = 0; b < 2; ++b ) {
					ITP sum = itp_of( ep[ a ]->r + op[ b ]->r, ep[ a ]->g + op[ b ]->g, ep[ a ]->b + op[ b ]->b );
					double e = dist2_product( t1[ j ], ep[ a ]->itp ) + dist2_product( t2[ j ], op[ b ]->itp );
					if( weff != 0.0 ) e += weff * dist2_product( ts[ j ], sum );
					if( fj || e < bj ) { bj = e; ba = a; bb = b; fj = false; }
				}
			}
			total += bj;
			assign[ 0 ][ j ] = ep[ ba ]->idx;
			assign[ 1 ][ j ] = op[ bb ]->idx;
		}
		if( first || total < best_total ) {
			best_total = total;
			first = false;
			memcpy( best_assign, assign, sizeof( assign ) );
			best_pair[ 0 ][ 0 ] = list[ 0 ][ e1 ];
			best_pair[ 0 ][ 1 ] = list[ 0 ][ e2 ];
			best_pair[ 1 ][ 0 ] = list[ 1 ][ o1 ];
			best_pair[ 1 ][ 1 ] = list[ 1 ][ o2 ];
		}
	}
}

struct V3FitStats {
	int blocks_in = 0, blocks_out = 0, blocks_bnd = 0, bnd_pair = 0, bnd_line = 0;
};

// One fitting candidate for an 8x2 block: the two palette indices
// chosen per line plus the rendered palette index of every pixel.
struct FitCand {
	int first[ 2 ], second[ 2 ];
	int assign[ 2 ][ 8 ];
};

// v3 selective fitting: blocks fully inside the mask take the
// pair-aware fit, blocks fully outside take the conventional per-line
// fit, and boundary blocks evaluate BOTH candidates and keep the one
// with the smaller true (per-pixel, product-distance) error over the
// 8x2 block -- the author's boundary rule.  Ties go to the pair fit
// when at least half the block is masked, else to the line fit.
static void enforce_v3( const C_COLOR* in, const std::vector< unsigned char >& idx,
						int w, int h, const SETTING* Mode, C_COLOR* pal, double weight,
						const std::vector< char >& mask, const std::vector< char >& pairmode,
						unsigned char* vram, V3FitStats* fst ) {
	memset( vram, 0, 0x4000 );
	ITP palitp[ 16 ];
	for( int i = 0; i < 16; ++i )
		palitp[ i ] = itp_of( GET_RED( pal[ i ] ), GET_GREEN( pal[ i ] ), GET_BLUE( pal[ i ] ) );
	auto block_error = [ & ]( int yy, int cx, const FitCand& c ) {
		double e = 0;
		for( int ln = 0; ln < 2; ++ln )
			for( int j = 0; j < 8; ++j ) {
				C_COLOR px = in[ (size_t) ( yy + ln ) * w + cx * 8 + j ];
				ITP tp = itp_of( GET_RED( px ), GET_GREEN( px ), GET_BLUE( px ) );
				e += dist2_product( tp, palitp[ c.assign[ ln ][ j ] & 0x0F ] );
			}
		return e;
	};
	for( int y = 0; y + 1 < h; y += 2 ) {
		for( int cx = 0; cx < 32; ++cx ) {
			int nmask = 0;
			for( int ln = 0; ln < 2; ++ln )
				for( int j = 0; j < 8; ++j )
					if( mask[ (size_t) ( y + ln ) * w + cx * 8 + j ] ) nmask++;
			double colw[ 8 ];
			for( int j = 0; j < 8; ++j )
				colw[ j ] = pairmode[ (size_t) ( y / 2 ) * w + cx * 8 + j ] ? weight : 0.0;
			FitCand pcand, lcand, win;
			if( nmask > 0 ) {
				int bp[ 2 ][ 2 ];
				fit_pair_block( in, idx, w, y, cx, Mode, pal, colw, bp, pcand.assign );
				for( int ln = 0; ln < 2; ++ln ) {
					pcand.first[ ln ] = bp[ ln ][ 0 ];
					pcand.second[ ln ] = bp[ ln ][ 1 ];
				}
			}
			if( nmask < 16 ) {
				for( int ln = 0; ln < 2; ++ln )
					fit_line_cell( idx, w, y + ln, cx, pal, &lcand.first[ ln ], &lcand.second[ ln ], lcand.assign[ ln ] );
			}
			if( nmask == 16 ) {
				win = pcand;
				if( fst ) fst->blocks_in++;
			} else if( nmask == 0 ) {
				win = lcand;
				if( fst ) fst->blocks_out++;
			} else {
				bool pair_wins;
				int fb = flat_bnd_mode();
				if( fb >= 0 ) {
					pair_wins = ( fb == 0 );
				} else {
					double ep = block_error( y, cx, pcand );
					double el = block_error( y, cx, lcand );
					pair_wins = ( ep < el || ( ep == el && nmask >= 8 ) );
				}
				win = pair_wins ? pcand : lcand;
				if( fst ) {
					fst->blocks_bnd++;
					if( pair_wins ) fst->bnd_pair++; else fst->bnd_line++;
				}
			}
			for( int ln = 0; ln < 2; ++ln ) {
				int yy = y + ln;
				int bits = 0;
				for( int j = 0; j < 8; ++j ) {
					bits <<= 1;
					if( win.assign[ ln ][ j ] == win.first[ ln ] ) bits |= 1;
				}
				int dadr = ( yy & 7 ) + ( cx * 8 ) + ( yy >> 3 ) * 256;
				emit_cell( vram, dadr, pal, win.first[ ln ], win.second[ ln ], bits );
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Module state for dumps (single-threaded CLI; convert calls are
// strictly before -> convert -> after).
// ---------------------------------------------------------------------------
static std::vector< unsigned char > g_shadow_idx;
static bool g_shadow_ok = false;
static std::vector< unsigned char > g_pair_idx;
static bool g_pair_ran = false;
static std::vector< float > g_edge;
static bool g_edge_ok = false;
static float g_edge_scale = 0.0f;
static std::vector< char > g_flat_mask;
static bool g_flat_ok = false;
static double g_flat_coverage = 0.0;
static bool g_flat_used_shadow = false;
static V3FitStats g_v3fit;

void sc2_dump_before( const SETTING& Mode, C_COLOR* target, int width, int height, C_COLOR* pal ) {
	g_pair_ran = false;
	g_shadow_ok = false;
	g_shadow_idx.clear();
	g_pair_idx.clear();
	g_edge_ok = false;
	g_edge.clear();
	g_flat_ok = false;
	g_flat_mask.clear();
	g_v3fit = V3FitStats();
	std::string dir = dump_dir();
	if( dir.empty() ) return;
	std::error_code ec;
	std::filesystem::create_directories( dir, ec );

	std::vector< unsigned char > rgb;
	target_rgb( target, width, height, rgb );
	write_ppm( dir + "/target.ppm", rgb.data(), width, height );

	FILE* f = fopen( ( dir + "/pal.txt" ).c_str(), "w" );
	if( f ) {
		for( int i = 0; i < 16; ++i ) {
			fprintf( f, "%d %d %d %d %d\n", i, (int) GET_RED( pal[ i ] ), (int) GET_GREEN( pal[ i ] ),
					 (int) GET_BLUE( pal[ i ] ), (int) Mode.PalEn[ i ] );
		}
		fclose( f );
	}

	g_shadow_ok = shadow_quantize( target, width, height, &Mode, pal, g_shadow_idx );
	if( g_shadow_ok ) {
		write_index_bin( dir + "/shadow_index.bin", g_shadow_idx, width, height );
	}
}

void sc2_dump_after( const SETTING& Mode, const unsigned char* bmp, C_COLOR* pal ) {
	std::string dir = dump_dir();
	if( dir.empty() ) return;
	std::error_code ec;
	std::filesystem::create_directories( dir, ec );

	int w = 256, h = 192;
	std::vector< unsigned char > rgb, ridx;
	decode_vram( bmp, pal, w, h, rgb, ridx );
	write_ppm( dir + "/rendered.ppm", rgb.data(), w, h );
	write_index_bin( dir + "/rendered_index.bin", ridx, w, h );

	bool distinct_seen[ 16 ] = {};
	int distinct = 0;
	for( unsigned char v : ridx ) if( !distinct_seen[ v ] ) { distinct_seen[ v ] = true; distinct++; }

	FILE* f = fopen( ( dir + "/stats.txt" ).c_str(), "w" );
	if( !f ) return;
	fprintf( f, "label=%s\n", run_label().c_str() );
	fprintf( f, "screen_mode=%u\n", (unsigned) Mode.screen_mode );
	fprintf( f, "pair_env=%d\n", sc2_pair_env_set() ? 1 : 0 );
	if( sc2_pair_env_set() ) {
		fprintf( f, "pair_w=%.4g\n", pair_w() );
		fprintf( f, "fit=%s\n", fit_is_pair() ? "pair" : "line" );
		fprintf( f, "edge=%d\n", edge_enabled() ? 1 : 0 );
		if( edge_enabled() ) {
			fprintf( f, "edge_pct=%.4g\n", edge_pct() );
			fprintf( f, "edge_dilate=%d\n", edge_dilate() );
			fprintf( f, "edge_lo=%.4g\n", edge_lo() );
		}
		fprintf( f, "flat=%d\n", flat_enabled() ? 1 : 0 );
		if( flat_enabled() ) {
			fprintf( f, "flat_range=%.4g\n", flat_range() );
			fprintf( f, "flat_out=%s\n", flat_out_shadow() ? "shadow" : "nearest" );
		}
	}
	if( g_flat_ok ) {
		fprintf( f, "flat_coverage=%.4f\n", g_flat_coverage );
		fprintf( f, "flat_used_shadow=%d\n", g_flat_used_shadow ? 1 : 0 );
		fprintf( f, "flat_blocks_in=%d\nflat_blocks_out=%d\nflat_blocks_bnd=%d\nflat_bnd_pair=%d\nflat_bnd_line=%d\n",
				 g_v3fit.blocks_in, g_v3fit.blocks_out, g_v3fit.blocks_bnd, g_v3fit.bnd_pair, g_v3fit.bnd_line );
	}
	if( g_edge_ok ) {
		double sum = 0.0;
		int ge_half = 0;
		for( float v : g_edge ) {
			sum += v;
			if( v >= 0.5f ) ge_half++;
		}
		fprintf( f, "edge_scale=%.1f\n", (double) g_edge_scale );
		fprintf( f, "edge_mean=%.4f\n", g_edge.empty() ? 0.0 : sum / (double) g_edge.size() );
		fprintf( f, "edge_frac_ge_half=%.4f\n", g_edge.empty() ? 0.0 : (double) ge_half / (double) g_edge.size() );
	}
	fprintf( f, "diffusion=%d coef=%.3f x_weight=%.3f cuterr=%u\n",
			 Mode.diffusion_error_enable ? 1 : 0, (double) Mode.diffusion_error_coef,
			 (double) Mode.diffusion_error_x_weight, (unsigned) Mode.err );
	fprintf( f, "distinct_rendered=%d\n", distinct );
	fprintf( f, "shadow_ok=%d\n", g_shadow_ok ? 1 : 0 );
	if( g_shadow_ok ) {
		DemandStats ds = demand_stats( g_shadow_idx, w, h );
		fprintf( f, "shadow_cells_eq1=%d\nshadow_cells_eq2=%d\nshadow_cells_ge3=%d\nshadow_excess=%d\n",
				 ds.cells_eq1, ds.cells_eq2, ds.cells_ge3, ds.excess );
		// Validate the shadow: enforce it and compare against the real
		// product VRAM.  Only meaningful when the pair path did not run.
		if( !g_pair_ran ) {
			std::vector< unsigned char > svram( 0x4000 );
			enforce_line_product( g_shadow_idx, w, h, pal, svram.data() );
			int match = memcmp( svram.data(), bmp, 0x4000 ) == 0;
			fprintf( f, "shadow_vram_match=%d\n", match );
		}
	}
	if( g_pair_ran ) {
		DemandStats ds = demand_stats( g_pair_idx, w, h );
		fprintf( f, "pair_cells_eq1=%d\npair_cells_eq2=%d\npair_cells_ge3=%d\npair_excess=%d\n",
				 ds.cells_eq1, ds.cells_eq2, ds.cells_ge3, ds.excess );
	}
	fclose( f );
}

// ---------------------------------------------------------------------------
// The prototype conversion itself.
// ---------------------------------------------------------------------------
bool sc2_pair_convert( C_COLOR* in, int width, int height, unsigned char* out,
					   SETTING* Mode, C_COLOR* pal, std::string* err ) {
	if( width != 256 || height != 192 ) {
		if( err ) *err = "sc2_pair prototype: expected a 256x192 target";
		return false;
	}
	double wgt = pair_w();
	const float* emap = nullptr;
	g_edge_ok = false;
	g_edge.clear();
	if( edge_enabled() ) {
		g_edge = compute_edge_map( in, width, height, edge_pct(), edge_dilate(), edge_lo(), &g_edge_scale );
		g_edge_ok = true;
		emap = g_edge.data();
		if( sc2_dump_env_set() ) {
			std::string path = dump_dir() + "/edge.pgm";
			FILE* f = fopen( path.c_str(), "wb" );
			if( f ) {
				fprintf( f, "P5\n%d %d\n255\n", width, height );
				for( float v : g_edge ) {
					unsigned char b = (unsigned char) ( v * 255.0f + 0.5f );
					fwrite( &b, 1, 1, f );
				}
				fclose( f );
			}
		}
	}
	std::vector< unsigned char > idx;
	std::vector< unsigned char > vram( 0x4000 );
	if( flat_enabled() ) {
		// v3: selective pair processing inside the flat mask only.
		// (If EDGE is also set it is ignored in this mode.)
		double cov = 0.0;
		g_flat_mask = compute_flat_mask( in, width, height, flat_range(), &cov );
		g_flat_ok = true;
		g_flat_coverage = cov;
		std::vector< char > pairmode;
		quantize_v3( in, width, height, Mode, pal, wgt, g_flat_mask, pairmode, idx, &g_flat_used_shadow );
		g_v3fit = V3FitStats();
		enforce_v3( in, idx, width, height, Mode, pal, wgt, g_flat_mask, pairmode, vram.data(), &g_v3fit );
		if( sc2_dump_env_set() ) {
			std::string path = dump_dir() + "/mask.pgm";
			FILE* f = fopen( path.c_str(), "wb" );
			if( f ) {
				fprintf( f, "P5\n%d %d\n255\n", width, height );
				for( char v : g_flat_mask ) {
					unsigned char b = v ? 255 : 0;
					fwrite( &b, 1, 1, f );
				}
				fclose( f );
			}
		}
	} else {
		pair_quantize( in, width, height, Mode, pal, wgt, emap, idx );
		if( fit_is_pair() ) {
			enforce_pair_aware( in, idx, width, height, Mode, pal, wgt, emap, vram.data() );
		} else {
			enforce_line_product( idx, width, height, pal, vram.data() );
		}
	}
	memcpy( out, vram.data(), 0x4000 );

	g_pair_idx = idx;
	g_pair_ran = true;
	if( sc2_dump_env_set() ) {
		write_index_bin( dump_dir() + "/index.bin", idx, width, height );
	}
	return true;
}
