// See script_dlg.h.
#include "script_dlg.h"

#include <wx/dnd.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <cstdio>
#include <cstring>

#include "script.h"
#include "capture_log.h"

namespace {

class DropTarget : public wxFileDropTarget {
public:
	explicit DropTarget( ScriptFrame* frame ) : m_frame( frame ) {}
	bool OnDropFiles( wxCoord, wxCoord, const wxArrayString& files ) override {
		if( !files.empty() ) m_frame->SetTarget( files[ 0 ] );
		return true;
	}
private:
	ScriptFrame* m_frame;
};

} // namespace

ScriptFrame::ScriptFrame( wxWindow* parent, std::function<SETTING()> current_settings )
	: wxFrame( parent, wxID_ANY, "BMP2MSX script", wxDefaultPosition, wxSize( 720, 620 ) ),
	  m_current_settings( std::move( current_settings ) ) {
	auto* menu_bar = new wxMenuBar();
	auto* file_menu = new wxMenu();
	file_menu->Append( wxID_NEW, "New" );
	file_menu->Append( wxID_OPEN, "Open..." );
	file_menu->Append( wxID_SAVE, "Save..." );
	file_menu->AppendSeparator();
	file_menu->Append( wxID_CLOSE, "Close" );
	menu_bar->Append( file_menu, "File" );
	auto* run_menu = new wxMenu();
	run_menu->Append( wxID_EXECUTE, "Run" );
	menu_bar->Append( run_menu, "Run" );
	SetMenuBar( menu_bar );

	Bind( wxEVT_MENU, &ScriptFrame::OnNew, this, wxID_NEW );
	Bind( wxEVT_MENU, &ScriptFrame::OnOpen, this, wxID_OPEN );
	Bind( wxEVT_MENU, &ScriptFrame::OnSave, this, wxID_SAVE );
	Bind( wxEVT_MENU, &ScriptFrame::OnClose, this, wxID_CLOSE );
	Bind( wxEVT_MENU, &ScriptFrame::OnRun, this, wxID_EXECUTE );

	auto* top = new wxBoxSizer( wxVERTICAL );
	m_target_label = new wxStaticText( this, wxID_ANY, "Target: (none)" );
	top->Add( m_target_label, 0, wxALL, 6 );

	m_editor = new wxTextCtrl( this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE | wxTE_DONTWRAP | wxHSCROLL );
	m_editor->SetFont( wxFont( 10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
							   wxFONTWEIGHT_NORMAL ) );
	top->Add( m_editor, 1, wxEXPAND | wxALL, 4 );

	auto* bar = new wxBoxSizer( wxHORIZONTAL );
	auto* run_btn = new wxButton( this, wxID_EXECUTE, "Run" );
	run_btn->Bind( wxEVT_BUTTON, &ScriptFrame::OnRun, this );
	bar->Add( run_btn, 0, wxALL, 3 );
	top->Add( bar, 0, wxEXPAND );

	m_log = new wxTextCtrl( this, wxID_ANY, "",
		wxDefaultPosition, wxSize( -1, 150 ),
		wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxHSCROLL );
	m_log->SetFont( wxFont( 9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
							wxFONTWEIGHT_NORMAL ) );
	top->Add( m_log, 0, wxEXPAND | wxALL, 4 );

	SetSizer( top );
	SetDropTarget( new DropTarget( this ) );
}

void ScriptFrame::SetTarget( const wxString& path ) {
	m_target = path.utf8_str().data();
	m_target_label->SetLabel( wxString::Format( "Target: %s", path ) );
}

void ScriptFrame::OnNew( wxCommandEvent& ) {
	m_editor->Clear();
	m_script_path.clear();
}

void ScriptFrame::OnOpen( wxCommandEvent& ) {
	wxFileDialog dlg( this, "Open script", "", "",
					  "Script files (*.txt)|*.txt|All files|*.*", wxFD_OPEN );
	if( dlg.ShowModal() != wxID_OK ) return;
	// Load as raw bytes and let wxString decode UTF-8; Shift_JIS scripts
	// are the engine's business when *running* -- for editing, decode
	// with the wxCSConv fallback so the text shows readably.
	wxString path = dlg.GetPath();
	m_editor->LoadFile( path );
	m_script_path = path.utf8_str().data();
}

void ScriptFrame::OnSave( wxCommandEvent& ) {
	wxFileDialog dlg( this, "Save script", "", "script.txt",
					  "Script files (*.txt)|*.txt|All files|*.*",
					  wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	if( dlg.ShowModal() != wxID_OK ) return;
	m_editor->SaveFile( dlg.GetPath() );
	m_script_path = dlg.GetPath().utf8_str().data();
}

void ScriptFrame::OnClose( wxCommandEvent& ) { Close(); }

void ScriptFrame::OnRun( wxCommandEvent& ) {
	// The engine runs from a file; write the editor text to a scratch
	// file (or reuse the loaded path if the text is unmodified is not
	// tracked -- always write the current text so edits take effect).
	wxString tmp = wxFileName::CreateTempFileName( "bmp2msx_script" );
	{
		wxString text = m_editor->GetValue();
		FILE* f = fopen( tmp.utf8_str().data(), "wb" );
		if( !f ) {
			m_log->AppendText( "error: cannot create a temporary script file\n" );
			return;
		}
		wxCharBuffer utf8 = text.utf8_str();
		fwrite( utf8.data(), 1, strlen( utf8.data() ), f );
		fclose( f );
	}
	SETTING initial = m_current_settings();
	m_log->AppendText( "--- run ---\n" );
	wxBusyCursor busy;
	StdioCapture capture;
	capture.Begin();
	int rc = cli_run_script( tmp.utf8_str().data(), initial, m_target, "" );
	std::string out = capture.End();
	wxRemoveFile( tmp );
	if( !out.empty() ) m_log->AppendText( wxString::FromUTF8( out ) );
	m_log->AppendText( wxString::Format( "--- finished (rc=%d) ---\n", rc ) );
}
