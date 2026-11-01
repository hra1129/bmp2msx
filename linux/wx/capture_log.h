// ---------------------------------------------------------------------------
// PLATFORM SEAM (clearly marked): capturing C stdio output.
//
// The script engine (../src/script.cpp) reports through printf()/fprintf()
// exactly like the CLI.  The GUI's script window wants that text in its log
// view, so while a script runs, stdout/stderr are redirected to a temporary
// file and read back afterwards.
//
// Redirecting a FILE* portably has no standard-C solution, so this is one
// of the very few platform-specific spots in the GUI.  It is isolated in
// capture_log.cpp behind this class; POSIX (Linux) uses dup()/dup2(),
// Windows would use _dup()/_dup2() from <io.h> (already sketched there).
// Everything else in the GUI is plain wxWidgets + the shared core.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_WX_CAPTURE_LOG_H
#define BMP2MSX_WX_CAPTURE_LOG_H

#include <string>

class StdioCapture {
public:
	StdioCapture();
	~StdioCapture();

	// Start capturing stdout and stderr.  Returns false if the platform
	// redirection could not be set up (the script then runs uncaptured).
	bool Begin();
	// Stop capturing and return everything that was written.
	std::string End();

private:
	int m_saved_out = -1;
	int m_saved_err = -1;
	std::string m_path;
	bool m_active = false;
};

#endif
