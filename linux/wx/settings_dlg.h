// ---------------------------------------------------------------------------
// Settings dialog of the wxWidgets front end: the counterpart of the
// Windows setting dialog (../source/Setting.cpp).  It edits a SETTING
// copy with the same groups and the same enable/disable rules as the
// original (palette controls only for the palette modes, the natural
// image algorithm only for SCREEN10/12, tiles only for the modes whose
// converter builds them, ...), in two flavors:
//
//   Purpose::DefaultSettings -- the "default setting" dialog; OK commits
//                               the edited copy as the new defaults.
//   Purpose::Reconvert       -- the "re-convert" dialog; OK re-runs the
//                               conversion of the preview it was opened
//                               for, and it offers "Copy to defaults".
//
// Plain wxWidgets + the shared core only (portable; the file dialogs go
// through wxFileDialog, the B2M load/save through the core's
// GetCfgFile()/SaveCfgFile()).
// ---------------------------------------------------------------------------
#ifndef BMP2MSX_WX_SETTINGS_DLG_H
#define BMP2MSX_WX_SETTINGS_DLG_H

#include <wx/wx.h>
#include <functional>
#include "Convert.h"

class PalettePanel;
class wxColourPickerCtrl;
class wxSpinCtrl;

class SettingsDialog : public wxDialog {
public:
	enum class Purpose { DefaultSettings, Reconvert };

	// copy_to_defaults (Reconvert only) is invoked when the user presses
	// the "Copy to defaults" button, with the dialog's current edits.
	SettingsDialog( wxWindow* parent, const SETTING& initial, Purpose purpose,
					std::function<void( const SETTING& )> copy_to_defaults = nullptr );

	const SETTING& Edited() const { return m_edit; }

private:
	void BuildUi( Purpose purpose );
	void LoadAll();
	void StoreAll();
	void UpdateEnabled();
	void UpdateFourceZeroEnabled();
	void RefreshPaletteControls();
	void RefreshSliderLabels();
	void ApplyPaletteRange();
	void SelectPaletteEntry( int idx );

	void OnModeChanged( wxCommandEvent& );
	void OnOk( wxCommandEvent& );
	void OnSaveFile( wxCommandEvent& );
	void OnLoadFile( wxCommandEvent& );
	void OnCopyToDefaults( wxCommandEvent& );
	void OnPalettePreset( wxCommandEvent& );
	void OnApplyPalEnToAll( wxCommandEvent& );
	void OnLoadPalette( wxCommandEvent& );
	void OnSavePalette( wxCommandEvent& );
	void OnPalEnChanged( wxCommandEvent& );
	void OnPaletteRgbSlider( wxCommandEvent& );
	void OnDiffusionSlider( wxCommandEvent& );
	void OnFourceZeroChanged( wxCommandEvent& );

	SETTING m_edit;
	int m_pal_sel = 0;
	std::function<void( const SETTING& )> m_copy_to_defaults;

	wxChoice* m_mode = nullptr;
	wxRadioBox* m_preview = nullptr;
	wxCheckBox* m_b192 = nullptr;
	wxCheckBox* m_interlace = nullptr;
	wxCheckBox* m_autoname = nullptr;
	wxCheckBox* m_basic = nullptr;
	wxRadioBox* m_cnv_mode = nullptr;
	wxCheckBox* m_bdefault = nullptr;

	wxChoice* m_dither = nullptr;
	wxChoice* m_dither_add = nullptr;
	wxChoice* m_accuracy = nullptr;
	wxCheckBox* m_tile = nullptr;
	wxCheckBox* m_diffusion = nullptr;
	wxSlider* m_coef = nullptr;
	wxSlider* m_cuterr = nullptr;
	wxSlider* m_xy = nullptr;
	wxStaticText* m_coef_val = nullptr;
	wxStaticText* m_cuterr_val = nullptr;
	wxStaticText* m_xy_val = nullptr;
	wxChoice* m_algo = nullptr;
	wxCheckBox* m_jkrc = nullptr;

	wxCheckBox* m_resize = nullptr;
	wxChoice* m_resample = nullptr;
	wxChoice* m_size_mode = nullptr;
	wxColourPickerCtrl* m_bg = nullptr;

	wxChoice* m_fzero = nullptr;
	wxColourPickerCtrl* m_fz_color = nullptr;
	wxSpinCtrl* m_fz_x = nullptr;
	wxSpinCtrl* m_fz_y = nullptr;

	wxCheckBox* m_selcol = nullptr;
	wxCheckBox* m_nonzero = nullptr;
	wxCheckBox* m_fixed_palette = nullptr;
	wxCheckBox* m_epal_hguard = nullptr;
	PalettePanel* m_pal_panel = nullptr;
	wxStaticText* m_pal_note = nullptr;
	wxStaticText* m_color_label = nullptr;
	wxChoice* m_pal_en = nullptr;
	wxSlider* m_pal_r = nullptr;
	wxSlider* m_pal_g = nullptr;
	wxSlider* m_pal_b = nullptr;
	wxStaticText* m_pal_r_val = nullptr;
	wxStaticText* m_pal_g_val = nullptr;
	wxStaticText* m_pal_b_val = nullptr;
	wxChoice* m_sort = nullptr;
	wxChoice* m_plt_mode = nullptr;
	wxButton* m_apply_all = nullptr;
	wxButton* m_load_pal = nullptr;
	wxButton* m_save_pal = nullptr;
};

#endif
