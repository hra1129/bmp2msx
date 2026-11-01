// ---------------------------------------------------------------------------
// Portable preview renderer for the wxWidgets front end.
//
// This is a platform-independent transcription of ../source/winConvert.cpp's
// draw_screen(): instead of SetPixel() calls on an HDC it fills a caller
// provided RGB buffer (3 bytes per pixel, R,G,B order, top row first) of
// PREVIEW_W x PREVIEW_H pixels -- exactly the 512x424 canvas the Windows
// preview paints (every mode is drawn at 2x, on a black background).
//
// The file deliberately has no wxWidgets (or OS) dependency: the same
// renderer can be reused by a future Windows build of this GUI.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_WX_PREVIEW_RENDER_H
#define BMP2MSX_WX_PREVIEW_RENDER_H

#include "Convert.h"

enum { PREVIEW_W = 512, PREVIEW_H = 424 };

// Render the converted VRAM image (the 128 KB buffer produced by
// cnvRecolor()/cnvNtcolor(), as stored with a preview) into rgb_out,
// which must hold PREVIEW_W * PREVIEW_H * 3 bytes.
void render_converted_preview( const unsigned char* bmp, const SETTING* mode,
							   unsigned char* rgb_out );

// Copy a full-color pixel buffer (C_COLOR array, top row first, as
// produced by GetBmp() and the resize/copy stage) into rgb_out, which
// must hold w * h * 3 bytes.
void render_color_buffer( const C_COLOR* px, int w, int h, unsigned char* rgb_out );

#endif
