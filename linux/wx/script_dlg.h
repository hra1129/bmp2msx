// ---------------------------------------------------------------------------
// Script window of the wxWidgets front end: the counterpart of the
// Windows script window (manual ch.9).  A text editor with New / Open /
// Save, a Run button, and a log view.  Execution uses the Phase 4 script
// engine (../src/script.cpp) unchanged, so the language -- commands,
// flags, constants, expressions, SCREEN 8P extension -- is exactly the
// CLI's.  Two presentation differences from the Windows GUI, both
// inherited from the headless engine: MSG output and the SETTING
// command's settings dump appear in the log instead of in windows of
// their own.
//
// Dropping an image file onto the window presets it as the conversion
// target, as in the Windows GUI.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_WX_SCRIPT_DLG_H
#define BMP2MSX_WX_SCRIPT_DLG_H

#include <wx/wx.h>
#include <functional>
#include <string>
#include "Convert.h"

class ScriptFrame : public wxFrame {
public:
	// current_settings supplies the SETTING a run starts from (the main
	// window's defaults at the time Run is pressed).
	ScriptFrame( wxWindow* parent, std::function<SETTING()> current_settings );

	void SetTarget( const wxString& path );

private:
	void OnNew( wxCommandEvent& );
	void OnOpen( wxCommandEvent& );
	void OnSave( wxCommandEvent& );
	void OnRun( wxCommandEvent& );
	void OnClose( wxCommandEvent& );

	std::function<SETTING()> m_current_settings;
	wxTextCtrl* m_editor = nullptr;
	wxTextCtrl* m_log = nullptr;
	wxStaticText* m_target_label = nullptr;
	std::string m_target;
	std::string m_script_path;
};

#endif
