// ---------------------------------------------------------------------------
// Conversion driver for the wxWidgets front end.
//
// The conversion flow is the same transcription of WndProc.cpp's
// Converter()/CreatePal() that the CLI uses (see ../src/convert_run.cpp);
// the difference is that this driver keeps everything the GUI needs
// afterwards -- the VRAM image for the preview renderer, the resized
// full-color source, the original image, and the effective SETTING
// (including the palette the conversion generated) -- instead of only
// writing files.  File output, when requested, goes through the same
// bsvSaveBmp() as everywhere else, so results are byte-identical.
//
// No wxWidgets (or OS) dependency: reusable by a future Windows build.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_WX_GUI_CONVERT_H
#define BMP2MSX_WX_GUI_CONVERT_H

#include <string>
#include <vector>
#include "Convert.h"

struct GuiPreview {
	SETTING mode;							// effective settings (palette filled in)
	std::vector<unsigned char> vram;		// 128 KB VRAM image (0x20000 bytes)
	int save_w = 0, save_h = 0;				// dimensions for bsvSaveBmp()
	std::vector<C_COLOR> source;			// resized input pixels (source_w x source_h)
	int source_w = 0, source_h = 0;
	std::vector<C_COLOR> original;			// untouched input pixels (orig_w x orig_h)
	int orig_w = 0, orig_h = 0;
	std::string input_path;					// file the conversion came from
};

// Convert one image file into a GuiPreview.  mode is taken by copy.
// On failure returns false and, if err is non-null, stores a short
// English reason.
bool gui_convert_image( const SETTING& mode, const std::string& input_path,
						GuiPreview& result, std::string* err );

// The palette file name the CLI derives for PL/CSV palette output
// ("<stem>.PL<digit>" / "<stem>.CSV" beside name_source), or "" when the
// mode does not write a separate palette file.
std::string gui_derived_palette_name( const SETTING& mode,
									  const std::string& name_source );

// Write a preview's output files with bsvSaveBmp(), the way a save of
// the preview does in the Windows GUI.  name_source is the file name the
// output names are derived from.  When the mode writes a PL/CSV palette
// file, palette_file is the name queued for its prompt; pass
// gui_derived_palette_name() for automatic naming, a user-chosen name
// for an interactive save, or "" to skip the palette file (the prompt
// is then treated as cancelled, as when a user cancels the dialog).
// mode is taken by copy so the caller can flip AutoName for a
// "save as".  Returns the bsvSaveBmp() result code (BSV_NOERR on
// success).
int gui_save_preview_files( const GuiPreview& pv, const SETTING& mode,
							const std::string& name_source,
							const std::string& palette_file );

#endif
