// -------------------------------------------------------------
// BMP to MSX
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _PROGPROC_H_
#define	_PROGPROC_H_

#include "winEvent.h"

void prShow( const char* cszText );
void prSetText( const char* cszText );
void prDestroy( void );
bool prProg( int p );

#endif	// ndef _PROGPROC_H_
