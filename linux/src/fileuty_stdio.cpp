// ---------------------------------------------------------------------------
// stdio implementation of the fileuty.h interface (../source/fileuty.h,
// non-Win32 branch) for the Linux build of BMP2MSX.
//
// The Win32 implementation in ../source/fileuty.cpp cannot be used here
// (CreateFile/ReadFile/...), and fileuty.h explicitly anticipates a
// per-platform implementation ("for Others").  The functions the
// conversion core actually exercises (fu_open / fu_read / fu_write /
// fu_printf / fu_seek / fu_close / fu_eof / fu_length) behave byte-for-byte
// like the Win32 originals: fu_printf() formats into a buffer and writes
// it raw, exactly as the original does.
//
// The two memory modes are implemented with the documented semantics
// (sequential access from the current pointer).  No product code path in
// the conversion core uses them.
// ---------------------------------------------------------------------------
#include "fileuty.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>

static FU_FILE* _fu_alloc( FU_MODE mode ) {
	FU_FILE* p_file = (FU_FILE*) calloc( 1, sizeof( FU_FILE ) );
	if( p_file ) p_file->n_mode = mode;
	return p_file;
}

FU_FILE* fu_open( const char* s_filename, const char* s_option ) {
	if( s_option == NULL ) s_option = "rb";
	// Option letters may appear in any order; the last of R/W/A and the
	// last of T/B wins, mirroring the Win32 implementation.
	char access = 'r';
	bool binary = false;
	bool plus = false;
	for( const char* p = s_option; *p; ++p ) {
		switch( *p ) {
		case 'r': case 'R': access = 'r'; break;
		case 'w': case 'W': access = 'w'; break;
		case 'a': case 'A': access = 'a'; break;
		case 'b': case 'B': binary = true; break;
		case 't': case 'T': binary = false; break;
		case '+': plus = true; break;
		}
	}
	char mode[ 4 ];
	int m = 0;
	mode[ m++ ] = access;
	if( plus ) mode[ m++ ] = '+';
	mode[ m++ ] = 'b';	// Linux stdio makes no text/binary distinction.
	mode[ m ] = '\0';

	FILE* f = fopen( s_filename, mode );
	if( !f ) return NULL;
	FU_FILE* p_file = _fu_alloc( FU_MODE_FILE );
	if( !p_file ) { fclose( f ); return NULL; }
	p_file->h_file = f;
	p_file->b_binary = binary;
	return p_file;
}

FU_FILE* fu_open_linkmemory( void* p_memory, dword n_size ) {
	if( !p_memory ) return NULL;
	FU_FILE* p_file = _fu_alloc( FU_MODE_LINKMEMORY );
	if( !p_file ) return NULL;
	p_file->h_file = (FU_FILE_HANDLE) p_memory;	// any non-null marker
	p_file->b_binary = true;
	p_file->p_memory = (byte*) p_memory;
	p_file->n_size = n_size;
	p_file->n_pointer = 0;
	return p_file;
}

FU_FILE* fu_open_allocmemory( dword n_size ) {
	byte* mem = (byte*) calloc( n_size ? n_size : 1, 1 );
	if( !mem ) return NULL;
	FU_FILE* p_file = fu_open_linkmemory( mem, n_size );
	if( !p_file ) { free( mem ); return NULL; }
	p_file->n_mode = FU_MODE_ALLOCMEMORY;
	return p_file;
}

void fu_close( FU_FILE* p_file ) {
	if( !p_file ) return;
	switch( p_file->n_mode ) {
	case FU_MODE_FILE:
		if( p_file->h_file ) fclose( p_file->h_file );
		break;
	case FU_MODE_ALLOCMEMORY:
		free( p_file->p_memory );
		break;
	case FU_MODE_LINKMEMORY:
		break;
	}
	free( p_file );
}

void fu_printf( FU_FILE* p_file, const char* s_format, ... ) {
	if( !p_file ) return;
	char buf[ FU_BUFFER_LENGTH ];
	va_list ap;
	va_start( ap, s_format );
	vsnprintf( buf, sizeof( buf ), s_format, ap );
	va_end( ap );
	fu_write( p_file, buf, (int) strlen( buf ) );
}

bool fu_eof( FU_FILE* p_file ) {
	if( !p_file ) return true;
	if( p_file->n_mode != FU_MODE_FILE ) return p_file->n_pointer >= p_file->n_size;
	byte b;
	if( fu_read( p_file, &b, 1 ) == 0 ) return true;
	fu_seek( p_file, -1, FU_SEEK_CURRENT );
	return false;
}

char* fu_gets( FU_FILE* p_file, char* s_buffer, int n_size ) {
	s_buffer[ 0 ] = '\0';
	if( !p_file || n_size <= 0 ) return s_buffer;
	int i = 0;
	while( i < n_size - 1 ) {
		char c;
		if( fu_read( p_file, &c, 1 ) == 0 ) break;
		s_buffer[ i++ ] = c;
		if( c == '\n' ) break;
	}
	s_buffer[ i ] = '\0';
	// Text mode: strip a trailing CR of a CRLF pair (the original's
	// conversion helper is only reachable from here and is unused by the
	// conversion core).
	if( !p_file->b_binary && i >= 2 && s_buffer[ i - 2 ] == '\r' && s_buffer[ i - 1 ] == '\n' ) {
		s_buffer[ i - 2 ] = '\n';
		s_buffer[ i - 1 ] = '\0';
	}
	return s_buffer;
}

int fu_write( FU_FILE* p_file, const void* s_buffer, int n_size ) {
	if( !p_file || n_size <= 0 ) return 0;
	if( p_file->n_mode == FU_MODE_FILE ) {
		return (int) fwrite( s_buffer, 1, n_size, p_file->h_file );
	}
	dword avail = ( p_file->n_pointer < p_file->n_size ) ? ( p_file->n_size - p_file->n_pointer ) : 0;
	dword n = ( (dword) n_size < avail ) ? (dword) n_size : avail;
	memcpy( p_file->p_memory + p_file->n_pointer, s_buffer, n );
	p_file->n_pointer += n;
	return (int) n;
}

int fu_read( FU_FILE* p_file, void* s_buffer, int n_size ) {
	if( !p_file || n_size <= 0 ) return 0;
	if( p_file->n_mode == FU_MODE_FILE ) {
		return (int) fread( s_buffer, 1, n_size, p_file->h_file );
	}
	dword avail = ( p_file->n_pointer < p_file->n_size ) ? ( p_file->n_size - p_file->n_pointer ) : 0;
	dword n = ( (dword) n_size < avail ) ? (dword) n_size : avail;
	memcpy( s_buffer, p_file->p_memory + p_file->n_pointer, n );
	p_file->n_pointer += n;
	return (int) n;
}

bool fu_seek( FU_FILE* p_file, sint32 n_offset, int n_origin ) {
	if( !p_file ) return false;
	if( p_file->n_mode == FU_MODE_FILE ) {
		return fseek( p_file->h_file, n_offset, n_origin ) == 0;
	}
	long base;
	switch( n_origin ) {
	case FU_SEEK_TOP: base = 0; break;
	case FU_SEEK_CURRENT: base = (long) p_file->n_pointer; break;
	case FU_SEEK_BOTTOM: base = (long) p_file->n_size; break;
	default: return false;
	}
	long pos = base + n_offset;
	if( pos < 0 || pos > (long) p_file->n_size ) return false;
	p_file->n_pointer = (dword) pos;
	return true;
}

unsigned long fu_length( FU_FILE* p_file ) {
	if( !p_file ) return 0;
	if( p_file->n_mode != FU_MODE_FILE ) return p_file->n_size;
	FILE* f = p_file->h_file;
	long cur = ftell( f );
	if( cur < 0 ) return 0;
	if( fseek( f, 0, SEEK_END ) != 0 ) return 0;
	long size = ftell( f );
	fseek( f, cur, SEEK_SET );
	return size < 0 ? 0 : (unsigned long) size;
}
