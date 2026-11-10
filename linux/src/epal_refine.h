// ---------------------------------------------------------------------------
// epal_refine: EXPERIMENTAL palette-selection variants for the EPAL
// auto-selection (SCREEN2-7 EPAL via cnvGetPaletteEpalN and SCREEN8
// EPAL via cnvGetPalette256), Linux CLI only.
//
// Everything here is opt-in through environment variables and completely
// inert when they are not set (the default conversion output is then
// byte-identical to the unmodified program).  Enabling any of the three
// experiments also keeps the auto-selected palette at full RGB555
// precision through CreatePal, instead of reflecting it through the
// 3-bit palette as the stock code does.
//
// 1) Swap refinement (post-selection patch):
//   BMP2MSX_EPAL_REFINE           alpha for the area-compression weight
//                                 (a number, e.g. "0.5").  Setting this
//                                 variable enables the experiment: the
//                                 swap refinement below runs on the
//                                 median-cut result.
//   BMP2MSX_EPAL_REFINE_MAXITER   swap iteration cap (default 32;
//                                 0 = precision passthrough only)
//   BMP2MSX_EPAL_REFINE_GUARD     max true-SSE increase allowed per swap,
//                                 as a fraction of the current total SSE
//                                 (default 0.01)
//   BMP2MSX_EPAL_REFINE_REDMAN    "1" = use a redmean-style weighted
//                                 distance inside the refinement instead
//                                 of plain Euclidean RGB555 distance
//   BMP2MSX_EPAL_REFINE_SATBETA   beta >= 0 (default 0): the per-bin
//                                 weight becomes count^alpha *
//                                 (1 + beta * saturation), a continuous
//                                 saturation bonus inside the swap
//                                 scorer / acceptance objective.
//   BMP2MSX_EPAL_REFINE_DEBUG     "1" = log every accepted swap and the
//                                 before/after objective values to stderr
//   BMP2MSX_EPAL_HISTDUMP         path of a file the RGB555 histogram of
//                                 the (resized) selection image is written
//                                 to: uint32 width, uint32 height, then
//                                 32768 uint32 bin counts, index =
//                                 (r5<<10)|(g5<<5)|b5.  Output-neutral:
//                                 conversion results do not change.
//
// The refinement itself: over the same 32768-bin RGB555 histogram the
// median cut worked on, repeatedly pick the worst-served bin cluster
// (score = count^alpha * dist^2, so alpha < 1 compresses the advantage
// of large areas) and swap its centroid color in for the palette entry
// whose removal costs the least, whenever the weighted objective
// improves and the true SSE does not grow beyond the guard.
//
// 2) High-pass bonus weighted histogram (the author's scheme; the
//    saliency enters at measurement time, before any merging):
//   BMP2MSX_EPAL_HPBONUS          beta >= 0.  Per-pixel weight
//                                 w = 1 + beta * hpn, where hpn is the
//                                 normalized high-pass response of the
//                                 (resized, pre-quantization) image:
//                                 per channel, response = |channel -
//                                 box-blurred channel| (blur radius
//                                 BMP2MSX_EPAL_HP_RADIUS, default 2),
//                                 pixel response = max over channels;
//                                 a dead zone BMP2MSX_EPAL_HP_DZ
//                                 (default 6/255) is subtracted (flat-
//                                 area JPEG noise also responds), and
//                                 the remainder is normalized by its
//                                 99th percentile and clamped to 1.
//                                 The weights are accumulated into the
//                                 RGB555 histogram (double counts) and
//                                 the median cut (and, if enabled, the
//                                 swap refinement's count terms) work
//                                 on the weighted histogram.  Base
//                                 weight is 1, so flat areas keep
//                                 their representation; singular points
//                                 (details / edges / highlights) count
//                                 up to 1 + beta times.
//   BMP2MSX_EPAL_HP_DEBUG         "1" = log HP statistics to stderr.
//
// 3) Accent reservation (comparison variant; non-error criterion):
//   BMP2MSX_EPAL_ACCENT           K > 0: reserve up to K palette slots
//                                 for small-area distinctive ("accent")
//                                 colors.  A histogram bin (the seed)
//                                 qualifies when its saturation
//                                 (max-min)/max >= BMP2MSX_EPAL_ACCENT_SAT
//                                 (0.35), its brightest channel >=
//                                 ..._VMIN (10), at least ..._MINPIX (8)
//                                 pixels of saturated bins (saturation
//                                 >= ..._SATBIN, 0.30) lie within
//                                 ..._RADIUS (5.0) of it (density
//                                 support), and its distance to every
//                                 committed color (fixed colors, the
//                                 median-cut palette, accents already
//                                 reserved) is >= ..._DIST (8.0).
//                                 Greedy score = saturation * dist^2
//                                 (area enters only through the support
//                                 floor: with a count^0.5 factor the
//                                 large clusters win again and the
//                                 accents this exists for lose).  The
//                                 reserved color is the seed bin itself
//                                 (micro-accents are diffuse in color
//                                 space; their neighborhood centroid
//                                 drifts toward unrelated colors).
//                                 Each reserved accent displaces the
//                                 median-cut entry whose exact removal
//                                 cost (with the accents present) is
//                                 smallest; reserved accents are then
//                                 treated as fixed colors downstream.
//   BMP2MSX_EPAL_ACCENT_DEBUG     "1" = log reservations / displaced
//                                 entries to stderr.
//
// 4) Chroma-change saliency bonus (third scheme; saliency = local
//    CHANGE of chroma, not its absolute level):
//   BMP2MSX_EPAL_UVBONUS          beta >= 0.  The resized image is
//                                 converted to YUV (BT.601).  Two
//                                 response variants, selected with
//                                 BMP2MSX_EPAL_UV_MODE:
//                                 "mag" (UV-MAG, the author's scheme):
//                                 build the chroma-magnitude map
//                                 M = |(U,V)| and high-pass it,
//                                 response = |M - boxblur(M)|;
//                                 "vec" (UV-VEC): high-pass U and V
//                                 separately, response =
//                                 sqrt(HP_U^2 + HP_V^2), which also
//                                 catches hue flips at equal |(U,V)|;
//                                 "y" (Y): response = |Y - boxblur(Y)|
//                                 (pure luminance change; dead zone
//                                 BMP2MSX_EPAL_UV_YDZ, default 6.0);
//                                 "uvy" (UVY): the UV-MAG and Y
//                                 responses, each dead-zoned and
//                                 p99-normalized on its own, combined
//                                 as sqrt(nM^2 + nY^2) clamped to 1.
//                                 A dead zone BMP2MSX_EPAL_UV_DZ
//                                 (default 2.0, in chroma units) is
//                                 subtracted, the remainder normalized
//                                 by its 99th percentile and clamped,
//                                 and the per-pixel weight is
//                                 w = 1 + beta * normalized response.
//                                 From there the path is identical to
//                                 the HP bonus: weighted histogram,
//                                 weighted median cut, optional swap
//                                 refinement.  UV takes precedence
//                                 over HP when both are set.
//   BMP2MSX_EPAL_UV_RADIUS        box blur radius (default 2).
//   BMP2MSX_EPAL_UV_DEBUG         "1" = log response statistics
//                                 (responding fraction etc.) to stderr.
//   BMP2MSX_EPAL_UV_DUMP          path of a binary dump of the
//                                 normalized response map (uint32 w,
//                                 uint32 h, then w*h uint8 = n * 255),
//                                 for offline visualization.  Only
//                                 written when the UV bonus is set.
//
// 5) Compressed-domain histogram + saliency bonus (fourth scheme;
//    selection never sees raw counts):
//   BMP2MSX_EPAL_SQRTHIST         k >= 0.  The effective histogram the
//                                 weighted median cut (and, when the
//                                 swap refinement is also enabled, its
//                                 count terms) consumes is built in a
//                                 compressed domain:
//                                   h(c) = sqrt( n(c) ) + k * b(c)
//                                 where n(c) is the raw RGB555 bin
//                                 count and b(c) is the bin's saliency
//                                 bonus mass: the sum, over the bin's
//                                 pixels, of the normalized chroma-
//                                 saliency response n_resp in [0,1]
//                                 (scheme 4's response map: dead-zoned,
//                                 p99-normalized; the carrier is the
//                                 BMP2MSX_EPAL_UV_MODE selection, with
//                                 UVY as this scheme's default when the
//                                 mode is not set explicitly).
//                                 b is in pixel-equivalent units: a
//                                 bin whose pixels all respond at
//                                 n_resp = 1 has b(c) = n(c), so at
//                                 k = 1 a fully salient bin recovers
//                                 its full linear mass while a non-
//                                 responding bin keeps only sqrt(n).
//                                 k = 0 isolates the pure-compression
//                                 effect.  Fixed-color bins and
//                                 FourceZero pixels are excluded from
//                                 both terms exactly as in the stock
//                                 selection.  Takes precedence over
//                                 the HP / UV pixel-weight paths when
//                                 several are set.
//   BMP2MSX_EPAL_SQRTHIST_DEBUG   "1" = log the h-mass statistics.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_CLI_EPAL_REFINE_H
#define BMP2MSX_CLI_EPAL_REFINE_H

#include "Convert.h"

// Experiment switches (parsed once from the environment).
bool epal_refine_enabled();
bool epal_accent_enabled();
bool epal_hp_enabled();
bool epal_uv_enabled();
bool epal_sqrthist_enabled();
bool epal_experiment_enabled();	// any of the five
int  epal_accent_k();			// BMP2MSX_EPAL_ACCENT value (>= 0)

// Quantize an 8-bit-per-channel C_COLOR to its RGB555 triple, using the
// same nearest-level table (cnvEpalTo255) as Convert.cpp's
// epal_quantize31.  Exact inverse for colors built from that table.
void epal_color_to_5bit( C_COLOR c, int* r5, int* g5, int* b5 );

// Build the RGB555 histogram of an image buffer (same quantization and
// same optional FourceZero exclusion as cnvGetPalette256 /
// cnvGetPaletteEpalN).  Returns a malloc'ed uint32_t[32768] or NULL.
uint32_t* epal_build_hist555( const C_COLOR* in, int w, int h,
							  bool skip_fz, C_COLOR fz_color );

// Write the full (no exclusions) RGB555 histogram of the image to the
// BMP2MSX_EPAL_HISTDUMP path, if that variable is set.  No-op otherwise.
void epal_maybe_histdump( const C_COLOR* in, int w, int h );

// Per-pixel high-pass bonus weights for the image (malloc'ed
// double[w*h], w = 1 + beta * normalized response), or NULL when the
// HP experiment is disabled.  See the header comment for the exact
// definition.
double* epal_hp_pixel_weights( const C_COLOR* in, int w, int h );

// Per-pixel chroma-change saliency weights (malloc'ed double[w*h],
// w = 1 + beta * normalized chroma-change response), or NULL when the
// UV experiment is disabled.  See the header comment (scheme 4).
double* epal_uv_pixel_weights( const C_COLOR* in, int w, int h );

// Normalized chroma-saliency response map (malloc'ed double[w*h],
// values in [0,1]: dead-zoned and p99-normalized, scheme 4's response
// before the 1 + beta scaling), or NULL when neither the UV bonus nor
// the sqrt-histogram scheme is enabled.  Honors BMP2MSX_EPAL_UV_MODE;
// with the sqrt scheme and no explicit mode, the carrier is UVY.
double* epal_uv_response_map( const C_COLOR* in, int w, int h );

// Compressed-domain effective histogram for scheme 5 (malloc'ed
// double[32768]): h(c) = sqrt( n(c) ) + k * b(c), with n the raw bin
// count and b the bin's saliency bonus mass (sum of the normalized
// UV response over the bin's pixels).  Same fixed-color / FourceZero
// exclusions as the stock selection.  NULL when the scheme is
// disabled or on allocation failure.
double* epal_build_hist_sqrt555( const C_COLOR* in, int w, int h,
								 bool skip_fz, C_COLOR fz_color );

// Weighted RGB555 histogram (malloc'ed double[32768]): like
// epal_build_hist555 but each pixel contributes pixw[i] (1.0 when
// pixw is NULL) instead of 1.
double* epal_build_histw555( const C_COLOR* in, int w, int h,
							 bool skip_fz, C_COLOR fz_color,
							 const double* pixw );

// Weighted median cut: a faithful mirror of the selection core of
// cnvGetPaletteEpalN / cnvGetPalette256 (same box choice, split and
// rounding rules) over a weighted histogram.  Bins belonging to fixed
// colors must already be zeroed in histw by the caller (the original
// zeroes them internally).  Fills out5 with up to `target` RGB555
// colors and returns their number.
int epal_select_weighted( const double* histw, int target, int (*out5)[ 3 ] );

// Swap-refine pal5[0..npal) (RGB555 triples, modified in place) against
// the histogram.  fixed5[0..nfixed) entries take part in the nearest-
// entry assignment but are never removed.  Returns the number of
// accepted swaps.  Iteration cap / guard / distance flavor come from
// the environment (see above).  The _w variant takes bin weights
// (double counts, e.g. an HP-bonus histogram) instead of raw counts.
int epal_refine_palette( const uint32_t* hist, int (*pal5)[ 3 ], int npal,
						 const int (*fixed5)[ 3 ], int nfixed,
						 const char* tag );
int epal_refine_palette_w( const double* histw, int (*pal5)[ 3 ], int npal,
						   const int (*fixed5)[ 3 ], int nfixed,
						   const char* tag );

// Accent reservation: greedily reserve up to maxaccent accent colors
// from the histogram (see the header comment for qualification and
// score), write them to accent5, and displace the same number of
// entries from pal5 (cheapest exact removal cost first), compacting
// pal5 and updating *npal_io.  Returns the number reserved.
int epal_accent_reserve( const uint32_t* hist, int (*pal5)[ 3 ], int* npal_io,
						 const int (*fixed5)[ 3 ], int nfixed,
						 int (*accent5)[ 3 ], int maxaccent,
						 const char* tag );

#endif
