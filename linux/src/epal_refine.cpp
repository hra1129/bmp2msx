// See epal_refine.h.  This is an experimental, environment-gated
// prototype; nothing in this file runs unless the variables documented
// there are set.
#include "epal_refine.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
// Environment (parsed once, lazily)

static bool   g_parsed  = false;
static bool   g_enabled = false;	// BMP2MSX_EPAL_REFINE (swap refinement)
static double g_alpha   = 0.5;
static int    g_maxiter = 32;
static double g_guard   = 0.01;
static bool   g_redmean = false;
static double g_satbeta = 0.0;		// BMP2MSX_EPAL_REFINE_SATBETA
static bool   g_debug   = false;
static const char* g_histdump = nullptr;
// High-pass bonus (BMP2MSX_EPAL_HPBONUS)
static bool   g_hp_enabled = false;
static double g_hp_beta = 0.0;
static double g_hp_dz   = 6.0;
static int    g_hp_radius = 2;
static bool   g_hp_debug = false;
// Accent reservation (BMP2MSX_EPAL_ACCENT)
static int    g_accent_k = 0;
static double g_acc_sat    = 0.35;
static double g_acc_satbin = 0.30;
static double g_acc_dist   = 8.0;
static int    g_acc_minpix = 8;
static int    g_acc_vmin   = 10;
static double g_acc_radius = 5.0;
static bool   g_acc_debug  = false;
// Chroma-change saliency bonus (BMP2MSX_EPAL_UVBONUS)
static bool   g_uv_enabled = false;
static double g_uv_beta = 0.0;
static int    g_uv_mode = 0;		// 0 = UV-MAG (author), 1 = UV-VEC, 2 = Y, 3 = UVY
static bool   g_uv_mode_explicit = false;
static double g_uv_dz   = 2.0;
static double g_uv_ydz  = 6.0;
static int    g_uv_radius = 2;
static bool   g_uv_debug = false;
static const char* g_uv_dump = nullptr;
// Compressed-domain histogram + saliency bonus (BMP2MSX_EPAL_SQRTHIST)
static bool   g_sqrt_enabled = false;
static double g_sqrt_k = 0.0;
static bool   g_sqrt_debug = false;

static double env_double( const char* name, double dflt, double lo )
{
	const char* s = getenv( name );
	if( s == nullptr || *s == '\0' ) return dflt;
	char* end = nullptr;
	double v = strtod( s, &end );
	if( end == s || v < lo ) return dflt;
	return v;
}

static long env_long( const char* name, long dflt, long lo, long hi )
{
	const char* s = getenv( name );
	if( s == nullptr || *s == '\0' ) return dflt;
	long v = strtol( s, nullptr, 10 );
	if( v < lo || v > hi ) return dflt;
	return v;
}

static void epal_parse_env()
{
	if( g_parsed ) return;
	g_parsed = true;

	const char* s = getenv( "BMP2MSX_EPAL_REFINE" );
	if( s != nullptr && *s != '\0' ) {
		char* end = nullptr;
		double a = strtod( s, &end );
		if( end != s ) {
			g_enabled = true;
			g_alpha = a;
			if( g_alpha < 0.0 ) g_alpha = 0.0;
			if( g_alpha > 1.0 ) g_alpha = 1.0;
		}
	}
	s = getenv( "BMP2MSX_EPAL_REFINE_MAXITER" );
	if( s != nullptr && *s != '\0' ) {
		long n = strtol( s, nullptr, 10 );
		if( n >= 0 && n <= 10000 ) g_maxiter = (int) n;
	}
	s = getenv( "BMP2MSX_EPAL_REFINE_GUARD" );
	if( s != nullptr && *s != '\0' ) {
		char* end = nullptr;
		double v = strtod( s, &end );
		if( end != s && v >= 0.0 ) g_guard = v;
	}
	s = getenv( "BMP2MSX_EPAL_REFINE_REDMAN" );
	if( s != nullptr && !strcmp( s, "1" ) ) g_redmean = true;
	g_satbeta = env_double( "BMP2MSX_EPAL_REFINE_SATBETA", 0.0, 0.0 );
	s = getenv( "BMP2MSX_EPAL_REFINE_DEBUG" );
	if( s != nullptr && !strcmp( s, "1" ) ) g_debug = true;
	g_histdump = getenv( "BMP2MSX_EPAL_HISTDUMP" );
	if( g_histdump != nullptr && *g_histdump == '\0' ) g_histdump = nullptr;

	s = getenv( "BMP2MSX_EPAL_HPBONUS" );
	if( s != nullptr && *s != '\0' ) {
		char* end = nullptr;
		double v = strtod( s, &end );
		if( end != s && v >= 0.0 ) {
			g_hp_enabled = true;
			g_hp_beta = v;
		}
	}
	g_hp_dz = env_double( "BMP2MSX_EPAL_HP_DZ", 6.0, 0.0 );
	g_hp_radius = (int) env_long( "BMP2MSX_EPAL_HP_RADIUS", 2, 1, 8 );
	s = getenv( "BMP2MSX_EPAL_HP_DEBUG" );
	if( s != nullptr && !strcmp( s, "1" ) ) g_hp_debug = true;

	g_accent_k = (int) env_long( "BMP2MSX_EPAL_ACCENT", 0, 0, 256 );
	g_acc_sat    = env_double( "BMP2MSX_EPAL_ACCENT_SAT",    0.35, 0.0 );
	g_acc_satbin = env_double( "BMP2MSX_EPAL_ACCENT_SATBIN", 0.30, 0.0 );
	g_acc_dist   = env_double( "BMP2MSX_EPAL_ACCENT_DIST",   8.0,  0.0 );
	g_acc_minpix = (int) env_long( "BMP2MSX_EPAL_ACCENT_MINPIX", 8, 1, 1000000 );
	g_acc_vmin   = (int) env_long( "BMP2MSX_EPAL_ACCENT_VMIN",  10, 0, 31 );
	g_acc_radius = env_double( "BMP2MSX_EPAL_ACCENT_RADIUS", 5.0, 0.0 );
	s = getenv( "BMP2MSX_EPAL_ACCENT_DEBUG" );
	if( s != nullptr && !strcmp( s, "1" ) ) g_acc_debug = true;

	s = getenv( "BMP2MSX_EPAL_UVBONUS" );
	if( s != nullptr && *s != '\0' ) {
		char* end = nullptr;
		double v = strtod( s, &end );
		if( end != s && v >= 0.0 ) {
			g_uv_enabled = true;
			g_uv_beta = v;
		}
	}
	s = getenv( "BMP2MSX_EPAL_UV_MODE" );
	if( s != nullptr && *s != '\0' ) {
		g_uv_mode_explicit = true;
		if( !strcmp( s, "vec" ) ) g_uv_mode = 1;
		else if( !strcmp( s, "y" ) ) g_uv_mode = 2;
		else if( !strcmp( s, "uvy" ) ) g_uv_mode = 3;
		else g_uv_mode = 0;
	}
	g_uv_dz = env_double( "BMP2MSX_EPAL_UV_DZ", 2.0, 0.0 );
	g_uv_ydz = env_double( "BMP2MSX_EPAL_UV_YDZ", 6.0, 0.0 );
	g_uv_radius = (int) env_long( "BMP2MSX_EPAL_UV_RADIUS", 2, 1, 8 );
	s = getenv( "BMP2MSX_EPAL_UV_DEBUG" );
	if( s != nullptr && !strcmp( s, "1" ) ) g_uv_debug = true;
	g_uv_dump = getenv( "BMP2MSX_EPAL_UV_DUMP" );
	if( g_uv_dump != nullptr && *g_uv_dump == '\0' ) g_uv_dump = nullptr;

	s = getenv( "BMP2MSX_EPAL_SQRTHIST" );
	if( s != nullptr && *s != '\0' ) {
		char* end = nullptr;
		double v = strtod( s, &end );
		if( end != s && v >= 0.0 ) {
			g_sqrt_enabled = true;
			g_sqrt_k = v;
		}
	}
	s = getenv( "BMP2MSX_EPAL_SQRTHIST_DEBUG" );
	if( s != nullptr && !strcmp( s, "1" ) ) g_sqrt_debug = true;
}

bool epal_refine_enabled()
{
	epal_parse_env();
	return g_enabled;
}

bool epal_accent_enabled()
{
	epal_parse_env();
	return g_accent_k > 0;
}

bool epal_hp_enabled()
{
	epal_parse_env();
	return g_hp_enabled;
}

bool epal_uv_enabled()
{
	epal_parse_env();
	return g_uv_enabled;
}

bool epal_sqrthist_enabled()
{
	epal_parse_env();
	return g_sqrt_enabled;
}

bool epal_experiment_enabled()
{
	epal_parse_env();
	return g_enabled || g_accent_k > 0 || g_hp_enabled || g_uv_enabled || g_sqrt_enabled;
}

int epal_accent_k()
{
	epal_parse_env();
	return g_accent_k;
}

// ---------------------------------------------------------------------------
// RGB555 helpers (mirror Convert.cpp's epal_quantize31 / cnvEpalTo255)

static void epal_table8( int tbl[ 32 ] )
{
	for( int v = 0; v < 32; ++v ) tbl[ v ] = cnvEpalTo255( v );
}

void epal_color_to_5bit( C_COLOR c, int* r5, int* g5, int* b5 )
{
	int tbl[ 32 ];
	epal_table8( tbl );
	*r5 = convert_rgb_to_palette( tbl, 32, GET_RED( c ) );
	*g5 = convert_rgb_to_palette( tbl, 32, GET_GREEN( c ) );
	*b5 = convert_rgb_to_palette( tbl, 32, GET_BLUE( c ) );
}

static inline int epal_bin_index( const int tbl[ 32 ], C_COLOR c )
{
	int r = convert_rgb_to_palette( tbl, 32, GET_RED( c ) );
	int g = convert_rgb_to_palette( tbl, 32, GET_GREEN( c ) );
	int b = convert_rgb_to_palette( tbl, 32, GET_BLUE( c ) );
	return ( r << 10 ) | ( g << 5 ) | b;
}

uint32_t* epal_build_hist555( const C_COLOR* in, int w, int h,
							  bool skip_fz, C_COLOR fz_color )
{
	int tbl[ 32 ];
	epal_table8( tbl );
	uint32_t* hist = (uint32_t*) calloc( 32768, sizeof( uint32_t ) );
	if( hist == nullptr ) return nullptr;
	for( int i = 0; i < w * h; ++i ) {
		C_COLOR c = in[ i ];
		if( skip_fz && c == fz_color ) continue;
		hist[ epal_bin_index( tbl, c ) ]++;
	}
	return hist;
}

double* epal_build_histw555( const C_COLOR* in, int w, int h,
							 bool skip_fz, C_COLOR fz_color,
							 const double* pixw )
{
	int tbl[ 32 ];
	epal_table8( tbl );
	double* hist = (double*) calloc( 32768, sizeof( double ) );
	if( hist == nullptr ) return nullptr;
	for( int i = 0; i < w * h; ++i ) {
		C_COLOR c = in[ i ];
		if( skip_fz && c == fz_color ) continue;
		hist[ epal_bin_index( tbl, c ) ] += ( pixw != nullptr ) ? pixw[ i ] : 1.0;
	}
	return hist;
}

void epal_maybe_histdump( const C_COLOR* in, int w, int h )
{
	epal_parse_env();
	// Lab-only per-pixel dump (BMP2MSX_EPAL_PIXDUMP): uint32 w, uint32 h,
	// then w*h uint16 RGB555 bin indices of the resized selection image.
	// Output-neutral, like HISTDUMP; used for offline seed simulations.
	const char* pixdump = getenv( "BMP2MSX_EPAL_PIXDUMP" );
	if( pixdump != nullptr ) {
		int tbl[ 32 ];
		epal_table8( tbl );
		FILE* pf = fopen( pixdump, "wb" );
		if( pf != nullptr ) {
			uint32_t wh[ 2 ] = { (uint32_t) w, (uint32_t) h };
			fwrite( wh, sizeof( uint32_t ), 2, pf );
			for( int i = 0; i < w * h; ++i ) {
				uint16_t b = (uint16_t) epal_bin_index( tbl, in[ i ] );
				fwrite( &b, sizeof( uint16_t ), 1, pf );
			}
			fclose( pf );
		}
	}
	if( g_histdump == nullptr ) return;
	uint32_t* hist = epal_build_hist555( in, w, h, false, 0 );
	if( hist == nullptr ) return;
	FILE* f = fopen( g_histdump, "wb" );
	if( f != nullptr ) {
		uint32_t wh[ 2 ] = { (uint32_t) w, (uint32_t) h };
		fwrite( wh, sizeof( uint32_t ), 2, f );
		fwrite( hist, sizeof( uint32_t ), 32768, f );
		fclose( f );
	}
	free( hist );
}

// ---------------------------------------------------------------------------
// High-pass bonus weights (the author's scheme)
//
// For each channel independently, response = |value - box blur(value)|
// (blur radius g_hp_radius, borders clamped).  The pixel response is
// the maximum over the three channels, in 8-bit units.  A dead zone
// (g_hp_dz) is subtracted so JPEG noise in flat areas does not respond,
// and the result is normalized by the 99th percentile of the positive
// responses and clamped to [0,1].  Weight = 1 + beta * normalized.

double* epal_hp_pixel_weights( const C_COLOR* in, int w, int h )
{
	epal_parse_env();
	if( !g_hp_enabled ) return nullptr;
	const int npix = w * h;
	double* weights = (double*) malloc( sizeof( double ) * npix );
	double* resp = (double*) malloc( sizeof( double ) * npix );
	double* blur = (double*) malloc( sizeof( double ) * npix );
	double* tmp  = (double*) malloc( sizeof( double ) * npix );
	if( !weights || !resp || !blur || !tmp ) {
		free( weights ); free( resp ); free( blur ); free( tmp );
		return nullptr;
	}
	for( int i = 0; i < npix; ++i ) resp[ i ] = 0.0;

	const int R = g_hp_radius;
	for( int ch = 0; ch < 3; ++ch ) {
		// Extract the channel as doubles.
		for( int i = 0; i < npix; ++i ) {
			C_COLOR c = in[ i ];
			tmp[ i ] = ( ch == 0 ) ? GET_RED( c ) : ( ch == 1 ) ? GET_GREEN( c ) : GET_BLUE( c );
		}
		// Separable box blur, clamped borders: tmp -> blur.
		for( int y = 0; y < h; ++y ) {
			for( int x = 0; x < w; ++x ) {
				double s = 0.0;
				int cnt = 0;
				for( int dx = -R; dx <= R; ++dx ) {
					int xx = x + dx;
					if( xx < 0 ) xx = 0;
					if( xx >= w ) xx = w - 1;
					s += tmp[ y * w + xx ];
					++cnt;
				}
				blur[ y * w + x ] = s / cnt;
			}
		}
		for( int y = 0; y < h; ++y ) {
			for( int x = 0; x < w; ++x ) {
				double s = 0.0;
				int cnt = 0;
				for( int dy = -R; dy <= R; ++dy ) {
					int yy = y + dy;
					if( yy < 0 ) yy = 0;
					if( yy >= h ) yy = h - 1;
					s += blur[ yy * w + x ];
					++cnt;
				}
				double r = fabs( tmp[ y * w + x ] - s / cnt );
				if( r > resp[ y * w + x ] ) resp[ y * w + x ] = r;
			}
		}
	}
	free( blur );
	free( tmp );

	// Dead zone + percentile normalization.
	std::vector< double > pos;
	pos.reserve( npix );
	double rmax = 0.0;
	long nresp = 0;
	for( int i = 0; i < npix; ++i ) {
		double r = resp[ i ] - g_hp_dz;
		if( r > 0.0 ) {
			pos.push_back( r );
			if( r > rmax ) rmax = r;
			++nresp;
		}
		resp[ i ] = ( r > 0.0 ) ? r : 0.0;
	}
	double p99 = 0.0;
	if( !pos.empty() ) {
		std::sort( pos.begin(), pos.end() );
		p99 = pos[ (size_t) ( 0.99 * (double) ( pos.size() - 1 ) ) ];
	}
	for( int i = 0; i < npix; ++i ) {
		double n = ( p99 > 0.0 ) ? resp[ i ] / p99 : 0.0;
		if( n > 1.0 ) n = 1.0;
		weights[ i ] = 1.0 + g_hp_beta * n;
	}
	if( g_hp_debug ) {
		fprintf( stderr, "[epal-hp] %dx%d beta=%.2f dz=%.1f radius=%d responding=%ld (%.1f%%) p99=%.1f max=%.1f\n",
				 w, h, g_hp_beta, g_hp_dz, R, nresp,
				 npix ? 100.0 * (double) nresp / npix : 0.0, p99, rmax );
	}
	free( resp );
	return weights;
}

// ---------------------------------------------------------------------------
// Chroma-change saliency weights (scheme 4: UV-MAG / UV-VEC)
//
// The image is converted to YUV (BT.601):
//   Y = 0.299 R + 0.587 G + 0.114 B
//   U = 0.492111 (B - Y),  V = 0.877283 (R - Y)     (R,G,B in 0..255)
// UV-MAG (the author's scheme): M = |(U,V)| = sqrt(U^2 + V^2) is the
// chroma-magnitude map; response = |M - boxblur(M)|.  UV-VEC: the
// high-pass is taken per component and combined as a vector length,
// response = sqrt( (U - blur U)^2 + (V - blur V)^2 ), which also
// fires on hue flips that keep |(U,V)| constant.  Blur radius,
// dead zone and p99 normalization mirror the HP bonus; the weight is
// w = 1 + beta * normalized response.

// Separable box blur with clamped borders: src -> dst (tmp is scratch
// of the same size).
static void box_blur_d( const double* src, double* dst, double* tmp,
						int w, int h, int R )
{
	for( int y = 0; y < h; ++y ) {
		for( int x = 0; x < w; ++x ) {
			double s = 0.0;
			for( int dx = -R; dx <= R; ++dx ) {
				int xx = x + dx;
				if( xx < 0 ) xx = 0;
				if( xx >= w ) xx = w - 1;
				s += src[ y * w + xx ];
			}
			tmp[ y * w + x ] = s / ( 2 * R + 1 );
		}
	}
	for( int y = 0; y < h; ++y ) {
		for( int x = 0; x < w; ++x ) {
			double s = 0.0;
			for( int dy = -R; dy <= R; ++dy ) {
				int yy = y + dy;
				if( yy < 0 ) yy = 0;
				if( yy >= h ) yy = h - 1;
				s += tmp[ yy * w + x ];
			}
			dst[ y * w + x ] = s / ( 2 * R + 1 );
		}
	}
}

// Dead zone + p99 normalization, in place: resp becomes the
// normalized response in [0,1] (same recipe as the HP bonus).
// Returns the number of pixels whose response exceeded the dead zone.
static long normalize_response( double* resp, int npix, double dz,
								double* p99_out, double* rmax_out )
{
	std::vector< double > pos;
	pos.reserve( npix );
	double rmax = 0.0;
	long nresp = 0;
	for( int i = 0; i < npix; ++i ) {
		double r = resp[ i ] - dz;
		if( r > 0.0 ) {
			pos.push_back( r );
			if( r > rmax ) rmax = r;
			++nresp;
		}
		resp[ i ] = ( r > 0.0 ) ? r : 0.0;
	}
	double p99 = 0.0;
	if( !pos.empty() ) {
		std::sort( pos.begin(), pos.end() );
		p99 = pos[ (size_t) ( 0.99 * (double) ( pos.size() - 1 ) ) ];
	}
	for( int i = 0; i < npix; ++i ) {
		double n = ( p99 > 0.0 ) ? resp[ i ] / p99 : 0.0;
		if( n > 1.0 ) n = 1.0;
		resp[ i ] = n;
	}
	*p99_out = p99;
	*rmax_out = rmax;
	return nresp;
}

// The response carrier: an explicitly selected BMP2MSX_EPAL_UV_MODE
// wins; otherwise the sqrt-histogram scheme (whose primary carrier is
// UVY) defaults to UVY and the plain UV bonus keeps UV-MAG.
static int uv_effective_mode()
{
	if( g_uv_mode_explicit ) return g_uv_mode;
	return g_sqrt_enabled ? 3 : 0;
}

double* epal_uv_response_map( const C_COLOR* in, int w, int h )
{
	epal_parse_env();
	if( !g_uv_enabled && !g_sqrt_enabled ) return nullptr;
	const int mode = uv_effective_mode();
	const int npix = w * h;
	const int R = g_uv_radius;
	double* resp = (double*) malloc( sizeof( double ) * npix );
	double* fld  = (double*) calloc( npix, sizeof( double ) );
	double* blur = (double*) calloc( npix, sizeof( double ) );
	double* tmp  = (double*) calloc( npix, sizeof( double ) );
	double* uu   = (double*) calloc( npix, sizeof( double ) );
	double* vv   = (double*) calloc( npix, sizeof( double ) );
	if( !resp || !fld || !blur || !tmp || !uu || !vv ) {
		free( resp ); free( fld ); free( blur );
		free( tmp ); free( uu ); free( vv );
		return nullptr;
	}
	// fld holds Y for now (reused as the M map below).
	for( int i = 0; i < npix; ++i ) {
		C_COLOR c = in[ i ];
		double r = GET_RED( c ), g = GET_GREEN( c ), b = GET_BLUE( c );
		double y = 0.299 * r + 0.587 * g + 0.114 * b;
		fld[ i ] = y;
		uu[ i ] = 0.492111 * ( b - y );
		vv[ i ] = 0.877283 * ( r - y );
	}
	double p99 = 0.0, rmax = 0.0;
	long nresp = 0;
	const char* tag = "-mag";
	if( mode == 1 ) {
		// UV-VEC: magnitude of the chroma vector's local change.
		tag = "-vec";
		box_blur_d( uu, blur, tmp, w, h, R );
		for( int i = 0; i < npix; ++i ) resp[ i ] = ( uu[ i ] - blur[ i ] ) * ( uu[ i ] - blur[ i ] );
		box_blur_d( vv, blur, tmp, w, h, R );
		for( int i = 0; i < npix; ++i ) resp[ i ] = sqrt( resp[ i ] + ( vv[ i ] - blur[ i ] ) * ( vv[ i ] - blur[ i ] ) );
		nresp = normalize_response( resp, npix, g_uv_dz, &p99, &rmax );
	} else if( mode == 2 ) {
		// Y: luminance high-pass only (fld already holds Y).
		tag = "-y";
		box_blur_d( fld, blur, tmp, w, h, R );
		for( int i = 0; i < npix; ++i ) resp[ i ] = fabs( fld[ i ] - blur[ i ] );
		nresp = normalize_response( resp, npix, g_uv_ydz, &p99, &rmax );
	} else if( mode == 3 ) {
		// UVY: UV-MAG and Y responses, each dead-zoned and normalized
		// on its own, combined as sqrt(nM^2 + nY^2) clamped to 1.
		// nM is parked in uu (U no longer needed).
		tag = "-uvy";
		for( int i = 0; i < npix; ++i ) vv[ i ] = sqrt( uu[ i ] * uu[ i ] + vv[ i ] * vv[ i ] );
		box_blur_d( vv, blur, tmp, w, h, R );
		for( int i = 0; i < npix; ++i ) resp[ i ] = fabs( vv[ i ] - blur[ i ] );
		double p99m = 0.0, rmaxm = 0.0;
		normalize_response( resp, npix, g_uv_dz, &p99m, &rmaxm );
		for( int i = 0; i < npix; ++i ) uu[ i ] = resp[ i ];	// nM
		box_blur_d( fld, blur, tmp, w, h, R );
		for( int i = 0; i < npix; ++i ) resp[ i ] = fabs( fld[ i ] - blur[ i ] );
		nresp = normalize_response( resp, npix, g_uv_ydz, &p99, &rmax );
		nresp = 0;
		for( int i = 0; i < npix; ++i ) {
			double n = sqrt( uu[ i ] * uu[ i ] + resp[ i ] * resp[ i ] );
			if( n > 1.0 ) n = 1.0;
			resp[ i ] = n;
			if( n > 0.0 ) ++nresp;
		}
	} else {
		// UV-MAG: high-pass of the chroma-magnitude map (fld is
		// overwritten with M; Y is not needed in this mode).
		for( int i = 0; i < npix; ++i ) fld[ i ] = sqrt( uu[ i ] * uu[ i ] + vv[ i ] * vv[ i ] );
		box_blur_d( fld, blur, tmp, w, h, R );
		for( int i = 0; i < npix; ++i ) resp[ i ] = fabs( fld[ i ] - blur[ i ] );
		nresp = normalize_response( resp, npix, g_uv_dz, &p99, &rmax );
	}
	free( fld ); free( blur ); free( tmp ); free( uu ); free( vv );

	long n25 = 0, n50 = 0;
	for( int i = 0; i < npix; ++i ) {
		double n = resp[ i ];
		if( n >= 0.25 ) ++n25;
		if( n >= 0.50 ) ++n50;
	}
	if( g_uv_debug ) {
		fprintf( stderr, "[epal-uv%s] %dx%d dz=%.1f radius=%d responding=%ld (%.1f%%) n>=0.25: %ld (%.1f%%) n>=0.50: %ld (%.1f%%) p99=%.1f max=%.1f\n",
				 tag, w, h,
				 ( mode == 2 ) ? g_uv_ydz : g_uv_dz, R,
				 nresp, npix ? 100.0 * (double) nresp / npix : 0.0,
				 n25, npix ? 100.0 * (double) n25 / npix : 0.0,
				 n50, npix ? 100.0 * (double) n50 / npix : 0.0, p99, rmax );
	}
	if( g_uv_dump != nullptr ) {
		FILE* f = fopen( g_uv_dump, "wb" );
		if( f != nullptr ) {
			uint32_t wh[ 2 ] = { (uint32_t) w, (uint32_t) h };
			fwrite( wh, sizeof( uint32_t ), 2, f );
			for( int i = 0; i < npix; ++i ) {
				uint8_t q = (uint8_t) ( resp[ i ] * 255.0 + 0.5 );
				fwrite( &q, 1, 1, f );
			}
			fclose( f );
		}
	}
	return resp;
}

double* epal_uv_pixel_weights( const C_COLOR* in, int w, int h )
{
	epal_parse_env();
	if( !g_uv_enabled ) return nullptr;
	double* resp = epal_uv_response_map( in, w, h );
	if( resp == nullptr ) return nullptr;
	const int npix = w * h;
	for( int i = 0; i < npix; ++i ) resp[ i ] = 1.0 + g_uv_beta * resp[ i ];
	return resp;
}

// ---------------------------------------------------------------------------
// Compressed-domain histogram + saliency bonus (scheme 5)
//
// h(c) = sqrt( n(c) ) + k * b(c): n(c) is the raw bin count, b(c) the
// saliency bonus mass = the sum of the normalized UV response over the
// bin's pixels (same response map as scheme 4, so b is in pixel-
// equivalent units: a bin of n pixels all responding at 1.0 has
// b = n).  Both terms see the same fixed-color / FourceZero
// exclusions as the stock selection.

double* epal_build_hist_sqrt555( const C_COLOR* in, int w, int h,
								 bool skip_fz, C_COLOR fz_color )
{
	epal_parse_env();
	if( !g_sqrt_enabled ) return nullptr;
	double* resp = epal_uv_response_map( in, w, h );
	uint32_t* hist = epal_build_hist555( in, w, h, skip_fz, fz_color );
	double* b = ( resp != nullptr ) ? epal_build_histw555( in, w, h, skip_fz, fz_color, resp )
									: nullptr;
	free( resp );
	if( hist == nullptr || b == nullptr ) {
		free( hist );
		free( b );
		return nullptr;
	}
	double* hh = (double*) malloc( sizeof( double ) * 32768 );
	if( hh == nullptr ) {
		free( hist );
		free( b );
		return nullptr;
	}
	double sumb = 0.0, maxb = 0.0, sumh = 0.0, sumn = 0.0;
	int argmaxb = -1;
	for( int i = 0; i < 32768; ++i ) {
		hh[ i ] = sqrt( (double) hist[ i ] ) + g_sqrt_k * b[ i ];
		sumb += b[ i ];
		sumn += (double) hist[ i ];
		sumh += hh[ i ];
		if( b[ i ] > maxb ) { maxb = b[ i ]; argmaxb = i; }
	}
	if( g_sqrt_debug ) {
		fprintf( stderr, "[epal-sqrt] %dx%d k=%.2f pixels=%.0f sum_b=%.1f max_b=%.1f at (%d,%d,%d) sum_h=%.1f\n",
				 w, h, g_sqrt_k, sumn, sumb, maxb,
				 ( argmaxb >> 10 ) & 31, ( argmaxb >> 5 ) & 31, argmaxb & 31, sumh );
	}
	free( hist );
	free( b );
	return hh;
}

// ---------------------------------------------------------------------------
// Weighted median cut (mirror of the selection core of
// cnvGetPaletteEpalN / cnvGetPalette256 in Convert.cpp, with the bin
// counts generalized to double weights; for integral weights the split
// and averaging rules reduce exactly to the original integer ones)

namespace {

struct WBin {
	int r, g, b;
	double w;
};

struct WBox {
	int lo, hi;
	double pop;
	int rmin, rmax, gmin, gmax, bmin, bmax;
};

void wbox_update( WBox* box, const WBin* bins )
{
	box->pop = 0.0;
	box->rmin = box->gmin = box->bmin = 31;
	box->rmax = box->gmax = box->bmax = 0;
	for( int i = box->lo; i < box->hi; ++i ) {
		box->pop += bins[ i ].w;
		if( bins[ i ].r < box->rmin ) box->rmin = bins[ i ].r;
		if( bins[ i ].r > box->rmax ) box->rmax = bins[ i ].r;
		if( bins[ i ].g < box->gmin ) box->gmin = bins[ i ].g;
		if( bins[ i ].g > box->gmax ) box->gmax = bins[ i ].g;
		if( bins[ i ].b < box->bmin ) box->bmin = bins[ i ].b;
		if( bins[ i ].b > box->bmax ) box->bmax = bins[ i ].b;
	}
}

// The original uses pop/2 with integer (floor) division both for the
// split threshold and for the averaging rounding; for integral total
// weights keep exactly that, otherwise use the natural weighted value.
inline double whalf( double pop )
{
	if( pop == floor( pop ) ) return floor( pop * 0.5 );
	return pop * 0.5;
}

} // namespace

int epal_select_weighted( const double* histw, int target, int (*out5)[ 3 ] )
{
	if( histw == nullptr || target <= 0 ) return 0;
	std::vector< WBin > bins;
	for( int i = 0; i < 32768; ++i ) {
		if( histw[ i ] > 0.0 ) {
			WBin b = { ( i >> 10 ) & 31, ( i >> 5 ) & 31, i & 31, histw[ i ] };
			bins.push_back( b );
		}
	}
	const int nbin = (int) bins.size();
	if( nbin == 0 ) return 0;

	if( nbin <= target ) {
		// Few enough colors: take them all, most frequent first (the
		// original sorts by count with cnvColorTblCompare).
		std::stable_sort( bins.begin(), bins.end(),
						  []( const WBin& a, const WBin& b ) { return a.w > b.w; } );
		for( int i = 0; i < nbin; ++i ) {
			out5[ i ][ 0 ] = bins[ i ].r;
			out5[ i ][ 1 ] = bins[ i ].g;
			out5[ i ][ 2 ] = bins[ i ].b;
		}
		return nbin;
	}

	std::vector< WBox > boxes( target );
	boxes[ 0 ].lo = 0;
	boxes[ 0 ].hi = nbin;
	wbox_update( &boxes[ 0 ], bins.data() );
	int nbox = 1;
	while( nbox < target ) {
		// Box to split: largest population with a color spread.
		int tbox = -1;
		double tpop = -1.0;
		for( int i = 0; i < nbox; ++i ) {
			if( boxes[ i ].hi - boxes[ i ].lo < 2 ) continue;
			int range = boxes[ i ].rmax - boxes[ i ].rmin;
			if( boxes[ i ].gmax - boxes[ i ].gmin > range ) range = boxes[ i ].gmax - boxes[ i ].gmin;
			if( boxes[ i ].bmax - boxes[ i ].bmin > range ) range = boxes[ i ].bmax - boxes[ i ].bmin;
			if( range == 0 ) continue;
			if( boxes[ i ].pop > tpop ) {
				tpop = boxes[ i ].pop;
				tbox = i;
			}
		}
		if( tbox < 0 ) break;

		WBox* box = &boxes[ tbox ];
		int channel = 0;
		int range = box->rmax - box->rmin;
		if( box->gmax - box->gmin > range ) { channel = 1; range = box->gmax - box->gmin; }
		if( box->bmax - box->bmin > range ) { channel = 2; range = box->bmax - box->bmin; }
		std::stable_sort( bins.begin() + box->lo, bins.begin() + box->hi,
						  [ channel ]( const WBin& a, const WBin& b ) {
							  int ka = ( channel == 0 ) ? a.r : ( channel == 1 ) ? a.g : a.b;
							  int kb = ( channel == 0 ) ? b.r : ( channel == 1 ) ? b.g : b.b;
							  return ka < kb;
						  } );
		double half = whalf( box->pop );
		double acc = 0.0;
		int split = box->hi - 1;
		for( int i = box->lo; i < box->hi - 1; ++i ) {
			acc += bins[ i ].w;
			if( acc >= half ) {
				split = i + 1;
				break;
			}
		}
		boxes[ nbox ].lo = split;
		boxes[ nbox ].hi = box->hi;
		box->hi = split;
		wbox_update( box, bins.data() );
		wbox_update( &boxes[ nbox ], bins.data() );
		++nbox;
	}

	for( int i = 0; i < nbox; ++i ) {
		double rsum = 0.0, gsum = 0.0, bsum = 0.0;
		for( int j = boxes[ i ].lo; j < boxes[ i ].hi; ++j ) {
			rsum += bins[ j ].r * bins[ j ].w;
			gsum += bins[ j ].g * bins[ j ].w;
			bsum += bins[ j ].b * bins[ j ].w;
		}
		double pop = boxes[ i ].pop;
		double half = whalf( pop );
		int r5 = (int) ( ( rsum + half ) / pop );
		int g5 = (int) ( ( gsum + half ) / pop );
		int b5 = (int) ( ( bsum + half ) / pop );
		if( r5 < 0 ) r5 = 0;
		if( r5 > 31 ) r5 = 31;
		if( g5 < 0 ) g5 = 0;
		if( g5 > 31 ) g5 = 31;
		if( b5 < 0 ) b5 = 0;
		if( b5 > 31 ) b5 = 31;
		out5[ i ][ 0 ] = r5;
		out5[ i ][ 1 ] = g5;
		out5[ i ][ 2 ] = b5;
	}
	return nbox;
}

// ---------------------------------------------------------------------------
// Swap refinement (bin counts generalized to double weights, so the
// HP-bonus histogram can feed it unchanged; with raw counts the weights
// are the counts themselves)

namespace {

struct RBin {
	int r, g, b;
	double n;		// bin weight (raw count, or HP-weighted count)
	double w;		// pow( n, alpha ) * ( 1 + satbeta * saturation )
	double sat;		// (max-min)/max of the bin color, 0 for grays/black
};

inline double dist2( int r1, int g1, int b1, int r2, int g2, int b2 )
{
	int dr = r1 - r2, dg = g1 - g2, db = b1 - b2;
	if( g_redmean ) {
		// The usual redmean approximation, rescaled to 5-bit channels:
		// mean red mapped back to 0..255.
		double rm = ( r1 + r2 ) * ( 255.0 / 62.0 );
		double wr = 2.0 + rm / 256.0;
		double wb = 2.0 + ( 255.0 - rm ) / 256.0;
		return wr * dr * dr + 4.0 * dg * dg + wb * db * db;
	}
	return (double)( dr * dr + dg * dg + db * db );
}

int refine_impl( const double* hist, int (*pal5)[ 3 ], int npal,
				 const int (*fixed5)[ 3 ], int nfixed,
				 const char* tag )
{
	if( !g_enabled || npal <= 0 || g_maxiter <= 0 ) return 0;

	// Collect the non-empty bins.
	int nbin = 0;
	for( int i = 0; i < 32768; ++i ) if( hist[ i ] > 0.0 ) ++nbin;
	if( nbin == 0 ) return 0;
	RBin* bins = (RBin*) malloc( sizeof( RBin ) * nbin );
	double* d1 = (double*) malloc( sizeof( double ) * nbin );
	double* d2 = (double*) malloc( sizeof( double ) * nbin );
	double* dc = (double*) malloc( sizeof( double ) * nbin );
	int* owner = (int*) malloc( sizeof( int ) * nbin );
	double* corr_sse = (double*) malloc( sizeof( double ) * npal );
	double* corr_j = (double*) malloc( sizeof( double ) * npal );
	if( !bins || !d1 || !d2 || !dc || !owner || !corr_sse || !corr_j ) {
		free( bins ); free( d1 ); free( d2 ); free( dc ); free( owner );
		free( corr_sse ); free( corr_j );
		return 0;
	}
	nbin = 0;
	for( int i = 0; i < 32768; ++i ) {
		if( !( hist[ i ] > 0.0 ) ) continue;
		bins[ nbin ].r = ( i >> 10 ) & 31;
		bins[ nbin ].g = ( i >> 5 ) & 31;
		bins[ nbin ].b = i & 31;
		bins[ nbin ].n = hist[ i ];
		int mx = bins[ nbin ].r;
		if( bins[ nbin ].g > mx ) mx = bins[ nbin ].g;
		if( bins[ nbin ].b > mx ) mx = bins[ nbin ].b;
		int mn = bins[ nbin ].r;
		if( bins[ nbin ].g < mn ) mn = bins[ nbin ].g;
		if( bins[ nbin ].b < mn ) mn = bins[ nbin ].b;
		bins[ nbin ].sat = ( mx > 0 ) ? (double) ( mx - mn ) / mx : 0.0;
		bins[ nbin ].w = pow( hist[ i ], g_alpha ) * ( 1.0 + g_satbeta * bins[ nbin ].sat );
		++nbin;
	}

	const int nent = nfixed + npal;	// combined entries: fixed first
	auto entry = [ & ]( int e, int ch ) -> int {
		return ( e < nfixed ) ? fixed5[ e ][ ch ] : pal5[ e - nfixed ][ ch ];
	};

	int swaps = 0;
	double total_sse = 0.0, total_j = 0.0;
	for( int iter = 0; iter < g_maxiter; ++iter ) {
		// Assignment pass: nearest / second-nearest entry per bin, plus
		// the worst-served bin under the area-compressed score.
		total_sse = 0.0;
		total_j = 0.0;
		int worst = -1;
		double worst_score = 0.0;
		for( int i = 0; i < nbin; ++i ) {
			double best = 1e30, second = 1e30;
			int arg = -1;
			for( int e = 0; e < nent; ++e ) {
				double d = dist2( bins[ i ].r, bins[ i ].g, bins[ i ].b,
								  entry( e, 0 ), entry( e, 1 ), entry( e, 2 ) );
				if( d < best ) { second = best; best = d; arg = e; }
				else if( d < second ) { second = d; }
			}
			d1[ i ] = best;
			d2[ i ] = second;
			owner[ i ] = arg;
			total_sse += bins[ i ].n * best;
			total_j += bins[ i ].w * best;
			double score = bins[ i ].w * best;
			if( best > 0.0 && score > worst_score ) {
				worst_score = score;
				worst = i;
			}
		}
		if( iter == 0 && g_debug ) {
			fprintf( stderr, "[epal-refine %s] start: bins=%d entries=%d (fixed %d) alpha=%.2f satbeta=%.2f SSE=%.0f J=%.0f\n",
					 tag, nbin, npal, nfixed, g_alpha, g_satbeta, total_sse, total_j );
		}
		if( worst < 0 ) break;	// every bin is exactly represented

		// Candidate color: count-weighted centroid of the bins within
		// radius 3 (plain 5-bit Euclidean) of the worst bin's color.
		int wr = bins[ worst ].r, wg = bins[ worst ].g, wb = bins[ worst ].b;
		double sn = 0.0, sr = 0.0, sg = 0.0, sb = 0.0;
		for( int i = 0; i < nbin; ++i ) {
			int dr = bins[ i ].r - wr, dg = bins[ i ].g - wg, db = bins[ i ].b - wb;
			if( dr * dr + dg * dg + db * db <= 9 ) {
				sn += bins[ i ].n;
				sr += bins[ i ].r * bins[ i ].n;
				sg += bins[ i ].g * bins[ i ].n;
				sb += bins[ i ].b * bins[ i ].n;
			}
		}
		int cr = (int) ( ( sr + whalf( sn ) ) / sn );
		int cg = (int) ( ( sg + whalf( sn ) ) / sn );
		int cb = (int) ( ( sb + whalf( sn ) ) / sn );
		auto is_entry = [ & ]( int r, int g, int b ) {
			for( int e = 0; e < nent; ++e ) {
				if( entry( e, 0 ) == r && entry( e, 1 ) == g && entry( e, 2 ) == b ) return true;
			}
			return false;
		};
		if( is_entry( cr, cg, cb ) ) { cr = wr; cg = wg; cb = wb; }
		if( is_entry( cr, cg, cb ) ) break;	// worst bin already in palette

		// Exact swap deltas.  For palette entry e, with dc = dist^2 to
		// the candidate: bins not owned by e see min(d1, dc); bins owned
		// by e see min(d2, dc).  Split as (add-candidate delta, same for
		// every e) + (per-entry correction over the bins e owns).
		for( int i = 0; i < nbin; ++i ) {
			dc[ i ] = dist2( bins[ i ].r, bins[ i ].g, bins[ i ].b, cr, cg, cb );
		}
		memset( corr_sse, 0, sizeof( double ) * npal );
		memset( corr_j, 0, sizeof( double ) * npal );
		double add_sse = 0.0, add_j = 0.0;
		for( int i = 0; i < nbin; ++i ) {
			double keep = d1[ i ] < dc[ i ] ? d1[ i ] : dc[ i ];	// min(d1, dc)
			double alt = d2[ i ] < dc[ i ] ? d2[ i ] : dc[ i ];	// min(d2, dc)
			add_sse += bins[ i ].n * ( keep - d1[ i ] );
			add_j += bins[ i ].w * ( keep - d1[ i ] );
			int o = owner[ i ];
			if( o >= nfixed ) {
				corr_sse[ o - nfixed ] += bins[ i ].n * ( alt - keep );
				corr_j[ o - nfixed ] += bins[ i ].w * ( alt - keep );
			}
		}

		// Choose the entry with the best weighted improvement that
		// keeps the true-SSE increase within the guard.
		int best_e = -1;
		double best_dj = 0.0, best_dsse = 0.0;
		double tol_j = 1e-6 * ( total_j > 1.0 ? total_j : 1.0 );
		for( int e = 0; e < npal; ++e ) {
			double dsse = add_sse + corr_sse[ e ];
			double dj = add_j + corr_j[ e ];
			if( dj < -tol_j && dsse <= g_guard * total_sse && ( best_e < 0 || dj < best_dj ) ) {
				best_e = e;
				best_dj = dj;
				best_dsse = dsse;
			}
		}
		if( best_e < 0 ) break;

		if( g_debug ) {
			fprintf( stderr, "[epal-refine %s] swap %d: out (%d,%d,%d) in (%d,%d,%d) dSSE=%+.0f (%+.3f%%) dJ=%+.0f\n",
					 tag, swaps + 1,
					 pal5[ best_e ][ 0 ], pal5[ best_e ][ 1 ], pal5[ best_e ][ 2 ],
					 cr, cg, cb, best_dsse,
					 total_sse > 0 ? 100.0 * best_dsse / total_sse : 0.0, best_dj );
		}
		pal5[ best_e ][ 0 ] = cr;
		pal5[ best_e ][ 1 ] = cg;
		pal5[ best_e ][ 2 ] = cb;
		++swaps;
	}

	if( g_debug ) {
		// Final objectives after the last accepted swap.
		total_sse = 0.0;
		total_j = 0.0;
		for( int i = 0; i < nbin; ++i ) {
			double best = 1e30;
			for( int e = 0; e < nent; ++e ) {
				double d = dist2( bins[ i ].r, bins[ i ].g, bins[ i ].b,
								  entry( e, 0 ), entry( e, 1 ), entry( e, 2 ) );
				if( d < best ) best = d;
			}
			total_sse += bins[ i ].n * best;
			total_j += bins[ i ].w * best;
		}
		fprintf( stderr, "[epal-refine %s] done: swaps=%d SSE=%.0f J=%.0f\n",
				 tag, swaps, total_sse, total_j );
	}

	free( bins ); free( d1 ); free( d2 ); free( dc ); free( owner );
	free( corr_sse ); free( corr_j );
	return swaps;
}

} // namespace

int epal_refine_palette_w( const double* histw, int (*pal5)[ 3 ], int npal,
						   const int (*fixed5)[ 3 ], int nfixed,
						   const char* tag )
{
	epal_parse_env();
	if( histw == nullptr ) return 0;
	return refine_impl( histw, pal5, npal, fixed5, nfixed, tag );
}

int epal_refine_palette( const uint32_t* hist, int (*pal5)[ 3 ], int npal,
						 const int (*fixed5)[ 3 ], int nfixed,
						 const char* tag )
{
	epal_parse_env();
	if( hist == nullptr ) return 0;
	double* histw = (double*) malloc( sizeof( double ) * 32768 );
	if( histw == nullptr ) return 0;
	for( int i = 0; i < 32768; ++i ) histw[ i ] = (double) hist[ i ];
	int swaps = refine_impl( histw, pal5, npal, fixed5, nfixed, tag );
	free( histw );
	return swaps;
}

// ---------------------------------------------------------------------------
// Accent reservation (comparison variant)
//
// Qualification and score are documented in epal_refine.h.  The
// committed set starts as fixed colors + the median-cut palette and
// grows with every reserved accent, so near-duplicates of an accent
// (or of anything already in the palette) cannot qualify.  Displace-
// ment uses the same d1/d2 bookkeeping as the swap refinement: the
// removal cost of an entry is the exact SSE increase over the bins it
// owns when its bins fall back to their second-nearest entry (with
// the accents already in the set).

namespace {

struct ABin {
	int r, g, b;
	uint32_t n;
	double sat;
};

inline double adist2( const ABin& b, int r, int g, int bl )
{
	int dr = b.r - r, dg = b.g - g, db = b.b - bl;
	return (double) ( dr * dr + dg * dg + db * db );
}

} // namespace

int epal_accent_reserve( const uint32_t* hist, int (*pal5)[ 3 ], int* npal_io,
						 const int (*fixed5)[ 3 ], int nfixed,
						 int (*accent5)[ 3 ], int maxaccent,
						 const char* tag )
{
	epal_parse_env();
	int npal = *npal_io;
	if( hist == nullptr || npal <= 1 || maxaccent <= 0 ) return 0;
	if( maxaccent > npal - 1 ) maxaccent = npal - 1;

	// Collect the non-empty bins.
	int nbin = 0;
	for( int i = 0; i < 32768; ++i ) if( hist[ i ] ) ++nbin;
	if( nbin == 0 ) return 0;
	ABin* bins = (ABin*) malloc( sizeof( ABin ) * nbin );
	long long* support = (long long*) malloc( sizeof( long long ) * nbin );
	int (*committed)[ 3 ] = (int (*)[ 3 ]) malloc( sizeof( int[ 3 ] ) * ( nfixed + npal + maxaccent ) );
	if( !bins || !support || !committed ) {
		free( bins ); free( support ); free( committed );
		return 0;
	}
	nbin = 0;
	for( int i = 0; i < 32768; ++i ) {
		if( !hist[ i ] ) continue;
		bins[ nbin ].r = ( i >> 10 ) & 31;
		bins[ nbin ].g = ( i >> 5 ) & 31;
		bins[ nbin ].b = i & 31;
		bins[ nbin ].n = hist[ i ];
		int mx = bins[ nbin ].r;
		if( bins[ nbin ].g > mx ) mx = bins[ nbin ].g;
		if( bins[ nbin ].b > mx ) mx = bins[ nbin ].b;
		int mn = bins[ nbin ].r;
		if( bins[ nbin ].g < mn ) mn = bins[ nbin ].g;
		if( bins[ nbin ].b < mn ) mn = bins[ nbin ].b;
		bins[ nbin ].sat = ( mx > 0 ) ? (double) ( mx - mn ) / mx : 0.0;
		++nbin;
	}

	// Density support: pixels of saturated bins within the radius.
	const double r2max = g_acc_radius * g_acc_radius;
	for( int i = 0; i < nbin; ++i ) {
		long long s = 0;
		for( int j = 0; j < nbin; ++j ) {
			if( bins[ j ].sat < g_acc_satbin ) continue;
			if( adist2( bins[ j ], bins[ i ].r, bins[ i ].g, bins[ i ].b ) <= r2max ) {
				s += bins[ j ].n;
			}
		}
		support[ i ] = s;
	}

	// Committed set: fixed colors first, then the current palette.
	int ncom = 0;
	for( int i = 0; i < nfixed; ++i ) {
		committed[ ncom ][ 0 ] = fixed5[ i ][ 0 ];
		committed[ ncom ][ 1 ] = fixed5[ i ][ 1 ];
		committed[ ncom ][ 2 ] = fixed5[ i ][ 2 ];
		++ncom;
	}
	for( int i = 0; i < npal; ++i ) {
		committed[ ncom ][ 0 ] = pal5[ i ][ 0 ];
		committed[ ncom ][ 1 ] = pal5[ i ][ 1 ];
		committed[ ncom ][ 2 ] = pal5[ i ][ 2 ];
		++ncom;
	}

	const double dmin2 = g_acc_dist * g_acc_dist;
	int nacc = 0;
	for( int k = 0; k < maxaccent; ++k ) {
		int best = -1;
		double best_score = 0.0, best_d2 = 0.0;
		for( int i = 0; i < nbin; ++i ) {
			if( bins[ i ].sat < g_acc_sat ) continue;
			int mx = bins[ i ].r;
			if( bins[ i ].g > mx ) mx = bins[ i ].g;
			if( bins[ i ].b > mx ) mx = bins[ i ].b;
			if( mx < g_acc_vmin ) continue;
			if( support[ i ] < g_acc_minpix ) continue;
			double dm = 1e30;
			for( int e = 0; e < ncom; ++e ) {
				double d = adist2( bins[ i ], committed[ e ][ 0 ], committed[ e ][ 1 ], committed[ e ][ 2 ] );
				if( d < dm ) dm = d;
			}
			if( dm < dmin2 ) continue;
			double score = bins[ i ].sat * dm;	// saturation * dist^2
			if( score > best_score ) {
				best_score = score;
				best_d2 = dm;
				best = i;
			}
		}
		if( best < 0 ) break;
		accent5[ nacc ][ 0 ] = bins[ best ].r;
		accent5[ nacc ][ 1 ] = bins[ best ].g;
		accent5[ nacc ][ 2 ] = bins[ best ].b;
		committed[ ncom ][ 0 ] = bins[ best ].r;
		committed[ ncom ][ 1 ] = bins[ best ].g;
		committed[ ncom ][ 2 ] = bins[ best ].b;
		++ncom;
		if( g_acc_debug || g_debug ) {
			fprintf( stderr, "[epal-accent %s] reserve %d: (%d,%d,%d) sat=%.2f dist=%.1f support=%lld score=%.0f\n",
					 tag, nacc + 1, bins[ best ].r, bins[ best ].g, bins[ best ].b,
					 bins[ best ].sat, sqrt( best_d2 ), support[ best ], best_score );
		}
		++nacc;
	}

	// Displacement: repeatedly drop the auto entry with the smallest
	// exact removal cost (accents participate as committed entries).
	if( nacc > 0 ) {
		// Entry list: [0, nfixed) fixed, then accents, then autos.
		const int noff = nfixed + nacc;
		int* owner = (int*) malloc( sizeof( int ) * nbin );
		double* d1 = (double*) malloc( sizeof( double ) * nbin );
		double* d2 = (double*) malloc( sizeof( double ) * nbin );
		double* cost = (double*) malloc( sizeof( double ) * npal );
		if( !owner || !d1 || !d2 || !cost ) {
			free( owner ); free( d1 ); free( d2 ); free( cost );
			free( bins ); free( support ); free( committed );
			return 0;
		}
		auto acentry = [ & ]( int e, int ch ) -> int {
			if( e < nfixed ) return fixed5[ e ][ ch ];
			if( e < noff ) return accent5[ e - nfixed ][ ch ];
			return pal5[ e - noff ][ ch ];
		};
		int cur = npal;
		for( int drop = 0; drop < nacc; ++drop ) {
			const int nent = noff + cur;
			for( int i = 0; i < nbin; ++i ) {
				double best = 1e30, second = 1e30;
				int arg = -1;
				for( int e = 0; e < nent; ++e ) {
					double d = adist2( bins[ i ], acentry( e, 0 ), acentry( e, 1 ), acentry( e, 2 ) );
					if( d < best ) { second = best; best = d; arg = e; }
					else if( d < second ) { second = d; }
				}
				d1[ i ] = best;
				d2[ i ] = second;
				owner[ i ] = arg;
			}
			for( int e = 0; e < cur; ++e ) cost[ e ] = 0.0;
			for( int i = 0; i < nbin; ++i ) {
				int o = owner[ i ];
				if( o >= noff ) cost[ o - noff ] += (double) bins[ i ].n * ( d2[ i ] - d1[ i ] );
			}
			int be = 0;
			for( int e = 1; e < cur; ++e ) if( cost[ e ] < cost[ be ] ) be = e;
			if( g_acc_debug || g_debug ) {
				fprintf( stderr, "[epal-accent %s] displace %d: out (%d,%d,%d) removal cost=%.0f\n",
						 tag, drop + 1, pal5[ be ][ 0 ], pal5[ be ][ 1 ], pal5[ be ][ 2 ], cost[ be ] );
			}
			for( int e = be; e + 1 < cur; ++e ) {
				pal5[ e ][ 0 ] = pal5[ e + 1 ][ 0 ];
				pal5[ e ][ 1 ] = pal5[ e + 1 ][ 1 ];
				pal5[ e ][ 2 ] = pal5[ e + 1 ][ 2 ];
			}
			--cur;
		}
		*npal_io = cur;
		free( owner ); free( d1 ); free( d2 ); free( cost );
	}

	free( bins ); free( support ); free( committed );
	return nacc;
}
