// ---------------------------------------------------------------------------
// Minimal <windows.h> replacement for building the BMP2MSX conversion core
// (../source/Convert.cpp, bsave.cpp, utils.cpp, calc.cpp) on Linux.
//
// Only the declarations those files actually use are provided.  All GUI
// entry points are backed by the stubs in src/platform_linux.cpp; the
// conversion results do not depend on any of them.
//
// NOTE: LONG/DWORD/WORD are fixed-width here so that BITMAPINFOHEADER is
// exactly 40 bytes, matching the Win32 layout the BMP decoder in utils.cpp
// relies on.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_LINUX_WINDOWS_SHIM_H
#define BMP2MSX_LINUX_WINDOWS_SHIM_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>

typedef void* HWND;
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HBRUSH;
typedef void* HANDLE;
typedef void* HINSTANCE;
typedef void* HMENU;
typedef void* HGDIOBJ;
typedef unsigned char BYTE;
typedef BYTE* LPBYTE;
typedef uint32_t DWORD;
typedef unsigned int UINT;
typedef int32_t LONG;
typedef uint16_t WORD;
typedef int BOOL;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_PATH 260
#define LMEM_FIXED 0x0000
#define LMEM_ZEROINIT 0x0040
#define LPTR ( LMEM_FIXED | LMEM_ZEROINIT )

static inline void* LocalAlloc( unsigned int uFlags, size_t n ) {
	void* p = malloc( n ? n : 1 );
	if( p && ( uFlags & LMEM_ZEROINIT ) ) memset( p, 0, n );
	return p;
}
static inline void* LocalFree( void* p ) { free( p ); return nullptr; }

#define ZeroMemory( p, n ) memset( (p), 0, (n) )
#define CopyMemory( d, s, n ) memcpy( (d), (s), (n) )

typedef struct tagRECT { LONG left, top, right, bottom; } RECT;

#pragma pack( push, 1 )
typedef struct tagBITMAPFILEHEADER {
	WORD bfType; DWORD bfSize; WORD bfReserved1; WORD bfReserved2; DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack( pop )

typedef struct tagBITMAPINFOHEADER {
	DWORD biSize; LONG biWidth; LONG biHeight; WORD biPlanes; WORD biBitCount;
	DWORD biCompression; DWORD biSizeImage; LONG biXPelsPerMeter; LONG biYPelsPerMeter;
	DWORD biClrUsed; DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	BYTE rgbBlue; BYTE rgbGreen; BYTE rgbRed; BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[ 1 ];
} BITMAPINFO;

#define BI_RGB 0
#define BI_BITFIELDS 3
#define DIB_RGB_COLORS 0
#define BLACK_BRUSH 4

#define RGB( r, g, b ) ( (uint32_t)( (uint8_t)(r) | ( (uint16_t)( (uint8_t)(g) ) << 8 ) | ( (uint32_t)( (uint8_t)(b) ) << 16 ) ) )

// String helpers (ANSI builds of the Win32 string functions used by the core)
static inline char* lstrcpy( char* dst, const char* src ) { return strcpy( dst, src ); }
static inline int lstrlen( const char* s ) { return (int) strlen( s ); }

// GDI / window stubs (implemented in src/platform_linux.cpp)
HDC GetDC( HWND );
HDC CreateCompatibleDC( HDC );
int ReleaseDC( HWND, HDC );
HBITMAP CreateDIBSection( HDC, const BITMAPINFO*, UINT, void**, HANDLE, DWORD );
HGDIOBJ SelectObject( HDC, HGDIOBJ );
bool GdiFlush( void );
bool DeleteDC( HDC );
bool DeleteObject( HGDIOBJ );
int SetPixel( HDC, int, int, DWORD );
int FillRect( HDC, const RECT*, HBRUSH );
HGDIOBJ GetStockObject( int );

int MessageBox( HWND, const char* text, const char* caption, UINT type );
#define MB_OK 0
#define MB_ICONWARNING 0x30
#define MB_YESNO 0x04
#define MB_DEFBUTTON2 0x0100
#define IDYES 6
#define IDNO 7

int wsprintf( char* dst, const char* fmt, ... );

// Common save-file dialog: on Linux there is no dialog; the CLI queues the
// file name it wants (see cli_set_save_filename in src/platform_linux.cpp)
// and GetSaveFileName() hands it back, mirroring a user who always accepts
// the suggested name.
typedef struct tagOFN {
	DWORD lStructSize;
	HWND hwndOwner;
	HINSTANCE hInstance;
	const char* lpstrFilter;
	char* lpstrCustomFilter;
	DWORD nMaxCustFilter;
	DWORD nFilterIndex;
	char* lpstrFile;
	DWORD nMaxFile;
	char* lpstrFileTitle;
	DWORD nMaxFileTitle;
	const char* lpstrInitialDir;
	const char* lpstrTitle;
	DWORD Flags;
	WORD nFileOffset;
	WORD nFileExtension;
	const char* lpstrDefExt;
	void* lCustData;
	void* lpfnHook;
	const char* lpTemplateName;
} OPENFILENAME;

#define OFN_LONGNAMES 0x00200000
#define OFN_HIDEREADONLY 0x00000004
#define OFN_OVERWRITEPROMPT 0x00000002
#define OFN_NOCHANGEDIR 0x00000008

int GetSaveFileName( OPENFILENAME* ofn );

#endif // BMP2MSX_LINUX_WINDOWS_SHIM_H
