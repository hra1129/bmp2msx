// ---------------------------------------------------------------------------
// Headless conversion driver: the Linux counterpart of WndProc.cpp's
// Converter() + CreatePal().  The conversion logic is transcribed from
// the Windows code with only the GUI parts removed (preview bookkeeping,
// progress dialog, MessageBox) so that, given the same SETTING and the
// same input pixels, the byte stream handed to bsvSaveBmp() is identical.
//
// Output file naming: the GUI derives output names from the input file
// name (AutoName) and asks for the palette file name (PL/CSV) through a
// dialog.  Here AutoName is always on, output goes to output_dir (or
// beside the input when empty), and the palette file name is queued for
// the dialog stub as "<stem>.PL<mode digit>" / "<stem>.CSV".
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_CLI_CONVERT_RUN_H
#define BMP2MSX_CLI_CONVERT_RUN_H

#include <string>
#include "Convert.h"

// Convert one image file.  mode is taken by copy (PreView is forced to
// "file output only" and AutoName to true).  On failure returns false
// and, if err is non-null, stores a short English reason.
bool cli_convert_file( const SETTING& mode, const std::string& input_path,
					   const std::string& output_dir, std::string* err );

// Human-readable dump of the conversion-relevant SETTING fields
// (used by --show-settings and the script SETTING command).
std::string cli_describe_setting( const SETTING& mode );

#endif
