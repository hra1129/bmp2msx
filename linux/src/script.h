// ---------------------------------------------------------------------------
// Text script interpreter for the Linux CLI: a headless re-implementation
// of the script engine in ../source/ScriptProc.cpp, accepting the same
// language (same command words in English and Japanese, same flag and
// constant names — including the historical INTERACE spelling — and the
// same numeric expressions, evaluated by the real calc.cpp).
//
// Differences from the GUI version, all forced by headless operation:
//   - MSG prints to stdout instead of showing a message box.
//   - SETTING prints the current settings instead of opening the dialog.
//   - LOADSETTING reads a .B2M file with the same code the GUI uses
//     (utils.cpp GetCfgFile); settings are NOT loaded from any default
//     file first — a script starts from the CLI's initial settings.
//   - Errors are reported on stderr and abort the script with a
//     non-zero exit status (the GUI shows a dialog and stops).
//   - Extension: SCREEN 8P and SCREEN 8P256 select the Phase 3 EPAL
//     modes, which the GUI script grammar (numeric only) cannot name.
// Script text and file names are UTF-8.  A script file that is not
// valid UTF-8 is treated as Shift_JIS and converted (older scripts were
// written on Windows), via iconv.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_CLI_SCRIPT_H
#define BMP2MSX_CLI_SCRIPT_H

#include <string>
#include "Convert.h"

// Run a script file.  initial is the starting SETTING (CLI defaults plus
// any command-line options); initial_target presets the conversion
// target the way dropping a file on the GUI script window does.
// output_dir is forwarded to conversions ("" = beside each input).
// Returns 0 on success, non-zero on error (message printed to stderr).
int cli_run_script( const char* script_path, const SETTING& initial,
					const std::string& initial_target, const std::string& output_dir );

#endif
