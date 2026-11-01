// ---------------------------------------------------------------------------
// Original image window of the wxWidgets front end: the counterpart of
// the Windows original image window (manual ch.11).  It shows the
// untouched input image (1x / 2x zoom), resizable, and clicking a pixel
// opens the eyedropper dialog with the same information as the Windows
// one: the raw color, its MSX 512-color approximation, the color code,
// R/G/B values, coordinates, and the MSX palette reference values.
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_WX_ORIGINAL_WND_H
#define BMP2MSX_WX_ORIGINAL_WND_H

#include <wx/wx.h>
#include <vector>
#include "Convert.h"

class OriginalFrame : public wxFrame {
public:
	OriginalFrame( wxWindow* parent, const wxString& title,
				   const std::vector<C_COLOR>& pixels, int w, int h );

private:
	void OnPaint( wxPaintEvent& );
	void OnClick( wxMouseEvent& );
	void OnZoom( wxCommandEvent& );

	std::vector<C_COLOR> m_pixels;
	int m_w, m_h;
	int m_zoom = 1;
	wxScrolledWindow* m_scroll = nullptr;
	wxPanel* m_canvas = nullptr;
};

#endif
