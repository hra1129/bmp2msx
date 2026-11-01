// See original_wnd.h.
#include "original_wnd.h"

#include "preview_render.h"

namespace {

enum { ID_ZOOM_1 = wxID_HIGHEST + 1, ID_ZOOM_2 };

wxImage ImageFromPixels( const std::vector<C_COLOR>& px, int w, int h ) {
	std::vector<unsigned char> rgb( (size_t) w * h * 3 );
	render_color_buffer( px.data(), w, h, rgb.data() );
	wxImage img( w, h );
	memcpy( img.GetData(), rgb.data(), rgb.size() );
	return img;
}

} // namespace

OriginalFrame::OriginalFrame( wxWindow* parent, const wxString& title,
							  const std::vector<C_COLOR>& pixels, int w, int h )
	: wxFrame( parent, wxID_ANY, title, wxDefaultPosition, wxSize( 560, 480 ) ),
	  m_pixels( pixels ), m_w( w ), m_h( h ) {
	auto* menu_bar = new wxMenuBar();
	auto* func = new wxMenu();
	func->AppendRadioItem( ID_ZOOM_1, "1x" );
	func->AppendRadioItem( ID_ZOOM_2, "2x" );
	menu_bar->Append( func, "View" );
	auto* file = new wxMenu();
	file->Append( wxID_CLOSE, "Close" );
	menu_bar->Append( file, "File" );
	SetMenuBar( menu_bar );
	Bind( wxEVT_MENU, &OriginalFrame::OnZoom, this, ID_ZOOM_1 );
	Bind( wxEVT_MENU, &OriginalFrame::OnZoom, this, ID_ZOOM_2 );
	Bind( wxEVT_MENU, [ this ]( wxCommandEvent& ) { Close(); }, wxID_CLOSE );

	m_scroll = new wxScrolledWindow( this );
	m_scroll->SetScrollRate( 16, 16 );
	m_canvas = new wxPanel( m_scroll );
	m_canvas->SetBackgroundStyle( wxBG_STYLE_PAINT );
	m_canvas->Bind( wxEVT_PAINT, &OriginalFrame::OnPaint, this );
	m_canvas->Bind( wxEVT_LEFT_DOWN, &OriginalFrame::OnClick, this );
	m_canvas->SetCursor( wxCursor( wxCURSOR_CROSS ) );
	auto* sizer = new wxBoxSizer( wxVERTICAL );
	sizer->Add( m_canvas, 1, wxEXPAND );
	m_scroll->SetSizer( sizer );
	m_canvas->SetSize( m_w, m_h );
	m_scroll->SetVirtualSize( m_w, m_h );

	auto* top = new wxBoxSizer( wxVERTICAL );
	top->Add( m_scroll, 1, wxEXPAND );
	SetSizer( top );
}

void OriginalFrame::OnZoom( wxCommandEvent& e ) {
	m_zoom = ( e.GetId() == ID_ZOOM_2 ) ? 2 : 1;
	m_canvas->SetSize( m_w * m_zoom, m_h * m_zoom );
	m_scroll->SetVirtualSize( m_w * m_zoom, m_h * m_zoom );
	m_canvas->Refresh();
}

void OriginalFrame::OnPaint( wxPaintEvent& ) {
	wxPaintDC dc( m_canvas );
	wxImage img = ImageFromPixels( m_pixels, m_w, m_h );
	if( m_zoom != 1 ) {
		img = img.Rescale( m_w * m_zoom, m_h * m_zoom, wxIMAGE_QUALITY_NEAREST );
	}
	dc.DrawBitmap( wxBitmap( img ), 0, 0 );
}

void OriginalFrame::OnClick( wxMouseEvent& e ) {
	wxPoint p = e.GetPosition();
	int x = p.x / m_zoom, y = p.y / m_zoom;
	if( x < 0 || x >= m_w || y < 0 || y >= m_h ) return;
	C_COLOR c = m_pixels[ (size_t) y * m_w + x ];
	int r8 = (int) GET_RED( c ), g8 = (int) GET_GREEN( c ), b8 = (int) GET_BLUE( c );
	// MSX 512-color approximation, as in SpitProc.cpp:
	//   level = (value + 18) * 7 / 255, shown through the convert tables.
	int mr = ( r8 + 18 ) * 7 / 255;
	int mg = ( g8 + 18 ) * 7 / 255;
	int mb = ( b8 + 18 ) * 7 / 255;

	wxDialog dlg( this, wxID_ANY, "Eyedropper", wxDefaultPosition, wxDefaultSize,
				  wxDEFAULT_DIALOG_STYLE );
	auto* sizer = new wxBoxSizer( wxVERTICAL );
	auto* swatches = new wxBoxSizer( wxHORIZONTAL );
	auto add_swatch = [ & ]( const wxColour& col, const char* label ) {
		auto* col_box = new wxBoxSizer( wxVERTICAL );
		auto* sw = new wxPanel( &dlg, wxID_ANY, wxDefaultPosition, wxSize( 64, 48 ),
								wxBORDER_SUNKEN );
		sw->SetBackgroundColour( col );
		col_box->Add( sw, 0, wxALL, 3 );
		col_box->Add( new wxStaticText( &dlg, wxID_ANY, wxString::FromUTF8( label ) ),
					  0, wxALIGN_CENTER );
		swatches->Add( col_box, 0, wxALL, 4 );
	};
	add_swatch( wxColour( r8, g8, b8 ), "Picked color" );
	add_swatch( wxColour( convert7to255_r[ mr ], convert7to255_g[ mg ],
						  convert7to255_b[ mb ] ), "MSX approximation" );
	sizer->Add( swatches, 0, wxALL, 4 );

	auto add_row = [ & ]( const char* label, const wxString& value ) {
		auto* row = new wxBoxSizer( wxHORIZONTAL );
		row->Add( new wxStaticText( &dlg, wxID_ANY, wxString::FromUTF8( label ) ),
				  0, wxRIGHT, 8 );
		row->Add( new wxStaticText( &dlg, wxID_ANY, value ), 1 );
		sizer->Add( row, 0, wxEXPAND | wxALL, 3 );
	};
	add_row( "Color code:", wxString::Format( "%08XH", (unsigned) c ) );
	add_row( "Red / Green / Blue:",
			 wxString::Format( "%d / %d / %d", r8, g8, b8 ) );
	add_row( "Position:", wxString::Format( "(%d, %d)", x, y ) );
	add_row( "MSX palette reference:",
			 wxString::Format( "R %d, G %d, B %d", mr, mg, mb ) );

	auto* ok = new wxButton( &dlg, wxID_OK, "Close" );
	sizer->Add( ok, 0, wxALIGN_CENTER | wxALL, 6 );
	dlg.SetSizerAndFit( sizer );
	dlg.ShowModal();
}
