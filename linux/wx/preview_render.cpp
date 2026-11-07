// See preview_render.h.  The per-mode decode loops below follow
// winConvert.cpp's _draw_screen*() functions statement by statement;
// SetPixel( hDC, X, Y, GET_RGB( r, g, b ) ) becomes plot( X, Y, r, g, b ).
#include "preview_render.h"

#include <cstring>

namespace {

struct Canvas {
	unsigned char* rgb;
	void plot( int x, int y, int r, int g, int b ) const {
		if( x < 0 || x >= PREVIEW_W || y < 0 || y >= PREVIEW_H ) return;
		unsigned char* p = rgb + ( y * PREVIEW_W + x ) * 3;
		p[ 0 ] = (unsigned char) r;
		p[ 1 ] = (unsigned char) g;
		p[ 2 ] = (unsigned char) b;
	}
};

// winConvert.cpp: _get_height
int get_height( const SETTING* Mode ) {
	return Mode->b192 ? 192 : 212;
}

void draw_screen2( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r[ 2 ] = { 0 }, g[ 2 ] = { 0 }, b[ 2 ] = { 0 };
	int x, y, adr, i, c, bit;

	for( y = 0; y < 384; y += 2 ) {
		for( x = 0; x < 512; x += 16 ) {
			adr = y / 2;
			adr = ( adr >> 3 ) * 256 + ( x / 16 ) * 8 + ( adr & 7 );
			r[ 0 ] = convert7to255_r[ Mode->color_palette[ bmp[ adr + SC2COLOR ] & 0x0F ].red ];
			g[ 0 ] = convert7to255_g[ Mode->color_palette[ bmp[ adr + SC2COLOR ] & 0x0F ].green ];
			b[ 0 ] = convert7to255_b[ Mode->color_palette[ bmp[ adr + SC2COLOR ] & 0x0F ].blue ];
			r[ 1 ] = convert7to255_r[ Mode->color_palette[ bmp[ adr + SC2COLOR ] >> 4 ].red ];
			g[ 1 ] = convert7to255_g[ Mode->color_palette[ bmp[ adr + SC2COLOR ] >> 4 ].green ];
			b[ 1 ] = convert7to255_b[ Mode->color_palette[ bmp[ adr + SC2COLOR ] >> 4 ].blue ];

			c = bmp[ adr ];
			for( i = 0; i < 8; ++i ) {
				bit = ( c & 0x80 ) >> 7;
				cv.plot( x + i * 2 + 0, y + 20, r[ bit ], g[ bit ], b[ bit ] );
				cv.plot( x + i * 2 + 1, y + 20, r[ bit ], g[ bit ], b[ bit ] );
				cv.plot( x + i * 2 + 0, y + 21, r[ bit ], g[ bit ], b[ bit ] );
				cv.plot( x + i * 2 + 1, y + 21, r[ bit ], g[ bit ], b[ bit ] );
				c = c << 1;
			}
		}
	}
}

void draw_screen3( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, i;

	for( y = 0; y < 384; ++y ) {
		for( x = 0; x < 512; x += 16 ) {
			adr = y / 8;
			adr = ( adr >> 3 ) * 256 + ( x / 16 ) * 8 + ( adr & 7 );
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] >> 4 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] >> 4 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] >> 4 ].blue ];
			for( i = 0; i < 8; ++i ) {
				cv.plot( x + i, y + 20, r, g, b );
			}
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x0F ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x0F ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x0F ].blue ];
			for( i = 0; i < 8; ++i ) {
				cv.plot( x + i + 8, y + 20, r, g, b );
			}
		}
	}
}

void draw_screen5( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 4 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 128 + x / 4;
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] >> 4 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] >> 4 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] >> 4 ].blue ];
			cv.plot( x + 0, y + t, r, g, b );
			cv.plot( x + 1, y + t, r, g, b );
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x0F ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x0F ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x0F ].blue ];
			cv.plot( x + 2, y + t, r, g, b );
			cv.plot( x + 3, y + t, r, g, b );
		}
	}
}

void draw_screen6( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 4 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 128 + x / 4;
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] >> 6 ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] >> 6 ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] >> 6 ) & 0x03 ].blue ];
			cv.plot( x + 0, y + t, r, g, b );
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] >> 4 ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] >> 4 ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] >> 4 ) & 0x03 ].blue ];
			cv.plot( x + 1, y + t, r, g, b );
			r = convert7to255_r[ Mode->color_palette[ ( bmp[ adr ] >> 2 ) & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ ( bmp[ adr ] >> 2 ) & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ ( bmp[ adr ] >> 2 ) & 0x03 ].blue ];
			cv.plot( x + 2, y + t, r, g, b );
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x03 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x03 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x03 ].blue ];
			cv.plot( x + 3, y + t, r, g, b );
		}
	}
}

void draw_screen7( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 2 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] >> 4 ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] >> 4 ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] >> 4 ].blue ];
			cv.plot( x + 0, y + t, r, g, b );
			r = convert7to255_r[ Mode->color_palette[ bmp[ adr ] & 0x0F ].red ];
			g = convert7to255_g[ Mode->color_palette[ bmp[ adr ] & 0x0F ].green ];
			b = convert7to255_b[ Mode->color_palette[ bmp[ adr ] & 0x0F ].blue ];
			cv.plot( x + 1, y + t, r, g, b );
		}
	}
}

void draw_screen8( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 2 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r = convert7to255_s8r[ ( bmp[ adr ] & 0x1C ) >> 2 ];
			g = convert7to255_s8g[ ( bmp[ adr ] & 0xE0 ) >> 5 ];
			b = convert3to255_s8b[ bmp[ adr ] & 0x03 ];
			cv.plot( x + 0, y + t, r, g, b );
			cv.plot( x + 1, y + t, r, g, b );
		}
	}
}

void draw_screen8p( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 2 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r = Mode->color_palette_epal[ bmp[ adr ] ].red   * 255 / 31;
			g = Mode->color_palette_epal[ bmp[ adr ] ].green * 255 / 31;
			b = Mode->color_palette_epal[ bmp[ adr ] ].blue  * 255 / 31;
			cv.plot( x + 0, y + t, r, g, b );
			cv.plot( x + 1, y + t, r, g, b );
		}
	}
}

// draw_screen2 with the EPAL (RGB555) palette.
void draw_screen2p( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r[ 2 ] = { 0 }, g[ 2 ] = { 0 }, b[ 2 ] = { 0 };
	int x, y, adr, i, c, bit;

	for( y = 0; y < 384; y += 2 ) {
		for( x = 0; x < 512; x += 16 ) {
			adr = y / 2;
			adr = ( adr >> 3 ) * 256 + ( x / 16 ) * 8 + ( adr & 7 );
			r[ 0 ] = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr + SC2COLOR ] & 0x0F ].red );
			g[ 0 ] = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr + SC2COLOR ] & 0x0F ].green );
			b[ 0 ] = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr + SC2COLOR ] & 0x0F ].blue );
			r[ 1 ] = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr + SC2COLOR ] >> 4 ].red );
			g[ 1 ] = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr + SC2COLOR ] >> 4 ].green );
			b[ 1 ] = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr + SC2COLOR ] >> 4 ].blue );

			c = bmp[ adr ];
			for( i = 0; i < 8; ++i ) {
				bit = ( c & 0x80 ) >> 7;
				cv.plot( x + i * 2 + 0, y + 20, r[ bit ], g[ bit ], b[ bit ] );
				cv.plot( x + i * 2 + 1, y + 20, r[ bit ], g[ bit ], b[ bit ] );
				cv.plot( x + i * 2 + 0, y + 21, r[ bit ], g[ bit ], b[ bit ] );
				cv.plot( x + i * 2 + 1, y + 21, r[ bit ], g[ bit ], b[ bit ] );
				c = c << 1;
			}
		}
	}
}

// draw_screen3 with the EPAL (RGB555) palette.
void draw_screen3p( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, i;

	for( y = 0; y < 384; ++y ) {
		for( x = 0; x < 512; x += 16 ) {
			adr = y / 8;
			adr = ( adr >> 3 ) * 256 + ( x / 16 ) * 8 + ( adr & 7 );
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].blue );
			for( i = 0; i < 8; ++i ) {
				cv.plot( x + i, y + 20, r, g, b );
			}
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].blue );
			for( i = 0; i < 8; ++i ) {
				cv.plot( x + i + 8, y + 20, r, g, b );
			}
		}
	}
}

// draw_screen5 with the EPAL (RGB555) palette.
void draw_screen5p( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 4 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 128 + x / 4;
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].blue );
			cv.plot( x + 0, y + t, r, g, b );
			cv.plot( x + 1, y + t, r, g, b );
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].blue );
			cv.plot( x + 2, y + t, r, g, b );
			cv.plot( x + 3, y + t, r, g, b );
		}
	}
}

// draw_screen6 with the EPAL (RGB555) palette.
void draw_screen6p( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 4 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 128 + x / 4;
			r = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 6 ) & 0x03 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 6 ) & 0x03 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 6 ) & 0x03 ].blue );
			cv.plot( x + 0, y + t, r, g, b );
			r = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 4 ) & 0x03 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 4 ) & 0x03 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 4 ) & 0x03 ].blue );
			cv.plot( x + 1, y + t, r, g, b );
			r = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 2 ) & 0x03 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 2 ) & 0x03 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ ( bmp[ adr ] >> 2 ) & 0x03 ].blue );
			cv.plot( x + 2, y + t, r, g, b );
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x03 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x03 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x03 ].blue );
			cv.plot( x + 3, y + t, r, g, b );
		}
	}
}

// draw_screen7 with the EPAL (RGB555) palette.
void draw_screen7p( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b;
	int x, y, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 2 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] >> 4 ].blue );
			cv.plot( x + 0, y + t, r, g, b );
			r = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].red );
			g = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].green );
			b = cnvEpalTo255( Mode->color_palette_epal[ bmp[ adr ] & 0x0F ].blue );
			cv.plot( x + 1, y + t, r, g, b );
		}
	}
}

void draw_screen12( Canvas& cv, const unsigned char* bmp, const SETTING* Mode ) {
	int r, g, b, j, k, yy[ 4 ] = { 0 };
	int x, y, z, adr, t, h;

	h = get_height( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ) {
		for( x = 0; x < 512; x += 8 ) {
			if( Mode->interlace ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			for( z = 0; z < 4; ++z ) {
				yy[ z ] = ( bmp[ adr + z ] & 0xF8 ) >> 3;
			}
			k = ( bmp[ adr + 0 ] & 0x07 ) | ( ( bmp[ adr + 1 ] & 0x07 ) << 3 );
			if( k > 31 ) k = k - 64;
			j = ( bmp[ adr + 2 ] & 0x07 ) | ( ( bmp[ adr + 3 ] & 0x07 ) << 3 );
			if( j > 31 ) j = j - 64;
			for( z = 0; z < 4; ++z ) {
				r = range_limiter( yy[ z ] + j, 0, 31 ) * 255 / 31;
				g = range_limiter( yy[ z ] + k, 0, 31 ) * 255 / 31;
				b = range_limiter( 5 * yy[ z ] / 4 - j / 2 - k / 4, 0, 31 ) * 255 / 31;
				cv.plot( x + z * 2 + 0, y + t, r, g, b );
				cv.plot( x + z * 2 + 1, y + t, r, g, b );
			}
		}
	}
}

} // namespace

void render_converted_preview( const unsigned char* bmp, const SETTING* mode,
							   unsigned char* rgb_out ) {
	// The Windows original fills the canvas with the black brush first.
	memset( rgb_out, 0, PREVIEW_W * PREVIEW_H * 3 );
	Canvas cv{ rgb_out };

	switch( mode->screen_mode ) {
	case MD_SC2:	draw_screen2( cv, bmp, mode );	break;
	case MD_SC3:	draw_screen3( cv, bmp, mode );	break;
	case MD_SC5:
	case MD_SC5_256L:	draw_screen5( cv, bmp, mode );	break;
	case MD_SC6:
	case MD_SC6_256L:	draw_screen6( cv, bmp, mode );	break;
	case MD_SC7:
	case MD_SC7_256L:	draw_screen7( cv, bmp, mode );	break;
	case MD_SC8:
	case MD_SC8_256L:	draw_screen8( cv, bmp, mode );	break;
	case MD_SC8P:
	case MD_SC8P_256L:	draw_screen8p( cv, bmp, mode );	break;
	case MD_SC2P:	draw_screen2p( cv, bmp, mode );	break;
	case MD_SC3P:	draw_screen3p( cv, bmp, mode );	break;
	case MD_SC5P:
	case MD_SC5P_256L:	draw_screen5p( cv, bmp, mode );	break;
	case MD_SC6P:
	case MD_SC6P_256L:	draw_screen6p( cv, bmp, mode );	break;
	case MD_SC7P:
	case MD_SC7P_256L:	draw_screen7p( cv, bmp, mode );	break;
	case MD_SC10:
	case MD_SC12:
	case MD_SC10_256L:
	case MD_SC12_256L:	draw_screen12( cv, bmp, mode );	break;
	}
}

void render_color_buffer( const C_COLOR* px, int w, int h, unsigned char* rgb_out ) {
	for( int i = 0; i < w * h; ++i ) {
		rgb_out[ i * 3 + 0 ] = (unsigned char) GET_RED( px[ i ] );
		rgb_out[ i * 3 + 1 ] = (unsigned char) GET_GREEN( px[ i ] );
		rgb_out[ i * 3 + 2 ] = (unsigned char) GET_BLUE( px[ i ] );
	}
}
