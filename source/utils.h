// -------------------------------------------------------------
// BMP to MSX
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _UTILS_H_
#define _UTILS_H_

#include <windows.h>
#include "config.h"
#include "convert.h"

bool GetName( HWND hWnd,char *szFileName,int size,const char *szTitle,const char *szFilter, const char* szExt );
bool GetBmp( C_COLOR **out,int *width,int *height,char **ptr );
bool GetBmp1( BITMAPINFOHEADER *bih,char **ptr,C_COLOR *out );
bool GetBmp4( BITMAPINFOHEADER *bih,char **ptr,C_COLOR *out );
bool GetBmp8( BITMAPINFOHEADER *bih,char **ptr,C_COLOR *out );
bool GetBmp16( BITMAPINFOHEADER *bih,char **ptr,C_COLOR *out );
bool GetBmp24( BITMAPINFOHEADER *bih,char **ptr,C_COLOR *out );
bool GetBmp32( BITMAPINFOHEADER *bih,char **ptr,C_COLOR *out );
void GetDefCfg( SETTING *Mode );
void set_msx1_palette( C_PALETTE *pal );
void set_msx2_palette( C_PALETTE *pal );
void GetDefCustom( void );
bool GetCfgFile( SETTING *Mode,const char *sCfgFile );
void SaveCfgFile( SETTING *Mode,const char *sCfgFile );
bool GetPathFile( const char *sPathFile, char *sPath, int len );
void SavePathFile( const char *sPathFile, char *sPath, int len );
bool GetCustomFile( const char *sCustomFile );
void SaveCustomFile( const char *sCustomFile );

#endif	// _UTILS_H_
