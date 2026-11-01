// ---------------------------------------------------------------------------
// bmp2msx-wx: wxWidgets front end for BMP2MSX (Linux build; the source
// is kept portable -- plain wxWidgets + the shared conversion core, no
// OS-specific calls -- so it can be built on Windows later as well).
//
// The main window mirrors the Windows GUI (manual ch.4): converted
// previews accumulate (up to 50), the space key toggles between the
// converted image and its resized source, the arrow keys walk the
// preview list, F7/F8 zoom, clicking the image opens the default
// settings (when empty) or the re-convert dialog (on a preview), and
// the settings summary under the image is blue for the converted view
// and red for the source view.
//
// Startup/shutdown follow the Windows application: BMP2MSX_CUSTOM.BIN
// (calibration data) is honored if present, the default settings are
// read from BMP2MSX.B2M beside the executable and written back there
// on exit.
//
// Test hooks (used by the build's verification, harmless in normal use):
//   --smoke-convert <image> <outdir>   convert with the GUI code path
//                                      (SCREEN8 EPAL + BAS, the settings
//                                      of the Phase 3/4 oracle) and exit
//   --snapshot <image> <out.png>       show the main window with the
//                                      image converted, save a PNG of
//                                      it, and exit
//   --snapshot-settings <out.png>      same for the settings dialog
// ---------------------------------------------------------------------------
#include <wx/wx.h>
#include <wx/clipbrd.h>
#include <wx/dnd.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/stdpaths.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Config.h"
#include "Convert.h"
#include "utils.h"
#include "convert_run.h"	// cli_describe_setting()
#include "gui_convert.h"
#include "preview_render.h"
#include "settings_dlg.h"
#include "script_dlg.h"
#include "original_wnd.h"

class MainFrame;

class Bmp2msxApp : public wxApp {
public:
	bool OnInit() override;
	int OnExit() override;

	const SETTING& DefaultSetting() const { return m_default; }
	void SetDefaultSetting( const SETTING& s ) { m_default = s; }
	void SaveDefaults();

private:
	SETTING m_default;
	std::string m_exe_dir;
	MainFrame* m_frame = nullptr;
};

wxDECLARE_APP( Bmp2msxApp );

// ---------------------------------------------------------------------------

class MainFrame : public wxFrame {
public:
	MainFrame();

	bool ConvertFile( const wxString& path );	// also used by the snapshot hook
	void ShowSettingsDialogShot();				// snapshot hook support

private:
	enum {
		ID_SCRIPT_MODE = wxID_HIGHEST + 1,
		ID_CLIPBOARD,
		ID_PREV_NEXT, ID_PREV_PREV, ID_PREV_TOGGLE, ID_PREV_ORIGINAL,
		ID_PREV_CLEAR, ID_PREV_CLEAR_ALL, ID_PREV_SAVE_AS, ID_PREV_SAVE_BMP,
		ID_PREV_RECONVERT,
		ID_SET_DEFAULTS, ID_SET_SHOW_TEXT, ID_SET_RESET, ID_SET_CALIBRATION,
		ID_VERSION,
	};

	SETTING PickSettingForNewConversion() const;
	bool ConvertWith( const wxString& path, const SETTING& setting );
	void AfterConvertSave( const GuiPreview& pv );
	void SaveCurrent( bool save_as );
	void SaveCurrentAsBmp();
	void ShowOriginalWindow();
	void OpenDefaultSettings();
	void OpenReconvert();
	void UpdateView();
	void UpdateSummary();
	wxImage CurrentDisplayImage();

	void OnMenu( wxCommandEvent& e );
	void OnCanvasPaint( wxPaintEvent& );
	void OnCanvasClick( wxMouseEvent& );
	void OnCharHook( wxKeyEvent& e );
	void OnClose( wxCloseEvent& );

	std::vector<GuiPreview> m_previews;
	int m_current = -1;
	bool m_show_source = false;
	int m_zoom = 1;
	SETTING m_last;
	ScriptFrame* m_script_frame = nullptr;

	wxScrolledWindow* m_scroll = nullptr;
	wxPanel* m_canvas = nullptr;
	wxPanel* m_summary_panel = nullptr;
	wxStaticText* m_summary = nullptr;
	wxMenuItem* m_show_text_item = nullptr;

	// rendered-image cache
	wxImage m_cache_img;
	int m_cache_index = -2;
	bool m_cache_source = false;
};

// ---------------------------------------------------------------------------

namespace {

std::string ExeDir() {
	wxFileName fn( wxStandardPaths::Get().GetExecutablePath() );
	return fn.GetPath().utf8_str().data();
}

// Capture a top-level window's on-screen rectangle into a PNG.
// (Portable wxWidgets: wxScreenDC blit -- no OS calls.)
bool CaptureWindow( wxTopLevelWindow* win, const wxString& out_png ) {
	win->Raise();
	wxSize sz = win->GetSize();
	wxPoint pos = win->GetScreenPosition();
	wxBitmap bmp( sz.x, sz.y );
	{
		wxMemoryDC mem( bmp );
		wxScreenDC screen;
		mem.Blit( 0, 0, sz.x, sz.y, &screen, pos.x, pos.y );
	}
	return bmp.SaveFile( out_png, wxBITMAP_TYPE_PNG );
}

class FileDropTarget : public wxFileDropTarget {
public:
	explicit FileDropTarget( MainFrame* frame ) : m_frame( frame ) {}
	bool OnDropFiles( wxCoord, wxCoord, const wxArrayString& files ) override {
		if( !files.empty() ) m_frame->ConvertFile( files[ 0 ] );
		return true;
	}
private:
	MainFrame* m_frame;
};

} // namespace

// ---------------------------------------------------------------------------

bool Bmp2msxApp::OnInit() {
	wxInitAllImageHandlers();
	m_exe_dir = ExeDir();

	// Calibration data first, exactly as the Windows startup and the
	// CLI do: the copy beside the executable wins, then the current
	// directory (GetCustomFile falls back to the factory tables when
	// the file is absent).
	std::string custom = m_exe_dir + "/BMP2MSX_CUSTOM.BIN";
	if( wxFileExists( wxString::FromUTF8( custom ) ) ) {
		GetCustomFile( custom.c_str() );
	} else {
		GetCustomFile( "BMP2MSX_CUSTOM.BIN" );
	}

	// Default settings: BMP2MSX.B2M beside the executable when present,
	// else the application's own defaults.
	std::string b2m = m_exe_dir + "/BMP2MSX.B2M";
	if( wxFileExists( wxString::FromUTF8( b2m ) ) ) {
		GetCfgFile( &m_default, b2m.c_str() );
	} else if( wxFileExists( "BMP2MSX.B2M" ) ) {
		GetCfgFile( &m_default, "BMP2MSX.B2M" );
	} else {
		GetDefCfg( &m_default );
	}

	// ---- test hooks -------------------------------------------------
	if( argc >= 4 && wxString( argv[ 1 ] ) == "--smoke-convert" ) {
		wxString input = argv[ 2 ], outdir = argv[ 3 ];
		SETTING set;
		GetDefCfg( &set );
		set.screen_mode = MD_SC8P;
		set.output_basic_enable = true;
		set.PreView = 3;
		GuiPreview pv;
		std::string err;
		if( !gui_convert_image( set, input.utf8_str().data(), pv, &err ) ) {
			fprintf( stderr, "SMOKE_FAIL convert: %s\n", err.c_str() );
			return false;
		}
		wxFileName in_fn( input );
		wxString name_source = outdir + "/" + in_fn.GetName() + ".bmp";
		std::string pal = gui_derived_palette_name( pv.mode, name_source.utf8_str().data() );
		int ecode = gui_save_preview_files( pv, pv.mode, name_source.utf8_str().data(), pal );
		if( ecode != 0 ) {
			fprintf( stderr, "SMOKE_FAIL save: ecode=%d\n", ecode );
			return false;
		}
		printf( "SMOKE_OK\n" );
		return false;
	}

	m_frame = new MainFrame();

	if( argc >= 4 && wxString( argv[ 1 ] ) == "--snapshot" ) {
		m_frame->Show( true );
		m_frame->ConvertFile( argv[ 2 ] );
		auto* timer = new wxTimer();
		timer->Bind( wxEVT_TIMER, [ this, timer ]( wxTimerEvent& ) {
			timer->Stop();
			bool ok = CaptureWindow( m_frame, argv[ 3 ] );
			fprintf( stderr, "snapshot %s\n", ok ? "OK" : " FAILED" );
			m_frame->Close();
		} );
		timer->StartOnce( 1500 );
		return true;
	}
	if( argc >= 3 && wxString( argv[ 1 ] ) == "--snapshot-settings" ) {
		m_frame->Show( true );
		m_frame->ShowSettingsDialogShot();
		auto* timer = new wxTimer();
		timer->Bind( wxEVT_TIMER, [ this, timer ]( wxTimerEvent& ) {
			timer->Stop();
			// capture the whole virtual screen region of the dialog:
			// find the top-level window that is not the main frame.
			for( wxWindow* w : wxTopLevelWindows ) {
				if( w != m_frame && w->IsShown() ) {
					bool ok = CaptureWindow( (wxTopLevelWindow*) w, argv[ 2 ] );
					fprintf( stderr, "snapshot %s\n", ok ? "OK" : " FAILED" );
					break;
				}
			}
			m_frame->Close();
		} );
		timer->StartOnce( 1500 );
		return true;
	}

	m_frame->Show( true );
	return true;
}

void Bmp2msxApp::SaveDefaults() {
	std::string b2m = m_exe_dir + "/BMP2MSX.B2M";
	SaveCfgFile( &m_default, b2m.c_str() );
}

int Bmp2msxApp::OnExit() {
	return wxApp::OnExit();
}

wxIMPLEMENT_APP( Bmp2msxApp );

// ---------------------------------------------------------------------------

MainFrame::MainFrame()
	: wxFrame( nullptr, wxID_ANY, "BMP2MSX", wxDefaultPosition, wxSize( 860, 760 ) ) {
	m_last = wxGetApp().DefaultSetting();

	auto* menu_bar = new wxMenuBar();
	{
		auto* file = new wxMenu();
		file->Append( wxID_OPEN, "Open image..." );
		file->Append( ID_SCRIPT_MODE, "Script mode" );
		file->AppendSeparator();
		file->Append( wxID_EXIT, "Exit" );
		menu_bar->Append( file, "File" );

		auto* edit = new wxMenu();
		edit->Append( ID_CLIPBOARD, "Convert clipboard image\tCtrl+V" );
		menu_bar->Append( edit, "Edit" );

		auto* prev = new wxMenu();
		// (Arrow keys / Space are handled by the frame's key hook;
		// wxGTK cannot register them as menu accelerators.)
		prev->Append( ID_PREV_NEXT, "Next image" );
		prev->Append( ID_PREV_PREV, "Previous image" );
		prev->Append( ID_PREV_TOGGLE, "Toggle source / result" );
		prev->Append( ID_PREV_ORIGINAL, "Show original image\tCtrl+O" );
		prev->Append( wxID_ZOOM_IN, "Zoom in\tF7" );
		prev->Append( wxID_ZOOM_OUT, "Zoom out\tF8" );
		prev->AppendSeparator();
		prev->Append( ID_PREV_CLEAR, "Clear preview\tDel" );
		prev->Append( ID_PREV_CLEAR_ALL, "Clear all previews\tCtrl+A" );
		prev->AppendSeparator();
		prev->Append( wxID_SAVE, "Save" );
		prev->Append( ID_PREV_SAVE_AS, "Save as...\tCtrl+S" );
		prev->Append( ID_PREV_SAVE_BMP, "Save as BMP..." );
		prev->Append( ID_PREV_RECONVERT, "Re-convert...\tCtrl+R" );
		menu_bar->Append( prev, "Preview" );

		auto* settings = new wxMenu();
		settings->Append( ID_SET_DEFAULTS, "Settings...\tCtrl+E" );
		m_show_text_item = settings->AppendCheckItem( ID_SET_SHOW_TEXT, "Show settings text" );
		m_show_text_item->Check( true );
		settings->Append( ID_SET_RESET, "Reset settings to defaults" );
		settings->Append( ID_SET_CALIBRATION, "Color calibration..." );
		menu_bar->Append( settings, "Settings" );

		auto* ver = new wxMenu();
		ver->Append( ID_VERSION, "Version..." );
		// The Windows GUI (after Phase 1) has "Version..." as a
		// standalone item; wxMenuBar has no bare items, so it gets its
		// own one-entry menu.
		menu_bar->Append( ver, "Version" );
	}
	SetMenuBar( menu_bar );
	Bind( wxEVT_MENU, &MainFrame::OnMenu, this );

	m_scroll = new wxScrolledWindow( this );
	m_scroll->SetScrollRate( 16, 16 );
	m_canvas = new wxPanel( m_scroll );
	m_canvas->SetBackgroundStyle( wxBG_STYLE_PAINT );
	m_canvas->Bind( wxEVT_PAINT, &MainFrame::OnCanvasPaint, this );
	m_canvas->Bind( wxEVT_LEFT_DOWN, &MainFrame::OnCanvasClick, this );
	auto* scroll_sizer = new wxBoxSizer( wxVERTICAL );
	scroll_sizer->Add( m_canvas, 1, wxEXPAND );
	m_scroll->SetSizer( scroll_sizer );
	m_canvas->SetSize( PREVIEW_W, PREVIEW_H );
	m_scroll->SetVirtualSize( PREVIEW_W, PREVIEW_H );

	m_summary_panel = new wxPanel( this );
	m_summary_panel->SetBackgroundColour( wxColour( 0, 0, 200 ) );
	m_summary = new wxStaticText( m_summary_panel, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
	m_summary->SetForegroundColour( *wxWHITE );
	auto* sum_sizer = new wxBoxSizer( wxVERTICAL );
	sum_sizer->Add( m_summary, 1, wxEXPAND | wxALL, 6 );
	m_summary_panel->SetSizer( sum_sizer );
	m_summary_panel->SetMinSize( wxSize( -1, 96 ) );

	auto* top = new wxBoxSizer( wxVERTICAL );
	top->Add( m_scroll, 1, wxEXPAND );
	top->Add( m_summary_panel, 0, wxEXPAND );
	SetSizer( top );

	CreateStatusBar( 2 );
	SetDropTarget( new FileDropTarget( this ) );
	Bind( wxEVT_CHAR_HOOK, &MainFrame::OnCharHook, this );
	Bind( wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this );
	UpdateView();
}

SETTING MainFrame::PickSettingForNewConversion() const {
	const SETTING& def = wxGetApp().DefaultSetting();
	switch( def.CnvMode ) {
	case CM_LAST:
		return m_last;
	case CM_IMAGE:
		if( m_current >= 0 ) return m_previews[ (size_t) m_current ].mode;
		return def;
	default:
		return def;
	}
}

bool MainFrame::ConvertFile( const wxString& path ) {
	return ConvertWith( path, PickSettingForNewConversion() );
}

bool MainFrame::ConvertWith( const wxString& path, const SETTING& setting ) {
	wxBusyCursor busy;
	GuiPreview pv;
	std::string err;
	if( !gui_convert_image( setting, path.utf8_str().data(), pv, &err ) ) {
		wxMessageBox( wxString::Format( "Conversion failed:\n%s",
										wxString::FromUTF8( err ) ),
					  "BMP2MSX", wxOK | wxICON_WARNING, this );
		return false;
	}
	m_last = pv.mode;
	if( (int) m_previews.size() >= 50 ) {
		m_previews.erase( m_previews.begin() );	// keep at most 50, like the original
	}
	m_previews.push_back( std::move( pv ) );
	m_current = (int) m_previews.size() - 1;
	m_show_source = false;
	m_cache_index = -2;
	UpdateView();
	if( m_previews[ (size_t) m_current ].mode.PreView != 2 ) {
		AfterConvertSave( m_previews[ (size_t) m_current ] );
	}
	return true;
}

void MainFrame::AfterConvertSave( const GuiPreview& pv ) {
	std::string name_source = pv.input_path;
	std::string pal = gui_derived_palette_name( pv.mode, name_source );
	int ecode = gui_save_preview_files( pv, pv.mode, name_source, pal );
	if( ecode != 0 ) {
		wxMessageBox( "Failed to write the output files.", "BMP2MSX",
					  wxOK | wxICON_WARNING, this );
	}
}

void MainFrame::SaveCurrent( bool save_as ) {
	if( m_current < 0 ) return;
	const GuiPreview& pv = m_previews[ (size_t) m_current ];
	SETTING mode = pv.mode;
	wxString name_source = wxString::FromUTF8( pv.input_path );
	if( save_as || !mode.AutoName ) {
		wxFileDialog dlg( this, "Save preview", "", name_source,
						  "All files|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
		if( dlg.ShowModal() != wxID_OK ) return;
		name_source = dlg.GetPath();
		mode.AutoName = false;
	}
	std::string pal;
	if( mode.PltMode == PLT_PL || mode.PltMode == PLT_CSV ) {
		if( mode.AutoName && !save_as ) {
			pal = gui_derived_palette_name( mode, name_source.utf8_str().data() );
		} else {
			wxString def = wxString::FromUTF8(
				gui_derived_palette_name( mode, name_source.utf8_str().data() ) );
			wxFileDialog dlg( this, "Save palette file", "", def,
							  "All files|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
			if( dlg.ShowModal() == wxID_OK ) {
				pal = dlg.GetPath().utf8_str().data();
			}	// cancelled: pal stays "", the palette file is skipped
		}
	}
	int ecode = gui_save_preview_files( pv, mode, name_source.utf8_str().data(), pal );
	if( ecode != 0 ) {
		wxMessageBox( "Failed to write the output files.", "BMP2MSX",
					  wxOK | wxICON_WARNING, this );
	}
}

void MainFrame::SaveCurrentAsBmp() {
	if( m_current < 0 ) return;
	const GuiPreview& pv = m_previews[ (size_t) m_current ];
	wxFileName fn( wxString::FromUTF8( pv.input_path ) );
	wxFileDialog dlg( this, "Save preview as BMP", fn.GetPath(),
					  fn.GetName() + "_preview.bmp",
					  "BMP files (*.bmp)|*.bmp|All files|*.*",
					  wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	if( dlg.ShowModal() != wxID_OK ) return;
	// The Windows BMP save renders through draw_screen()+GDI; here the
	// portable renderer produces the same pixels and they are written
	// as a plain 24bpp BMP.
	std::vector<unsigned char> rgb( PREVIEW_W * PREVIEW_H * 3 );
	render_converted_preview( pv.vram.data(), &pv.mode, rgb.data() );
	wxImage img( PREVIEW_W, PREVIEW_H );
	memcpy( img.GetData(), rgb.data(), rgb.size() );
	if( !img.SaveFile( dlg.GetPath(), wxBITMAP_TYPE_BMP ) ) {
		wxMessageBox( "Failed to write the BMP file.", "BMP2MSX",
					  wxOK | wxICON_WARNING, this );
	}
}

void MainFrame::ShowOriginalWindow() {
	if( m_current < 0 ) return;
	const GuiPreview& pv = m_previews[ (size_t) m_current ];
	auto* frame = new OriginalFrame( this,
		wxString::Format( "Original image - %s",
						  wxString::FromUTF8( pv.input_path ) ),
		pv.original, pv.orig_w, pv.orig_h );
	frame->Show( true );
}

void MainFrame::OpenDefaultSettings() {
	SettingsDialog dlg( this, wxGetApp().DefaultSetting(),
						SettingsDialog::Purpose::DefaultSettings );
	if( dlg.ShowModal() == wxID_OK ) {
		wxGetApp().SetDefaultSetting( dlg.Edited() );
		UpdateSummary();
	}
}

void MainFrame::OpenReconvert() {
	if( m_current < 0 ) return;
	const GuiPreview& cur = m_previews[ (size_t) m_current ];
	wxString input = wxString::FromUTF8( cur.input_path );
	SettingsDialog dlg( this, cur.mode, SettingsDialog::Purpose::Reconvert,
		[ this ]( const SETTING& s ) { wxGetApp().SetDefaultSetting( s ); } );
	if( dlg.ShowModal() != wxID_OK ) return;
	SETTING next = dlg.Edited();
	if( next.bDefault ) {
		wxGetApp().SetDefaultSetting( next );
	}
	ConvertWith( input, next );
}

void MainFrame::ShowSettingsDialogShot() {
	// Snapshot hook: show the settings dialog modeless so it can be
	// captured; it is destroyed with the application.
	auto* dlg = new SettingsDialog( this, wxGetApp().DefaultSetting(),
									SettingsDialog::Purpose::DefaultSettings );
	dlg->Show( true );
}

wxImage MainFrame::CurrentDisplayImage() {
	if( m_current < 0 ) return wxImage();
	if( m_cache_index == m_current && m_cache_source == m_show_source ) {
		return m_cache_img;
	}
	const GuiPreview& pv = m_previews[ (size_t) m_current ];
	wxImage img;
	if( m_show_source ) {
		std::vector<unsigned char> rgb( (size_t) pv.source_w * pv.source_h * 3 );
		render_color_buffer( pv.source.data(), pv.source_w, pv.source_h, rgb.data() );
		img.Create( pv.source_w, pv.source_h );
		memcpy( img.GetData(), rgb.data(), rgb.size() );
		img = img.Rescale( PREVIEW_W, PREVIEW_H, wxIMAGE_QUALITY_NEAREST );
	} else {
		std::vector<unsigned char> rgb( PREVIEW_W * PREVIEW_H * 3 );
		render_converted_preview( pv.vram.data(), &pv.mode, rgb.data() );
		img.Create( PREVIEW_W, PREVIEW_H );
		memcpy( img.GetData(), rgb.data(), rgb.size() );
	}
	m_cache_img = img;
	m_cache_index = m_current;
	m_cache_source = m_show_source;
	return img;
}

void MainFrame::UpdateView() {
	m_canvas->SetSize( PREVIEW_W * m_zoom, PREVIEW_H * m_zoom );
	m_scroll->SetVirtualSize( PREVIEW_W * m_zoom, PREVIEW_H * m_zoom );
	m_canvas->Refresh();
	UpdateSummary();

	bool have = m_current >= 0;
	const int ids[] = { ID_PREV_NEXT, ID_PREV_PREV, ID_PREV_TOGGLE, ID_PREV_ORIGINAL,
						ID_PREV_CLEAR, ID_PREV_CLEAR_ALL, ID_PREV_SAVE_AS,
						ID_PREV_SAVE_BMP, ID_PREV_RECONVERT, wxID_SAVE, wxID_ZOOM_IN,
						wxID_ZOOM_OUT };
	for( int id : ids ) {
		wxMenuItem* item = GetMenuBar()->FindItem( id );
		if( item ) item->Enable( have );
	}
	if( have ) {
		const GuiPreview& pv = m_previews[ (size_t) m_current ];
		SetStatusText( wxString::Format( "Preview %d / %zu  zoom x%d",
			m_current + 1, m_previews.size(), m_zoom ), 0 );
		SetStatusText( wxString::FromUTF8( pv.input_path ), 1 );
		SetTitle( wxString::Format( "BMP2MSX - %s",
			wxFileName( wxString::FromUTF8( pv.input_path ) ).GetFullName() ) );
	} else {
		SetStatusText( "No preview", 0 );
		SetStatusText( "", 1 );
		SetTitle( "BMP2MSX" );
	}
}

void MainFrame::UpdateSummary() {
	if( m_current < 0 ) {
		m_summary->SetLabel( "Drop an image file here, or use File > Open image." );
		m_summary_panel->SetBackgroundColour( wxColour( 0, 0, 200 ) );
	} else {
		const GuiPreview& pv = m_previews[ (size_t) m_current ];
		m_summary->SetLabel( wxString::Format( "%s\n%s",
			wxString::FromUTF8( pv.input_path ),
			wxString::FromUTF8( cli_describe_setting( pv.mode ) ) ) );
		m_summary->Wrap( GetSize().x - 24 );
		// blue: converted result, red: source image (manual 4.2)
		m_summary_panel->SetBackgroundColour(
			m_show_source ? wxColour( 200, 0, 0 ) : wxColour( 0, 0, 200 ) );
	}
	m_summary_panel->Refresh();
}

void MainFrame::OnCanvasPaint( wxPaintEvent& ) {
	wxPaintDC dc( m_canvas );
	dc.SetBackground( wxBrush( wxColour( 32, 32, 32 ) ) );
	dc.Clear();
	if( m_current < 0 ) {
		dc.SetTextForeground( *wxWHITE );
		dc.SetFont( wxFont( 14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
							wxFONTWEIGHT_BOLD ) );
		dc.DrawText( "BMP2MSX", 24, 24 );
		dc.SetFont( wxFont( 11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
							wxFONTWEIGHT_NORMAL ) );
		dc.DrawText( "Drop an image file here.", 24, 56 );
		return;
	}
	wxImage img = CurrentDisplayImage();
	if( !img.IsOk() ) return;
	if( m_zoom != 1 ) {
		img = img.Rescale( PREVIEW_W * m_zoom, PREVIEW_H * m_zoom,
						   wxIMAGE_QUALITY_NEAREST );
	}
	dc.DrawBitmap( wxBitmap( img ), 0, 0 );
}

void MainFrame::OnCanvasClick( wxMouseEvent& ) {
	if( m_current < 0 ) {
		OpenDefaultSettings();
	} else {
		OpenReconvert();
	}
}

void MainFrame::OnCharHook( wxKeyEvent& e ) {
	bool have = m_current >= 0;
	int key = e.GetKeyCode();
	if( have && ( key == WXK_RIGHT || key == WXK_DOWN ) ) {
		m_current = ( m_current + 1 ) % (int) m_previews.size();
		m_show_source = false;
		m_cache_index = -2;
		UpdateView();
		return;
	}
	if( have && ( key == WXK_LEFT || key == WXK_UP ) ) {
		m_current = ( m_current + (int) m_previews.size() - 1 ) % (int) m_previews.size();
		m_show_source = false;
		m_cache_index = -2;
		UpdateView();
		return;
	}
	if( have && key == ' ' && !e.HasModifiers() ) {
		m_show_source = !m_show_source;
		UpdateView();
		return;
	}
	if( have && key == WXK_F7 ) {
		if( m_zoom < 4 ) { ++m_zoom; UpdateView(); }
		return;
	}
	if( have && key == WXK_F8 ) {
		if( m_zoom > 1 ) { --m_zoom; UpdateView(); }
		return;
	}
	if( have && key == WXK_DELETE ) {
		m_previews.erase( m_previews.begin() + m_current );
		if( m_current >= (int) m_previews.size() ) m_current = (int) m_previews.size() - 1;
		m_show_source = false;
		m_cache_index = -2;
		UpdateView();
		return;
	}
	e.Skip();
}

void MainFrame::OnMenu( wxCommandEvent& e ) {
	switch( e.GetId() ) {
	case wxID_OPEN: {
		wxFileDialog dlg( this, "Open image", "", "",
			"Image files (*.bmp;*.png;*.jpg;*.jpeg)|*.bmp;*.png;*.jpg;*.jpeg|All files|*.*",
			wxFD_OPEN );
		if( dlg.ShowModal() == wxID_OK ) ConvertFile( dlg.GetPath() );
		break;
	}
	case wxID_EXIT:
		Close();
		break;
	case ID_SCRIPT_MODE:
		if( !m_script_frame ) {
			m_script_frame = new ScriptFrame( this,
				[ this ]() { return wxGetApp().DefaultSetting(); } );
			m_script_frame->Bind( wxEVT_CLOSE_WINDOW,
				[ this ]( wxCloseEvent& ev ) {
					m_script_frame = nullptr;
					ev.Skip();
				} );
		}
		if( m_current >= 0 ) {
			m_script_frame->SetTarget(
				wxString::FromUTF8( m_previews[ (size_t) m_current ].input_path ) );
		}
		m_script_frame->Show( true );
		m_script_frame->Raise();
		break;
	case ID_CLIPBOARD: {
		bool done = false;
		if( wxTheClipboard->Open() ) {
			wxBitmapDataObject data;
			if( wxTheClipboard->GetData( data ) ) {
				wxImage img = data.GetBitmap().ConvertToImage();
				wxString tmp = wxFileName::CreateTempFileName( "bmp2msx_clip" ) + ".bmp";
				if( img.IsOk() && img.SaveFile( tmp, wxBITMAP_TYPE_BMP ) ) {
					done = ConvertFile( tmp );
				}
			}
			wxTheClipboard->Close();
		}
		if( !done ) {
			wxMessageBox( "No image on the clipboard.", "BMP2MSX",
						  wxOK | wxICON_INFORMATION, this );
		}
		break;
	}
	case ID_PREV_NEXT:
	case ID_PREV_PREV: {
		if( m_current < 0 ) break;
		int delta = ( e.GetId() == ID_PREV_NEXT ) ? 1 : -1;
		m_current = ( m_current + delta + (int) m_previews.size() ) % (int) m_previews.size();
		m_show_source = false;
		m_cache_index = -2;
		UpdateView();
		break;
	}
	case ID_PREV_TOGGLE:
		if( m_current >= 0 ) { m_show_source = !m_show_source; UpdateView(); }
		break;
	case ID_PREV_ORIGINAL:
		ShowOriginalWindow();
		break;
	case wxID_ZOOM_IN:
		if( m_zoom < 4 ) { ++m_zoom; UpdateView(); }
		break;
	case wxID_ZOOM_OUT:
		if( m_zoom > 1 ) { --m_zoom; UpdateView(); }
		break;
	case ID_PREV_CLEAR:
		if( m_current >= 0 ) {
			m_previews.erase( m_previews.begin() + m_current );
			if( m_current >= (int) m_previews.size() ) m_current = (int) m_previews.size() - 1;
			m_show_source = false;
			m_cache_index = -2;
			UpdateView();
		}
		break;
	case ID_PREV_CLEAR_ALL:
		m_previews.clear();
		m_current = -1;
		m_show_source = false;
		m_cache_index = -2;
		UpdateView();
		break;
	case wxID_SAVE:
		SaveCurrent( false );
		break;
	case ID_PREV_SAVE_AS:
		SaveCurrent( true );
		break;
	case ID_PREV_SAVE_BMP:
		SaveCurrentAsBmp();
		break;
	case ID_PREV_RECONVERT:
		OpenReconvert();
		break;
	case ID_SET_DEFAULTS:
		OpenDefaultSettings();
		break;
	case ID_SET_SHOW_TEXT: {
		bool show = m_show_text_item->IsChecked();
		m_summary_panel->Show( show );
		Layout();
		break;
	}
	case ID_SET_RESET:
		if( wxMessageBox( "Reset the default settings to the factory defaults?",
						  "BMP2MSX", wxYES_NO | wxICON_QUESTION, this ) == wxYES ) {
			SETTING def;
			GetDefCfg( &def );
			wxGetApp().SetDefaultSetting( def );
			m_last = def;
			UpdateSummary();
		}
		break;
	case ID_SET_CALIBRATION:
		wxMessageBox(
			"The color calibration editor is not part of this build yet.\n"
			"Calibration data (BMP2MSX_CUSTOM.BIN) is still honored at startup,\n"
			"exactly as in the CLI build.",
			"BMP2MSX", wxOK | wxICON_INFORMATION, this );
		break;
	case ID_VERSION:
		wxMessageBox( wxString::Format(
			"BMP2MSX - BMP to MSX image converter\n%s\n\n"
			"wxWidgets front end (Linux build)", CFG_VERSION ),
			"Version", wxOK | wxICON_INFORMATION, this );
		break;
	}
}

void MainFrame::OnClose( wxCloseEvent& ) {
	if( m_script_frame ) {
		m_script_frame->Destroy();
		m_script_frame = nullptr;
	}
	wxGetApp().SaveDefaults();
	Destroy();
}
