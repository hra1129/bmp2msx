// ---------------------------------------------------------------------------
// Linux backing for the Win32 entry points the BMP2MSX conversion core
// references.  None of these participate in the conversion math:
//
// - MessageBox() prints to stderr (the core only uses it for save errors).
// - The GDI functions exist only so Convert.cpp's preview-BMP writer
//   (cnvSaveBmpFile) links; the CLI never calls it (it renders through
//   draw_screen(), a GUI preview function, stubbed as a no-op here).
// - GetSaveFileName() returns the file name queued by the CLI.  In the
//   Windows GUI, bsave.cpp asks the user for the palette file name
//   (PL/CSV output) through utils.cpp's GetName(); headless, the CLI
//   queues the deterministic name it derived from the input file, which
//   is equivalent to the user accepting that name in the dialog.
// ---------------------------------------------------------------------------
#include <windows.h>
#include "Convert.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>

// Referenced as an extern by ../source/utils.cpp.
HINSTANCE hIns = nullptr;

// ---------------------------------------------------------------- GDI stubs
HDC GetDC( HWND ) { return nullptr; }
HDC CreateCompatibleDC( HDC ) { return nullptr; }
int ReleaseDC( HWND, HDC ) { return 0; }
HBITMAP CreateDIBSection( HDC, const BITMAPINFO*, UINT, void**, HANDLE, DWORD ) { return nullptr; }
HGDIOBJ SelectObject( HDC, HGDIOBJ ) { return nullptr; }
bool GdiFlush( void ) { return true; }
bool DeleteDC( HDC ) { return true; }
bool DeleteObject( HGDIOBJ ) { return true; }
int SetPixel( HDC, int, int, DWORD ) { return 0; }
int FillRect( HDC, const RECT*, HBRUSH ) { return 0; }
HGDIOBJ GetStockObject( int ) { return nullptr; }

// Preview rendering is a GUI feature; there is nothing to draw headless.
void draw_screen( const unsigned char*, HDC, const SETTING* ) {}

// ---------------------------------------------------------------- dialogs
int MessageBox( HWND, const char* text, const char*, UINT ) {
	fprintf( stderr, "bmp2msx-cli: %s\n", text ? text : "" );
	return 0;
}

int wsprintf( char* dst, const char* fmt, ... ) {
	va_list ap;
	va_start( ap, fmt );
	int n = vsprintf( dst, fmt, ap );
	va_end( ap );
	return n;
}

// ------------------------------------------------- save-file name hand-off
static std::string g_save_filename;
static bool g_save_filename_set = false;

// Queue the file name the next GetSaveFileName() call should return
// (used for the PL/CSV palette file that bsave.cpp asks about).
void cli_set_save_filename( const char* filename ) {
	g_save_filename = filename ? filename : "";
	g_save_filename_set = filename != nullptr;
}

int GetSaveFileName( OPENFILENAME* ofn ) {
	if( !g_save_filename_set || !ofn || !ofn->lpstrFile || ofn->nMaxFile == 0 ) return 0;
	strncpy( ofn->lpstrFile, g_save_filename.c_str(), ofn->nMaxFile - 1 );
	ofn->lpstrFile[ ofn->nMaxFile - 1 ] = '\0';
	return 1;
}
