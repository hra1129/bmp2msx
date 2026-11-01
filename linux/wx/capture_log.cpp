// See capture_log.h -- this file is the marked platform seam for stdio
// capture.  Keep all OS-specific calls for it inside this file.
#include "capture_log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#if defined( _WIN32 )
// Future Windows build: the MSVC runtime offers the same descriptor
// duplication through <io.h>; the shape below already matches it.
#include <io.h>
#define DUP _dup
#define DUP2 _dup2
#define CLOSE _close
#define FILENO_OUT _fileno( stdout )
#define FILENO_ERR _fileno( stderr )
#else
#include <unistd.h>
#define DUP dup
#define DUP2 dup2
#define CLOSE close
#define FILENO_OUT STDOUT_FILENO
#define FILENO_ERR STDERR_FILENO
#endif

StdioCapture::StdioCapture() = default;

StdioCapture::~StdioCapture() {
	if( m_active ) End();
}

bool StdioCapture::Begin() {
	char tmpl[] = "/tmp/bmp2msx_log_XXXXXX";
#if defined( _WIN32 )
	// _mktemp is the MSVC counterpart of mkstemp; the descriptor dance
	// below is identical.
	if( _mktemp( tmpl ) == nullptr ) return false;
	m_path = tmpl;
	FILE* f = fopen( m_path.c_str(), "w+" );
	if( !f ) return false;
	int fd = _fileno( f );
#else
	int fd = mkstemp( tmpl );
	if( fd < 0 ) return false;
	m_path = tmpl;
	FILE* f = fdopen( fd, "w+" );
	if( !f ) { CLOSE( fd ); return false; }
	fd = fileno( f );
#endif
	fflush( stdout );
	fflush( stderr );
	m_saved_out = DUP( FILENO_OUT );
	m_saved_err = DUP( FILENO_ERR );
	if( m_saved_out < 0 || m_saved_err < 0 ) {
		fclose( f );
		return false;
	}
	DUP2( fd, FILENO_OUT );
	DUP2( fd, FILENO_ERR );
	fclose( f );	// descriptors 1/2 now own the file
	m_active = true;
	return true;
}

std::string StdioCapture::End() {
	if( !m_active ) return std::string();
	fflush( stdout );
	fflush( stderr );
	DUP2( m_saved_out, FILENO_OUT );
	DUP2( m_saved_err, FILENO_ERR );
	CLOSE( m_saved_out );
	CLOSE( m_saved_err );
	m_saved_out = m_saved_err = -1;
	m_active = false;

	std::ifstream in( m_path, std::ios::binary );
	std::stringstream ss;
	ss << in.rdbuf();
	in.close();
	remove( m_path.c_str() );
	return ss.str();
}
