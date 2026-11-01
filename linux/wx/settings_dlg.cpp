// See settings_dlg.h.
#include "settings_dlg.h"

#include <wx/clrpicker.h>
#include <wx/filename.h>
#include <wx/spinctrl.h>
#include <cstdio>
#include <cstring>

#include "utils.h"
#include "bsave.h"

namespace {

const char* const kModeNames[ MD_MAX ] = {
	"SCREEN2", "SCREEN3", "SCREEN5", "SCREEN6", "SCREEN7", "SCREEN8",
	"SCREEN10", "SCREEN12", "SCREEN5 (256 lines)", "SCREEN6 (256 lines)",
	"SCREEN7 (256 lines)", "SCREEN8 (256 lines)", "SCREEN10 (256 lines)",
	"SCREEN12 (256 lines)", "SCREEN8 EPAL (V9968)",
	"SCREEN8 EPAL (V9968) (256 lines)"
};

bool IsPaletteMode( uint32_t m ) {
	return m < MD_SC8 || ( m >= MD_SC5_256L && m < MD_SC8_256L );
}
bool IsNaturalMode( uint32_t m ) {
	return m == MD_SC10 || m == MD_SC12 || m == MD_SC10_256L || m == MD_SC12_256L;
}
bool IsTileMode( uint32_t m ) {
	switch( m ) {
	case MD_SC2: case MD_SC3: case MD_SC5: case MD_SC6: case MD_SC7:
	case MD_SC5_256L: case MD_SC6_256L: case MD_SC7_256L: return true;
	}
	return false;
}
bool IsEpalMode( uint32_t m ) {
	return m == MD_SC8P || m == MD_SC8P_256L;
}

wxChoice* MakeChoice( wxWindow* parent, const char* const* items, int count ) {
	wxArrayString s;
	for( int i = 0; i < count; ++i ) s.Add( wxString::FromUTF8( items[ i ] ) );
	return new wxChoice( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, s );
}

wxSlider* MakeSlider( wxWindow* parent, int value, int min, int max, int width = 150 ) {
	return new wxSlider( parent, wxID_ANY, value, min, max,
						 wxDefaultPosition, wxSize( width, -1 ) );
}

// Add "label [slider] value" row to a sizer; returns the value label.
wxStaticText* AddSliderRow( wxSizer* sizer, wxWindow* parent, const char* label,
							wxSlider* slider ) {
	auto* row = new wxBoxSizer( wxHORIZONTAL );
	row->Add( new wxStaticText( parent, wxID_ANY, wxString::FromUTF8( label ) ),
			  0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
	row->Add( slider, 1, wxALIGN_CENTER_VERTICAL );
	auto* val = new wxStaticText( parent, wxID_ANY, "0" );
	row->Add( val, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6 );
	sizer->Add( row, 0, wxEXPAND | wxALL, 2 );
	return val;
}

} // namespace

// ---------------------------------------------------------------------------
// Palette swatch panel: 16 entries in an 8x2 grid.  Left click selects,
// left drag swaps entries (colors and use-permission together), right
// click cycles the use-permission state -- the same gestures as the
// Windows dialog (manual 5.5).  Markers: a red cross means "do not use",
// a white dot means "use set color", nothing means "auto".
// ---------------------------------------------------------------------------
class PalettePanel : public wxPanel {
public:
	PalettePanel( wxWindow* parent, SETTING* mode, int* sel,
				  std::function<void()> on_change )
		: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 8 * kCell + 8, 2 * kCell + 8 ),
				   wxBORDER_SUNKEN ),
		  m_mode( mode ), m_sel( sel ), m_on_change( std::move( on_change ) ) {
		SetBackgroundStyle( wxBG_STYLE_PAINT );
		Bind( wxEVT_PAINT, &PalettePanel::OnPaint, this );
		Bind( wxEVT_LEFT_DOWN, &PalettePanel::OnLeftDown, this );
		Bind( wxEVT_LEFT_UP, &PalettePanel::OnLeftUp, this );
		Bind( wxEVT_RIGHT_DOWN, &PalettePanel::OnRightDown, this );
	}

private:
	static const int kCell = 32;

	int Hit( const wxPoint& p ) const {
		int x = ( p.x - 4 ) / kCell, y = ( p.y - 4 ) / kCell;
		if( x < 0 || x >= 8 || y < 0 || y >= 2 ) return -1;
		if( ( p.x - 4 ) % kCell >= kCell - 2 || ( p.y - 4 ) % kCell >= kCell - 2 ) return -1;
		return y * 8 + x;
	}

	void OnPaint( wxPaintEvent& ) {
		wxPaintDC dc( this );
		for( int i = 0; i < 16; ++i ) {
			int x = 4 + ( i % 8 ) * kCell, y = 4 + ( i / 8 ) * kCell;
			const C_PALETTE& p = m_mode->color_palette[ i ];
			dc.SetBrush( wxBrush( wxColour( convert7to255_r[ p.red ],
											convert7to255_g[ p.green ],
											convert7to255_b[ p.blue ] ) ) );
			dc.SetPen( *wxBLACK_PEN );
			dc.DrawRectangle( x, y, kCell - 2, kCell - 2 );
			if( m_mode->PalEn[ i ] == PALEN_NONE ) {
				dc.SetPen( wxPen( *wxRED, 2 ) );
				dc.DrawLine( x + 3, y + 3, x + kCell - 5, y + kCell - 5 );
				dc.DrawLine( x + kCell - 5, y + 3, x + 3, y + kCell - 5 );
			} else if( m_mode->PalEn[ i ] == PALEN_USE ) {
				dc.SetBrush( *wxWHITE_BRUSH );
				dc.SetPen( *wxBLACK_PEN );
				dc.DrawCircle( x + kCell - 9, y + kCell - 9, 4 );
			}
			if( i == *m_sel ) {
				dc.SetBrush( wxNullBrush );
				dc.SetPen( wxPen( wxColour( 255, 255, 0 ), 3 ) );
				dc.DrawRectangle( x - 1, y - 1, kCell, kCell );
			}
		}
	}

	void OnLeftDown( wxMouseEvent& e ) {
		int idx = Hit( e.GetPosition() );
		if( idx < 0 ) return;
		m_drag_from = idx;
		*m_sel = idx;
		if( m_on_change ) m_on_change();
		Refresh();
	}

	void OnLeftUp( wxMouseEvent& e ) {
		int idx = Hit( e.GetPosition() );
		if( m_drag_from >= 0 && idx >= 0 && idx != m_drag_from ) {
			// swap (colors and use-permission state together)
			C_PALETTE tc = m_mode->color_palette[ m_drag_from ];
			m_mode->color_palette[ m_drag_from ] = m_mode->color_palette[ idx ];
			m_mode->color_palette[ idx ] = tc;
			uint8_t te = m_mode->PalEn[ m_drag_from ];
			m_mode->PalEn[ m_drag_from ] = m_mode->PalEn[ idx ];
			m_mode->PalEn[ idx ] = te;
			*m_sel = idx;
			if( m_on_change ) m_on_change();
			Refresh();
		}
		m_drag_from = -1;
	}

	void OnRightDown( wxMouseEvent& e ) {
		int idx = Hit( e.GetPosition() );
		if( idx < 0 ) return;
		*m_sel = idx;
		m_mode->PalEn[ idx ] = (uint8_t) ( ( m_mode->PalEn[ idx ] + 1 ) % PALEN_MAX );
		if( m_on_change ) m_on_change();
		Refresh();
	}

	SETTING* m_mode;
	int* m_sel;
	std::function<void()> m_on_change;
	int m_drag_from = -1;
};

// ---------------------------------------------------------------------------

SettingsDialog::SettingsDialog( wxWindow* parent, const SETTING& initial,
								Purpose purpose,
								std::function<void( const SETTING& )> copy_to_defaults )
	: wxDialog( parent, wxID_ANY,
				purpose == Purpose::Reconvert ? "Re-convert settings" : "Default settings",
				wxDefaultPosition, wxDefaultSize,
				wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER ),
	  m_edit( initial ), m_copy_to_defaults( std::move( copy_to_defaults ) ) {
	BuildUi( purpose );
	LoadAll();
	UpdateEnabled();
}

void SettingsDialog::BuildUi( Purpose purpose ) {
	auto* top = new wxBoxSizer( wxVERTICAL );

	// ---- top button bar (same arrangement as the Windows dialog)
	auto* bar = new wxBoxSizer( wxHORIZONTAL );
	auto* save_file = new wxButton( this, wxID_ANY, "Save to file..." );
	auto* load_file = new wxButton( this, wxID_ANY, "Load from file..." );
	save_file->Bind( wxEVT_BUTTON, &SettingsDialog::OnSaveFile, this );
	load_file->Bind( wxEVT_BUTTON, &SettingsDialog::OnLoadFile, this );
	bar->Add( save_file, 0, wxALL, 3 );
	bar->Add( load_file, 0, wxALL, 3 );
	bar->AddStretchSpacer();
	if( purpose == Purpose::Reconvert && m_copy_to_defaults ) {
		auto* copy_btn = new wxButton( this, wxID_ANY, "Copy to defaults" );
		copy_btn->Bind( wxEVT_BUTTON, &SettingsDialog::OnCopyToDefaults, this );
		bar->Add( copy_btn, 0, wxALL, 3 );
	}
	auto* ok = new wxButton( this, wxID_OK,
		purpose == Purpose::Reconvert ? "Re-convert" : "Set as defaults" );
	ok->Bind( wxEVT_BUTTON, &SettingsDialog::OnOk, this );
	auto* cancel = new wxButton( this, wxID_CANCEL, "Cancel" );
	bar->Add( ok, 0, wxALL, 3 );
	bar->Add( cancel, 0, wxALL, 3 );
	top->Add( bar, 0, wxEXPAND );

	auto* cols = new wxBoxSizer( wxHORIZONTAL );
	auto* left = new wxBoxSizer( wxVERTICAL );
	auto* right = new wxBoxSizer( wxVERTICAL );

	// ---- output group
	{
		auto* box = new wxStaticBoxSizer( wxVERTICAL, this, "Output" );
		auto* row = new wxBoxSizer( wxHORIZONTAL );
		row->Add( new wxStaticText( this, wxID_ANY, "MSX screen mode:" ), 0,
				  wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
		m_mode = MakeChoice( this, kModeNames, MD_MAX );
		m_mode->Bind( wxEVT_CHOICE, &SettingsDialog::OnModeChanged, this );
		row->Add( m_mode, 1 );
		box->Add( row, 0, wxEXPAND | wxALL, 3 );

		const char* const targets[] = { "File only", "Preview only", "Preview + file" };
		wxArrayString t;
		for( auto s : targets ) t.Add( s );
		m_preview = new wxRadioBox( this, wxID_ANY, "Output target", wxDefaultPosition,
									wxDefaultSize, t, 1 );
		box->Add( m_preview, 0, wxEXPAND | wxALL, 3 );

		m_b192 = new wxCheckBox( this, wxID_ANY, "192 lines" );
		m_interlace = new wxCheckBox( this, wxID_ANY, "Interlace" );
		m_autoname = new wxCheckBox( this, wxID_ANY, "Automatic file names" );
		m_basic = new wxCheckBox( this, wxID_ANY, "BAS loader output" );
		auto* checks = new wxBoxSizer( wxHORIZONTAL );
		checks->Add( m_b192, 0, wxALL, 3 );
		checks->Add( m_interlace, 0, wxALL, 3 );
		checks->Add( m_autoname, 0, wxALL, 3 );
		checks->Add( m_basic, 0, wxALL, 3 );
		box->Add( checks, 0, wxALL, 2 );

		const char* const cnv[] = { "Default settings", "Last conversion", "Displayed image" };
		wxArrayString c;
		for( auto s : cnv ) c.Add( s );
		m_cnv_mode = new wxRadioBox( this, wxID_ANY, "Settings used for conversion",
									 wxDefaultPosition, wxDefaultSize, c, 1 );
		box->Add( m_cnv_mode, 0, wxEXPAND | wxALL, 3 );

		m_bdefault = new wxCheckBox( this, wxID_ANY,
			"Always make re-converted settings the defaults" );
		box->Add( m_bdefault, 0, wxALL, 3 );
		left->Add( box, 0, wxEXPAND | wxALL, 4 );
	}

	// ---- conversion group
	{
		auto* box = new wxStaticBoxSizer( wxVERTICAL, this, "Conversion" );
		auto AddChoiceRow = [ & ]( const char* label, wxChoice*& choice,
								   const char* const* items, int count ) {
			auto* row = new wxBoxSizer( wxHORIZONTAL );
			row->Add( new wxStaticText( this, wxID_ANY, wxString::FromUTF8( label ) ), 0,
					  wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
			choice = MakeChoice( this, items, count );
			row->Add( choice, 1 );
			box->Add( row, 0, wxEXPAND | wxALL, 2 );
		};
		const char* const dither[] = { "None", "Pseudo-random", "Fine mesh",
			"Vertical mesh", "Brick", "Grainy", "Coarse mesh", "Light mesh", "Halftone" };
		AddChoiceRow( "Dither pattern:", m_dither, dither, 9 );
		const char* const dadd[] = { "Rotate", "Same phase", "Shift X", "Shift Y" };
		AddChoiceRow( "Dither add:", m_dither_add, dadd, 4 );
		const char* const acc[] = { "8 bit (no reduction)", "7 bit", "6 bit", "5 bit", "4 bit" };
		AddChoiceRow( "Color accuracy:", m_accuracy, acc, 5 );

		m_tile = new wxCheckBox( this, wxID_ANY, "Use 50:50 tiles" );
		box->Add( m_tile, 0, wxALL, 3 );

		m_diffusion = new wxCheckBox( this, wxID_ANY, "Error diffusion" );
		m_diffusion->Bind( wxEVT_CHECKBOX, [ this ]( wxCommandEvent& ) { UpdateEnabled(); } );
		box->Add( m_diffusion, 0, wxALL, 3 );
		m_coef = MakeSlider( this, 430, 0, 500 );
		m_coef_val = AddSliderRow( box, this, "Strength:", m_coef );
		m_cuterr = MakeSlider( this, 0, 0, 255 );
		m_cuterr_val = AddSliderRow( box, this, "Cut-off:", m_cuterr );
		m_xy = MakeSlider( this, 50, 0, 100 );
		m_xy_val = AddSliderRow( box, this, "X:Y ratio:", m_xy );
		m_coef->Bind( wxEVT_SLIDER, &SettingsDialog::OnDiffusionSlider, this );
		m_cuterr->Bind( wxEVT_SLIDER, &SettingsDialog::OnDiffusionSlider, this );
		m_xy->Bind( wxEVT_SLIDER, &SettingsDialog::OnDiffusionSlider, this );

		const char* const algo[] = { "None", "Recalculate (luminance)",
			"Powerful ray-tracing formula", "NTSC revised", "Recalculate (hue)" };
		AddChoiceRow( "Natural image algorithm:", m_algo, algo, 5 );
		m_jkrc = new wxCheckBox( this, wxID_ANY, "Recalculate color difference (JK)" );
		box->Add( m_jkrc, 0, wxALL, 3 );
		left->Add( box, 0, wxEXPAND | wxALL, 4 );
	}

	// ---- size group
	{
		auto* box = new wxStaticBoxSizer( wxVERTICAL, this, "Size" );
		m_resize = new wxCheckBox( this, wxID_ANY, "Resize image" );
		m_resize->Bind( wxEVT_CHECKBOX, [ this ]( wxCommandEvent& ) { UpdateEnabled(); } );
		box->Add( m_resize, 0, wxALL, 3 );
		auto AddChoiceRow = [ & ]( const char* label, wxChoice*& choice,
								   const char* const* items, int count ) {
			auto* row = new wxBoxSizer( wxHORIZONTAL );
			row->Add( new wxStaticText( this, wxID_ANY, wxString::FromUTF8( label ) ), 0,
					  wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
			choice = MakeChoice( this, items, count );
			row->Add( choice, 1 );
			box->Add( row, 0, wxEXPAND | wxALL, 2 );
		};
		const char* const resample[] = { "Normal", "Antialias", "Copy" };
		AddChoiceRow( "Resample:", m_resample, resample, 3 );
		const char* const sizemode[] = { "Force to output size", "Keep MSX aspect ratio" };
		AddChoiceRow( "Size:", m_size_mode, sizemode, 2 );
		auto* row = new wxBoxSizer( wxHORIZONTAL );
		row->Add( new wxStaticText( this, wxID_ANY, "Background color:" ), 0,
				  wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
		m_bg = new wxColourPickerCtrl( this, wxID_ANY, *wxBLACK );
		row->Add( m_bg, 0 );
		box->Add( row, 0, wxALL, 3 );
		left->Add( box, 0, wxEXPAND | wxALL, 4 );
	}

	// ---- force zero group
	{
		auto* box = new wxStaticBoxSizer( wxVERTICAL, this, "Force to zero" );
		const char* const fz[] = { "Not used", "By color", "By coordinate" };
		m_fzero = MakeChoice( this, fz, 3 );
		m_fzero->Bind( wxEVT_CHOICE, &SettingsDialog::OnFourceZeroChanged, this );
		box->Add( m_fzero, 0, wxEXPAND | wxALL, 3 );
		auto* row = new wxBoxSizer( wxHORIZONTAL );
		row->Add( new wxStaticText( this, wxID_ANY, "Color:" ), 0,
				  wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
		m_fz_color = new wxColourPickerCtrl( this, wxID_ANY, *wxBLACK );
		row->Add( m_fz_color, 0, wxRIGHT, 12 );
		row->Add( new wxStaticText( this, wxID_ANY, "X:" ), 0,
				  wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );
		m_fz_x = new wxSpinCtrl( this, wxID_ANY, "0", wxDefaultPosition, wxSize( 70, -1 ),
								 wxSP_ARROW_KEYS, 0, 4096, 0 );
		row->Add( m_fz_x, 0, wxRIGHT, 8 );
		row->Add( new wxStaticText( this, wxID_ANY, "Y:" ), 0,
				  wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );
		m_fz_y = new wxSpinCtrl( this, wxID_ANY, "0", wxDefaultPosition, wxSize( 70, -1 ),
								 wxSP_ARROW_KEYS, 0, 4096, 0 );
		row->Add( m_fz_y, 0 );
		box->Add( row, 0, wxALL, 3 );
		left->Add( box, 0, wxEXPAND | wxALL, 4 );
	}

	// ---- palette group
	{
		auto* box = new wxStaticBoxSizer( wxVERTICAL, this, "Palette" );
		m_selcol = new wxCheckBox( this, wxID_ANY, "Prefer most-used colors" );
		m_nonzero = new wxCheckBox( this, wxID_ANY, "Do not use color 0" );
		m_fixed_palette = new wxCheckBox( this, wxID_ANY, "Fixed palette (use all entries as-is)" );
		box->Add( m_selcol, 0, wxALL, 2 );
		box->Add( m_nonzero, 0, wxALL, 2 );
		box->Add( m_fixed_palette, 0, wxALL, 2 );

		m_pal_panel = new PalettePanel( this, &m_edit, &m_pal_sel,
			[ this ]() { RefreshPaletteControls(); } );
		box->Add( m_pal_panel, 0, wxALL, 3 );
		m_pal_note = new wxStaticText( this, wxID_ANY, "" );
		box->Add( m_pal_note, 0, wxALL, 3 );

		auto* crow = new wxBoxSizer( wxHORIZONTAL );
		m_color_label = new wxStaticText( this, wxID_ANY, "COLOR = 0" );
		crow->Add( m_color_label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8 );
		const char* const palen[] = { "Auto", "Do not use", "Use set color" };
		m_pal_en = MakeChoice( this, palen, 3 );
		m_pal_en->Bind( wxEVT_CHOICE, &SettingsDialog::OnPalEnChanged, this );
		crow->Add( m_pal_en, 1 );
		box->Add( crow, 0, wxEXPAND | wxALL, 2 );

		m_pal_r = MakeSlider( this, 0, 0, 7, 170 );
		m_pal_r_val = AddSliderRow( box, this, "R:", m_pal_r );
		m_pal_g = MakeSlider( this, 0, 0, 7, 170 );
		m_pal_g_val = AddSliderRow( box, this, "G:", m_pal_g );
		m_pal_b = MakeSlider( this, 0, 0, 7, 170 );
		m_pal_b_val = AddSliderRow( box, this, "B:", m_pal_b );
		m_pal_r->Bind( wxEVT_SLIDER, &SettingsDialog::OnPaletteRgbSlider, this );
		m_pal_g->Bind( wxEVT_SLIDER, &SettingsDialog::OnPaletteRgbSlider, this );
		m_pal_b->Bind( wxEVT_SLIDER, &SettingsDialog::OnPaletteRgbSlider, this );

		m_apply_all = new wxButton( this, wxID_ANY, "Apply permission to all" );
		m_apply_all->Bind( wxEVT_BUTTON, &SettingsDialog::OnApplyPalEnToAll, this );
		box->Add( m_apply_all, 0, wxALL, 3 );

		auto* prow = new wxBoxSizer( wxHORIZONTAL );
		auto* msx1 = new wxButton( this, wxID_ANY, "MSX1 colors" );
		auto* msx2 = new wxButton( this, wxID_ANY, "MSX2 defaults" );
		msx1->Bind( wxEVT_BUTTON, &SettingsDialog::OnPalettePreset, this );
		msx2->Bind( wxEVT_BUTTON, &SettingsDialog::OnPalettePreset, this );
		prow->Add( msx1, 0, wxALL, 3 );
		prow->Add( msx2, 0, wxALL, 3 );
		box->Add( prow, 0, wxALL, 2 );

		auto AddChoiceRow = [ & ]( const char* label, wxChoice*& choice,
								   const char* const* items, int count ) {
			auto* row = new wxBoxSizer( wxHORIZONTAL );
			row->Add( new wxStaticText( this, wxID_ANY, wxString::FromUTF8( label ) ), 0,
					  wxALIGN_CENTER_VERTICAL | wxRIGHT, 6 );
			choice = MakeChoice( this, items, count );
			row->Add( choice, 1 );
			box->Add( row, 0, wxEXPAND | wxALL, 2 );
		};
		const char* const sort[] = { "None (as decided)", "Auto entries, ascending",
			"Auto entries, descending", "All entries, ascending", "All entries, descending" };
		AddChoiceRow( "Sort:", m_sort, sort, 5 );
		const char* const plt[] = { "Do not output", "Embed in image (COLOR=RESTORE)",
			"GraphSaurus (PL) file", "CSV file" };
		AddChoiceRow( "Palette output:", m_plt_mode, plt, 4 );

		auto* frow = new wxBoxSizer( wxHORIZONTAL );
		m_load_pal = new wxButton( this, wxID_ANY, "Load palette..." );
		m_save_pal = new wxButton( this, wxID_ANY, "Save palette..." );
		m_load_pal->Bind( wxEVT_BUTTON, &SettingsDialog::OnLoadPalette, this );
		m_save_pal->Bind( wxEVT_BUTTON, &SettingsDialog::OnSavePalette, this );
		frow->Add( m_load_pal, 0, wxALL, 3 );
		frow->Add( m_save_pal, 0, wxALL, 3 );
		box->Add( frow, 0, wxALL, 2 );
		right->Add( box, 1, wxEXPAND | wxALL, 4 );
	}

	cols->Add( left, 0, wxEXPAND );
	cols->Add( right, 1, wxEXPAND );
	top->Add( cols, 1, wxEXPAND );
	SetSizerAndFit( top );
}

void SettingsDialog::LoadAll() {
	m_mode->SetSelection( m_edit.screen_mode < MD_MAX ? (int) m_edit.screen_mode : 0 );
	m_preview->SetSelection( (int) ( m_edit.PreView >= 1 && m_edit.PreView <= 3 ? m_edit.PreView : 3 ) - 1 );
	m_b192->SetValue( m_edit.b192 );
	m_interlace->SetValue( m_edit.interlace );
	m_autoname->SetValue( m_edit.AutoName );
	m_basic->SetValue( m_edit.output_basic_enable );
	m_cnv_mode->SetSelection( m_edit.CnvMode >= 0 && m_edit.CnvMode <= 2 ? m_edit.CnvMode : 0 );
	m_bdefault->SetValue( m_edit.bDefault );

	m_dither->SetSelection( (int) ( m_edit.ErrAlgo % EALGO_MAX ) );
	m_dither_add->SetSelection( (int) ( m_edit.ErrAdd % EADD_MAX ) );
	m_accuracy->SetSelection( (int) ( m_edit.Seido % SEIDO_MAX ) );
	m_tile->SetValue( m_edit.Tile );
	m_diffusion->SetValue( m_edit.diffusion_error_enable );
	m_coef->SetValue( (int) ( m_edit.diffusion_error_coef * 1000.0f + 0.5f ) );
	m_cuterr->SetValue( (int) m_edit.err );
	m_xy->SetValue( (int) ( m_edit.diffusion_error_x_weight * 100.0f + 0.5f ) );
	m_algo->SetSelection( (int) ( m_edit.AlgoMode % ALGO_MAX ) );
	m_jkrc->SetValue( m_edit.JKrc );

	m_resize->SetValue( m_edit.resize_enable );
	m_resample->SetSelection( m_edit.Resample == RS_ANTIALIAS ? 1 : ( m_edit.Resample == RS_NORMAL ? 0 : 2 ) );
	m_size_mode->SetSelection( m_edit.SizeMode == RM_MSX ? 1 : 0 );
	m_bg->SetColour( wxColour( GET_RED( m_edit.FCColor ), GET_GREEN( m_edit.FCColor ), GET_BLUE( m_edit.FCColor ) ) );

	m_fzero->SetSelection( (int) ( m_edit.FourceZero % FZ_MAX ) );
	m_fz_color->SetColour( wxColour( GET_RED( m_edit.FZColor ), GET_GREEN( m_edit.FZColor ), GET_BLUE( m_edit.FZColor ) ) );
	m_fz_x->SetValue( (int) m_edit.FZX );
	m_fz_y->SetValue( (int) m_edit.FZY );

	m_selcol->SetValue( m_edit.SelCol == SELCOL_TABUNPU );
	m_nonzero->SetValue( m_edit.NonZero );
	m_fixed_palette->SetValue( m_edit.fixed_palette );
	m_sort->SetSelection( m_edit.SortMode >= 0 && m_edit.SortMode <= 4 ? m_edit.SortMode : 0 );
	m_plt_mode->SetSelection( (int) ( m_edit.PltMode % PLT_COUNT ) );

	SelectPaletteEntry( 0 );
	RefreshSliderLabels();
}

void SettingsDialog::StoreAll() {
	m_edit.screen_mode = (uint32_t) m_mode->GetSelection();
	m_edit.PreView = (uint32_t) ( m_preview->GetSelection() + 1 );
	m_edit.b192 = m_b192->GetValue();
	m_edit.interlace = m_interlace->GetValue();
	m_edit.AutoName = m_autoname->GetValue();
	m_edit.output_basic_enable = m_basic->GetValue();
	m_edit.CnvMode = m_cnv_mode->GetSelection();
	m_edit.bDefault = m_bdefault->GetValue();

	m_edit.ErrAlgo = (uint32_t) m_dither->GetSelection();
	m_edit.ErrAdd = (uint32_t) m_dither_add->GetSelection();
	m_edit.Seido = (uint32_t) m_accuracy->GetSelection();
	m_edit.Tile = m_tile->GetValue();
	m_edit.diffusion_error_enable = m_diffusion->GetValue();
	m_edit.diffusion_error_coef = m_coef->GetValue() / 1000.0f;
	m_edit.err = (uint32_t) m_cuterr->GetValue();
	m_edit.diffusion_error_x_weight = m_xy->GetValue() / 100.0f;
	m_edit.AlgoMode = (uint32_t) m_algo->GetSelection();
	m_edit.JKrc = m_jkrc->GetValue();

	m_edit.resize_enable = m_resize->GetValue();
	m_edit.Resample = m_resample->GetSelection() == 1 ? RS_ANTIALIAS
					: ( m_resample->GetSelection() == 0 ? RS_NORMAL : 2 );
	m_edit.SizeMode = m_size_mode->GetSelection() == 1 ? RM_MSX : RM_FORCE;
	wxColour bg = m_bg->GetColour();
	m_edit.FCColor = GET_RGB( bg.Red(), bg.Green(), bg.Blue() );

	m_edit.FourceZero = (uint32_t) m_fzero->GetSelection();
	wxColour fz = m_fz_color->GetColour();
	m_edit.FZColor = GET_RGB( fz.Red(), fz.Green(), fz.Blue() );
	m_edit.FZX = (uint32_t) m_fz_x->GetValue();
	m_edit.FZY = (uint32_t) m_fz_y->GetValue();

	m_edit.SelCol = m_selcol->GetValue() ? SELCOL_TABUNPU : SELCOL_BUNSAN;
	m_edit.NonZero = m_nonzero->GetValue();
	m_edit.fixed_palette = m_fixed_palette->GetValue();
	m_edit.SortMode = m_sort->GetSelection();
	m_edit.PltMode = (uint32_t) m_plt_mode->GetSelection();
	// color_palette[] / PalEn[] are edited live by the palette controls.
}

void SettingsDialog::UpdateEnabled() {
	uint32_t mode = (uint32_t) m_mode->GetSelection();
	bool pal = IsPaletteMode( mode );
	for( wxWindow* w : { (wxWindow*) m_selcol, (wxWindow*) m_nonzero,
						 (wxWindow*) m_fixed_palette, (wxWindow*) m_pal_panel,
						 (wxWindow*) m_pal_en, (wxWindow*) m_pal_r, (wxWindow*) m_pal_g,
						 (wxWindow*) m_pal_b, (wxWindow*) m_apply_all, (wxWindow*) m_load_pal,
						 (wxWindow*) m_save_pal, (wxWindow*) m_sort, (wxWindow*) m_plt_mode } ) {
		w->Enable( pal );
	}
	if( IsEpalMode( mode ) ) {
		m_pal_note->SetLabel( "SCREEN8 EPAL generates its 256-color palette automatically;\nthe 16-color palette settings do not apply." );
	} else if( !pal ) {
		m_pal_note->SetLabel( "This screen mode does not use the 16-color palette." );
	} else {
		m_pal_note->SetLabel( "" );
	}
	m_algo->Enable( IsNaturalMode( mode ) );
	m_jkrc->Enable( IsNaturalMode( mode ) );
	m_tile->Enable( IsTileMode( mode ) );
	bool diff = m_diffusion->GetValue();
	m_coef->Enable( diff );
	m_cuterr->Enable( diff );
	m_xy->Enable( diff );
	m_b192->Enable( mode >= MD_SC5 );
	m_resample->Enable( m_resize->GetValue() );
	m_size_mode->Enable( m_resize->GetValue() );
	UpdateFourceZeroEnabled();
}

void SettingsDialog::UpdateFourceZeroEnabled() {
	int sel = m_fzero->GetSelection();
	m_fz_color->Enable( sel == FZ_COLOR );
	m_fz_x->Enable( sel == FZ_MATRIX );
	m_fz_y->Enable( sel == FZ_MATRIX );
}

void SettingsDialog::RefreshSliderLabels() {
	m_coef_val->SetLabel( wxString::Format( "%.3f", m_coef->GetValue() / 1000.0 ) );
	m_cuterr_val->SetLabel( wxString::Format( "%d", m_cuterr->GetValue() ) );
	m_xy_val->SetLabel( wxString::Format( "%.2f", m_xy->GetValue() / 100.0 ) );
}

void SettingsDialog::SelectPaletteEntry( int idx ) {
	m_pal_sel = idx;
	m_color_label->SetLabel( wxString::Format( "COLOR = %d", idx ) );
	m_pal_en->SetSelection( m_edit.PalEn[ idx ] % PALEN_MAX );
	m_pal_r->SetValue( m_edit.color_palette[ idx ].red );
	m_pal_g->SetValue( m_edit.color_palette[ idx ].green );
	m_pal_b->SetValue( m_edit.color_palette[ idx ].blue );
	m_pal_r_val->SetLabel( wxString::Format( "%d", m_edit.color_palette[ idx ].red ) );
	m_pal_g_val->SetLabel( wxString::Format( "%d", m_edit.color_palette[ idx ].green ) );
	m_pal_b_val->SetLabel( wxString::Format( "%d", m_edit.color_palette[ idx ].blue ) );
}

void SettingsDialog::RefreshPaletteControls() {
	SelectPaletteEntry( m_pal_sel );
	if( m_pal_panel ) m_pal_panel->Refresh();
}

void SettingsDialog::OnModeChanged( wxCommandEvent& ) { UpdateEnabled(); }

void SettingsDialog::OnFourceZeroChanged( wxCommandEvent& ) {
	UpdateFourceZeroEnabled();
}

void SettingsDialog::OnOk( wxCommandEvent& ) {
	StoreAll();
	EndModal( wxID_OK );
}

void SettingsDialog::OnSaveFile( wxCommandEvent& ) {
	wxFileDialog dlg( this, "Save settings", "", "BMP2MSX.B2M",
					  "BMP2MSX settings (*.B2M)|*.B2M|All files|*.*",
					  wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	if( dlg.ShowModal() != wxID_OK ) return;
	StoreAll();
	SaveCfgFile( &m_edit, dlg.GetPath().utf8_str().data() );
	LoadAll();	// StoreAll may have normalized values; keep the view in sync
}

void SettingsDialog::OnLoadFile( wxCommandEvent& ) {
	wxFileDialog dlg( this, "Load settings", "", "",
					  "BMP2MSX settings (*.B2M)|*.B2M|All files|*.*", wxFD_OPEN );
	if( dlg.ShowModal() != wxID_OK ) return;
	if( !GetCfgFile( &m_edit, dlg.GetPath().utf8_str().data() ) ) {
		wxMessageBox( "Could not load the settings file.", "BMP2MSX",
					  wxOK | wxICON_WARNING, this );
		return;
	}
	LoadAll();
	UpdateEnabled();
	if( m_pal_panel ) m_pal_panel->Refresh();
}

void SettingsDialog::OnCopyToDefaults( wxCommandEvent& ) {
	StoreAll();
	if( m_copy_to_defaults ) m_copy_to_defaults( m_edit );
	wxMessageBox( "The current edits were copied to the default settings.",
				  "BMP2MSX", wxOK | wxICON_INFORMATION, this );
}

void SettingsDialog::OnPalettePreset( wxCommandEvent& e ) {
	// The two preset buttons are told apart by their label.
	wxWindow* src = (wxWindow*) e.GetEventObject();
	if( src && src->GetLabel() == "MSX1 colors" ) {
		set_msx1_palette( m_edit.color_palette );
	} else {
		set_msx2_palette( m_edit.color_palette );
	}
	RefreshPaletteControls();
}

void SettingsDialog::OnApplyPalEnToAll( wxCommandEvent& ) {
	for( int i = 0; i < 16; ++i ) m_edit.PalEn[ i ] = m_edit.PalEn[ m_pal_sel ];
	if( m_pal_panel ) m_pal_panel->Refresh();
}

void SettingsDialog::OnPalEnChanged( wxCommandEvent& ) {
	m_edit.PalEn[ m_pal_sel ] = (uint8_t) m_pal_en->GetSelection();
	if( m_pal_panel ) m_pal_panel->Refresh();
}

void SettingsDialog::OnPaletteRgbSlider( wxCommandEvent& ) {
	m_edit.color_palette[ m_pal_sel ].red   = m_pal_r->GetValue();
	m_edit.color_palette[ m_pal_sel ].green = m_pal_g->GetValue();
	m_edit.color_palette[ m_pal_sel ].blue  = m_pal_b->GetValue();
	m_pal_r_val->SetLabel( wxString::Format( "%d", m_pal_r->GetValue() ) );
	m_pal_g_val->SetLabel( wxString::Format( "%d", m_pal_g->GetValue() ) );
	m_pal_b_val->SetLabel( wxString::Format( "%d", m_pal_b->GetValue() ) );
	if( m_pal_panel ) m_pal_panel->Refresh();
}

void SettingsDialog::OnDiffusionSlider( wxCommandEvent& ) { RefreshSliderLabels(); }

void SettingsDialog::OnLoadPalette( wxCommandEvent& ) {
	wxFileDialog dlg( this, "Load palette", "", "",
					  "Palette files (*.PL?;*.S?" "?)|*.PL?;*.S?" "?|All files|*.*", wxFD_OPEN );
	if( dlg.ShowModal() != wxID_OK ) return;
	wxString path = dlg.GetPath();
	wxFileName fn( path );
	wxString ext = fn.GetExt();
	// Same rule as Setting.cpp: a .PL? extension means the raw GraphSaurus
	// palette format (32 bytes); anything else is read as a BSAVE file
	// with the core's bsvLoadPLBsaveFile().
	if( ext.length() == 3 && ( ext[ 0 ] == 'P' || ext[ 0 ] == 'p' ) ) {
		FILE* f = fopen( path.utf8_str().data(), "rb" );
		if( !f ) {
			wxMessageBox( "Could not open the palette file.", "BMP2MSX",
						  wxOK | wxICON_WARNING, this );
			return;
		}
		unsigned char buf[ 32 ] = { 0 };
		size_t got = fread( buf, 1, sizeof( buf ), f );
		fclose( f );
		if( got == 0 ) return;
		for( int i = 0; i < 16; ++i ) {
			m_edit.color_palette[ i ].red   = ( buf[ i * 2 + 0 ] >> 4 ) & 0x07;
			m_edit.color_palette[ i ].blue  =   buf[ i * 2 + 0 ]        & 0x07;
			m_edit.color_palette[ i ].green =   buf[ i * 2 + 1 ]        & 0x07;
		}
	} else {
		bsvLoadPLBsaveFile( &m_edit, path.utf8_str().data() );
	}
	RefreshPaletteControls();
}

void SettingsDialog::OnSavePalette( wxCommandEvent& ) {
	wxFileDialog dlg( this, "Save palette", "", "palette.PL5",
					  "GraphSaurus palette (*.PL?)|*.PL?|All files|*.*",
					  wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	if( dlg.ShowModal() != wxID_OK ) return;
	bsvSavePLFile( nullptr, dlg.GetPath().utf8_str().data(), &m_edit );
}
