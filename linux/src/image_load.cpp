// See image_load.h for the contract.  The BMP normalization below mirrors
// what WIC (GUID_WICPixelFormat24bppBGR) produces for the same file:
// palette indices resolved through the file's palette, 16bpp BI_RGB read
// as 5-5-5 (bitfield masks honored when present), channels scaled to
// 8 bits, rows stored bottom-up in BGR order with 4-byte stride padding.
#include "image_load.h"

#include <windows.h>	// shim: BITMAPFILEHEADER / BITMAPINFOHEADER / RGBQUAD

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "stb_image.h"

static uint16_t rd16( const unsigned char* p ) { return (uint16_t)( p[ 0 ] | ( p[ 1 ] << 8 ) ); }
static uint32_t rd32( const unsigned char* p ) { return (uint32_t) p[ 0 ] | ( (uint32_t) p[ 1 ] << 8 ) | ( (uint32_t) p[ 2 ] << 16 ) | ( (uint32_t) p[ 3 ] << 24 ); }

// Build a 24bpp DIB (header + bottom-up BGR rows) from top-down RGB pixels.
static char* make_dib24( const unsigned char* rgb_topdown, int w, int h ) {
	const uint32_t stride = ( (uint32_t) w * 3 + 3 ) & ~3u;
	const size_t total = sizeof( BITMAPINFOHEADER ) + (size_t) stride * h;
	char* dib = (char*) calloc( 1, total );
	if( !dib ) return nullptr;
	BITMAPINFOHEADER* hdr = (BITMAPINFOHEADER*) dib;
	hdr->biSize = sizeof( BITMAPINFOHEADER );
	hdr->biWidth = w;
	hdr->biHeight = h;
	hdr->biPlanes = 1;
	hdr->biBitCount = 24;
	hdr->biCompression = BI_RGB;
	hdr->biSizeImage = stride * (uint32_t) h;
	unsigned char* rows = (unsigned char*) dib + sizeof( BITMAPINFOHEADER );
	for( int y = 0; y < h; ++y ) {
		const unsigned char* src = rgb_topdown + (size_t) ( h - 1 - y ) * w * 3;
		unsigned char* dst = rows + (size_t) y * stride;
		for( int x = 0; x < w; ++x ) {
			dst[ x * 3 + 0 ] = src[ x * 3 + 2 ];	// B
			dst[ x * 3 + 1 ] = src[ x * 3 + 1 ];	// G
			dst[ x * 3 + 2 ] = src[ x * 3 + 0 ];	// R
		}
	}
	return dib;
}

// Decode a BMP file image (whole file in memory) to top-down RGB.
// Returns false if the file is not a BMP this decoder handles.
static bool bmp_to_rgb( const unsigned char* file, size_t size, std::vector<unsigned char>& rgb, int& out_w, int& out_h ) {
	if( size < sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ) return false;
	if( file[ 0 ] != 'B' || file[ 1 ] != 'M' ) return false;
	const uint32_t offbits = rd32( file + 10 );
	const unsigned char* info = file + sizeof( BITMAPFILEHEADER );
	if( rd32( info ) < sizeof( BITMAPINFOHEADER ) ) return false;	// OS/2 core headers not handled
	const int32_t w = (int32_t) rd32( info + 4 );
	const int32_t h_signed = (int32_t) rd32( info + 8 );
	const uint16_t bpp = rd16( info + 14 );
	const uint32_t compression = rd32( info + 16 );
	const uint32_t clr_used = rd32( info + 32 );
	if( w <= 0 || h_signed == 0 ) return false;
	if( compression != BI_RGB && compression != BI_BITFIELDS ) return false;
	const bool topdown = h_signed < 0;
	const int h = topdown ? -h_signed : h_signed;
	if( offbits >= size ) return false;

	// Palette (bpp <= 8)
	RGBQUAD palette[ 256 ];
	uint32_t palette_count = 0;
	if( bpp <= 8 ) {
		palette_count = clr_used ? clr_used : ( 1u << bpp );
		if( palette_count > 256 ) return false;
		const unsigned char* pal = info + rd32( info );
		if( (size_t)( pal - file ) + (size_t) palette_count * 4 > size ) return false;
		memcpy( palette, pal, palette_count * sizeof( RGBQUAD ) );
	}
	// Bitfield masks (16/32bpp)
	uint32_t masks[ 3 ] = { 0, 0, 0 };
	if( compression == BI_BITFIELDS && ( bpp == 16 || bpp == 32 ) ) {
		const unsigned char* m = info + rd32( info );
		if( (size_t)( m - file ) + 12 > size ) return false;
		masks[ 0 ] = rd32( m ); masks[ 1 ] = rd32( m + 4 ); masks[ 2 ] = rd32( m + 8 );
	} else if( bpp == 16 ) {
		masks[ 0 ] = 0x7C00; masks[ 1 ] = 0x03E0; masks[ 2 ] = 0x001F;	// 5-5-5
	} else if( bpp == 32 ) {
		masks[ 0 ] = 0x00FF0000; masks[ 1 ] = 0x0000FF00; masks[ 2 ] = 0x000000FF;
	}
	int shifts[ 3 ];
	uint32_t maxv[ 3 ];
	for( int i = 0; i < 3; ++i ) {
		uint32_t m = masks[ i ];
		int s = 0;
		while( m && !( m & 1 ) ) { m >>= 1; ++s; }
		shifts[ i ] = s;
		maxv[ i ] = m;
	}

	const uint32_t src_stride = ( (uint32_t) bpp * (uint32_t) w + 31 ) / 32 * 4;
	if( (size_t) offbits + (size_t) src_stride * h > size && bpp != 24 ) {
		// 24bpp files written by some tools carry a short final row; the
		// per-pixel reads below are all bounds-checked instead.
		if( (size_t) offbits + (size_t) src_stride * ( h - 1 ) + (size_t) w * bpp / 8 > size ) return false;
	}
	rgb.assign( (size_t) w * h * 3, 0 );
	for( int y = 0; y < h; ++y ) {
		const int src_y = topdown ? y : ( h - 1 - y );
		const unsigned char* row = file + offbits + (size_t) src_y * src_stride;
		unsigned char* dst = rgb.data() + (size_t) y * w * 3;
		for( int x = 0; x < w; ++x ) {
			uint32_t r = 0, g = 0, b = 0;
			const unsigned char* px = row + (size_t) x * bpp / 8;
			bool in_range = (size_t)( px - file ) + bpp / 8 <= size;
			switch( bpp ) {
			case 1: case 4: case 8: {
				uint32_t idx = 0;
				if( in_range ) {
					if( bpp == 8 ) idx = *px;
					else if( bpp == 4 ) idx = ( x & 1 ) ? ( *px & 0x0F ) : ( *px >> 4 );
					else idx = ( *( row + x / 8 ) >> ( 7 - ( x & 7 ) ) ) & 1;
				}
				if( idx < palette_count ) {
					r = palette[ idx ].rgbRed; g = palette[ idx ].rgbGreen; b = palette[ idx ].rgbBlue;
				}
				break;
			}
			case 16: case 32: {
				uint32_t v = 0;
				if( in_range ) v = ( bpp == 16 ) ? rd16( px ) : rd32( px );
				uint32_t ch[ 3 ];
				for( int i = 0; i < 3; ++i ) {
					ch[ i ] = maxv[ i ] ? ( ( ( v & masks[ i ] ) >> shifts[ i ] ) * 255 / maxv[ i ] ) : 0;
				}
				r = ch[ 0 ]; g = ch[ 1 ]; b = ch[ 2 ];
				break;
			}
			case 24:
				if( in_range ) { b = px[ 0 ]; g = px[ 1 ]; r = px[ 2 ]; }
				break;
			default:
				return false;
			}
			dst[ x * 3 + 0 ] = (unsigned char) r;
			dst[ x * 3 + 1 ] = (unsigned char) g;
			dst[ x * 3 + 2 ] = (unsigned char) b;
		}
	}
	out_w = w;
	out_h = h;
	return true;
}

char* load_image_dib( const char* filename ) {
	FILE* f = fopen( filename, "rb" );
	if( !f ) return nullptr;
	fseek( f, 0, SEEK_END );
	long fsize = ftell( f );
	fseek( f, 0, SEEK_SET );
	if( fsize <= 0 ) { fclose( f ); return nullptr; }
	std::vector<unsigned char> file( (size_t) fsize );
	if( fread( file.data(), 1, file.size(), f ) != file.size() ) { fclose( f ); return nullptr; }
	fclose( f );

	const bool is_bmp = file.size() >= 2 && file[ 0 ] == 'B' && file[ 1 ] == 'M';
	if( is_bmp && file.size() > sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ) {
		const unsigned char* info = file.data() + sizeof( BITMAPFILEHEADER );
		// The Windows application's fast path (PreView.cpp bdAddFile):
		// an uncompressed 24bpp BMP with the standard header is handed
		// to GetBmp() as-is, without going through WIC.
		if( rd32( file.data() + 10 ) == sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) &&
			rd16( info + 14 ) == 24 && rd32( info + 16 ) == BI_RGB ) {
			const size_t dib_size = file.size() - sizeof( BITMAPFILEHEADER );
			char* dib = (char*) malloc( dib_size );
			if( dib ) memcpy( dib, info, dib_size );
			return dib;
		}
		std::vector<unsigned char> rgb;
		int w, h;
		if( bmp_to_rgb( file.data(), file.size(), rgb, w, h ) ) {
			return make_dib24( rgb.data(), w, h );
		}
		return nullptr;
	}

	// PNG / JPEG (and anything else stb_image understands)
	int w, h, channels;
	unsigned char* pixels = stbi_load( filename, &w, &h, &channels, 3 );
	if( !pixels ) return nullptr;
	char* dib = make_dib24( pixels, w, h );
	stbi_image_free( pixels );
	return dib;
}
