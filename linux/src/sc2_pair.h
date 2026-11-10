// ---------------------------------------------------------------------------
// sc2_pair.h — SCREEN2 line-pair joint quantization prototype (env-gated).
//
// SCREEN2 allows only 2 colors per 8x1 cell on each line, but every line
// is independent, so a flat area whose ideal color lies between two
// palette colors can be rendered as horizontal stripes (color A on even
// lines, color B on odd lines) whose vertical average is the wanted
// intermediate color.  The product quantizer (Convert.cpp cnvRecolor5)
// works per pixel / per line and the cell reduction (cnvSC5toSC2) then
// drops colors; this prototype instead quantizes vertical pixel PAIRS
// jointly.  For t1 = (x,2y), t2 = (x,2y+1) it picks palette colors
// (c1,c2) minimizing
//
//   E = d2(t1,c1) + d2(t2,c2) + w * d2(t1+t2, c1+c2)
//
// where d2 is the squared ICtCp distance (the space of the product's
// _distance_itp).  The w term asks the pair's average to match, which
// makes flat areas pick line-alternating bracketing colors; an exact
// palette color still wins with c1 == c2, and the first two terms keep
// vertical detail honest.  w comes from the environment:
//
//   BMP2MSX_SC2_PAIR=<w>     enable the prototype (w >= 0; 0 = plain
//                            per-pixel quantization through this code,
//                            which must reproduce the product's
//                            no-diffusion result byte for byte)
//   BMP2MSX_SC2_FIT=line     (default) feed the pair-quantized index
//                            image through a faithful transcription of
//                            the product's per-line cell enforcement
//   BMP2MSX_SC2_FIT=pair     pair-aware fitting: per 8x2 block, choose
//                            the two lines' color pairs jointly under
//                            the same objective
//   BMP2MSX_SC2_DUMP=<dir>   observation dumps (target / palette /
//                            rendered / index / stats).  Works with or
//                            without BMP2MSX_SC2_PAIR and never changes
//                            any output byte.
//   BMP2MSX_SC2_LABEL=<name> label recorded in stats.txt
//   BMP2MSX_SC2_EDGE=1       v2: edge-adaptive blending.  An edge map
//                            is computed from the target (per-channel
//                            Sobel magnitude, max over channels,
//                            percentile-normalized, 3x3-max dilated);
//                            each pixel pair's weight becomes
//                            w * (1 - e_pair), so edges quantize
//                            per-dot and flat areas keep v1 stripes.
//   BMP2MSX_SC2_EDGE_PCT=<p> percentile for the edge scale (def. 98)
//   BMP2MSX_SC2_EDGE_DILATE=<n> 3x3 max-dilation passes (def. 1)
//   BMP2MSX_SC2_EDGE_LO=<f>  dead zone: e below f counts as flat,
//                            rescaled above it (def. 0)
//   BMP2MSX_SC2_FLAT=1       v3: flat-region selective mode (the
//                            author's inversion of v2).  A per-dot
//                            flat / gentle-gradient mask is detected
//                            from the target (robust P90-P10 channel
//                            range over a 5x5 window + majority
//                            cleanup); the pair machinery runs at full
//                            w ONLY inside the mask, everything else
//                            uses the conventional method.  Cell
//                            fitting is selective too: inside blocks
//                            take the pair-aware fit, outside blocks
//                            the conventional fit, and boundary blocks
//                            (mixed 8x2) evaluate both candidates and
//                            keep the one with smaller true error.
//                            Replaces the FIT selection; EDGE is
//                            ignored in this mode.
//   BMP2MSX_SC2_FLAT_RANGE=<n> mask threshold in 8-bit channel units
//                            (def. 32; negative = empty mask,
//                            >1000 = full mask, for verification)
//   BMP2MSX_SC2_FLAT_OUT=shadow|nearest
//                            outside-mask quantization: the product's
//                            own quantizer incl. error diffusion via
//                            the validated shadow transcription
//                            (def. shadow), or per-dot nearest
//
// With neither variable set this module is completely inert.  The pair
// path does not cover Tile mode or interlace; the hook in
// convert_run.cpp falls back to the product cnvRecolor() there.
// ---------------------------------------------------------------------------
#pragma once

#include <string>

#include "Convert.h"

bool sc2_pair_env_set();
bool sc2_dump_env_set();

// Replacement for cnvRecolor() for MD_SC2 / MD_SC2P (256x192, no Tile,
// no interlace).  `in` is the resized target image, `out` the 128KB
// VRAM work buffer cnvRecolor() would have written.
bool sc2_pair_convert( C_COLOR* in, int width, int height,
					   unsigned char* out, SETTING* Mode, C_COLOR* pal,
					   std::string* err );

// Observation hooks (see BMP2MSX_SC2_DUMP above).
void sc2_dump_before( const SETTING& Mode, C_COLOR* target, int width, int height, C_COLOR* pal );
void sc2_dump_after( const SETTING& Mode, const unsigned char* bmp, C_COLOR* pal );
