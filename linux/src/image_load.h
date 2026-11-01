// ---------------------------------------------------------------------------
// Image loading for the Linux CLI: the counterpart of the Windows loader
// pair (PreView.cpp's bdAddFile + winImage.cpp's WIC-based decoder).
//
// load_image_dib() returns a malloc()ed buffer whose layout is exactly
// what the Windows code hands to GetBmp() (utils.cpp):
//   - a BITMAPINFOHEADER, followed by the pixel data.
// For an uncompressed 24bpp BMP with the standard 54-byte header this is
// the file content after the BITMAPFILEHEADER, unchanged (the same fast
// path the Windows application takes).  Every other supported input
// (BMP at 1/4/8/16/32bpp, top-down BMP, PNG, JPEG) is normalized to a
// 24bpp bottom-up BGR DIB, which is what the WIC path produces on
// Windows.  PNG/JPEG decoding uses stb_image (see third_party/).
//
// Returns nullptr on failure.  Free the result with free().
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_CLI_IMAGE_LOAD_H
#define BMP2MSX_CLI_IMAGE_LOAD_H

char* load_image_dib( const char* filename );

#endif
