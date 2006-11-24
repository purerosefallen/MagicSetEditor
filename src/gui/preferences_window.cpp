//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/preferences_window.hpp>
#include <gui/update_checker.hpp>
#include <data/settings.hpp>
#include <util/window_id.hpp>
#include <wx/spinctrl.h>
#include <wx/filename.h>

// ----------------------------------------------------------------------------- : Preferences pages

// A page from the preferences dialog
class PreferencesPage : public wxPanel {
  public:
	PreferencesPage(Window* parent)
		: wxPanel(parent, wxID_ANY)
	{}
	
	/// Stores the settings from the panel in the global settings object
	virtual void store() = 0;
};

// Preferences page for card viewing related settings
class DisplayPreferencesPage : public PreferencesPage {
  public:
	DisplayPreferencesPage(Window* parent);
	void store();	
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxCheckBox* high_quality, *borders;
	wxSpinCtrl* zoom;
	wxCheckBox* non_normal_export;
	
	void onSelectColumns(wxCommandEvent&);
};

// Preferences page for directories of programs
// i.e. Apprentice, Magic Workstation
// perhaps in the future also directories for packages?
class DirsPreferencesPage : public PreferencesPage {
  public:
	DirsPreferencesPage(Window* parent);
	void store();
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxTextCtrl* apprentice;
	
	void onApprenticeBrowse(wxCommandEvent&);
};

// Preferences page for automatic updates
class UpdatePreferencesPage : public PreferencesPage {
  public:
	UpdatePreferencesPage(Window* parent);
	void store();
	
  private:
	DECLARE_EVENT_TABLE();
	
	wxChoice* check_at_startup;
	
	// check for updates
	void onCheckUpdatesNow(wxCommandEvent&);
};


// ----------------------------------------------------------------------------- : PreferencesWindow

PreferencesWindow::PreferencesWindow(Window* parent)
	: wxDialog(parent, wxID_ANY, _("Preferences"), wxDefaultPosition)
{
	// init notebook
	wxNotebook* nb = new wxNotebook(this, ID_NOTEBOOK);
	nb->AddPage(new DisplayPreferencesPage(nb), _("Display"));
	nb->AddPage(new DirsPreferencesPage   (nb), _("Directories"));
	nb->AddPage(new UpdatePreferencesPage (nb), _("Updates"));
	
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
	s->Add(nb,                                 1, wxEXPAND | wxALL & ~wxBOTTOM, 8);
	s->AddSpacer(4);
	s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL & ~wxTOP,    8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void PreferencesWindow::onOk(wxCommandEvent&) {
	// store each page
	wxNotebook* nb = static_cast<wxNotebook*>(FindWindow(ID_NOTEBOOK));
	size_t count = nb->GetPageCount();
	for (size_t i = 0 ; i < count ; ++i) {
		static_cast<PreferencesPage*>(nb->GetPage(i))->store();
	}
	// close
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(PreferencesWindow, wxDialog)
	EVT_BUTTON       (wxID_OK, PreferencesWindow::onOk)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Preferences page : display

DisplayPreferencesPage::DisplayPreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	high_quality      = new wxCheckBox(this, wxID_ANY, _("&High quality rendering"));
	borders           = new wxCheckBox(this, wxID_ANY, _("Show &lines around fields"));
	zoom              = new wxSpinCtrl(this, wxID_ANY);
	non_normal_export = new wxCheckBox(this, wxID_ANY, _("Use zoom and rotation settings when e&xporting"));
	wxButton* columns = new wxButton(this, ID_SELECT_COLUMNS, _("Select..."));
	// set values
	high_quality->     SetValue( settings.default_stylesheet_settings.card_anti_alias());
	borders->          SetValue( settings.default_stylesheet_settings.card_borders());
	non_normal_export->SetValue(!settings.default_stylesheet_settings.card_normal_export());
	zoom->SetRange(1, 1000);
	zoom->             SetValue(  settings.default_stylesheet_settings.card_zoom() * 100);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, this, _("Card Display"));
			s2->Add(high_quality, 0, wxEXPAND | wxALL, 4);
			s2->Add(borders,      0, wxEXPAND | wxALL, 4);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				s3->Add(new wxStaticText(this, wxID_ANY, _("&Zoom:")),          0, wxALL & ~wxLEFT,  4);
				s3->Add(zoom);
				s3->Add(new wxStaticText(this, wxID_ANY, _("% of normal size")),1, wxALL & ~wxRIGHT, 4);
			s2->Add(s3, 0, wxEXPAND | wxALL, 4);
			s2->Add(non_normal_export,0, wxEXPAND | wxALL, 4);
			s2->Add(new wxStaticText(this, wxID_ANY, _("(When off the cards are exported\n and copied at 100% size and normal rotation)")), 0, wxALL & ~wxTOP, 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
		wxSizer* s4 = new wxStaticBoxSizer(wxVERTICAL, this, _("Card List"));
			wxSizer* s5 = new wxBoxSizer(wxHORIZONTAL);
				s5->Add(new wxStaticText(this, wxID_ANY, _("Columns: ")),      0, wxALL & ~wxLEFT | wxEXPAND, 4);
				s5->Add(columns);
			s4->Add(s5, 0, wxEXPAND | wxALL, 4);
		s->Add(s, 0, wxEXPAND | wxALL & ~wxTOP, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void DisplayPreferencesPage::store() {
	settings.default_stylesheet_settings.card_anti_alias    = high_quality->GetValue();
	settings.default_stylesheet_settings.card_borders       = borders->GetValue();
	settings.default_stylesheet_settings.card_zoom          = zoom->GetValue() / 100.0;
	settings.default_stylesheet_settings.card_normal_export = !non_normal_export->GetValue();
}

void DisplayPreferencesPage::onSelectColumns(wxCommandEvent&) {
	// TODO
}

BEGIN_EVENT_TABLE(DisplayPreferencesPage, wxPanel)
	EVT_BUTTON       (ID_SELECT_COLUMNS, DisplayPreferencesPage::onSelectColumns)
END_EVENT_TABLE  ()


// ----------------------------------------------------------------------------- : Preferences page : directories

DirsPreferencesPage::DirsPreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	apprentice   = new wxTextCtrl(this, wxID_ANY);
	wxButton* ab = new wxButton(this, ID_APPRENTICE_BROWSE, _("&Browse..."));
	// set values
	apprentice->SetValue(settings.apprentice_location);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		wxSizer* s2 = new wxStaticBoxSizer(wxVERTICAL, this, _("External programs"));
			s2->Add(new wxStaticText(this, wxID_ANY, _("&Apprentice:")), 0, wxALL, 4);
			wxSizer* s3 = new wxBoxSizer(wxHORIZONTAL);
				s3->Add(apprentice, 1, wxEXPAND | wxRIGHT, 4);
				s3->Add(ab,         0, wxEXPAND);
			s2->Add(s3, 0, wxEXPAND | wxALL & ~wxTOP, 4);
		s->Add(s2, 0, wxEXPAND | wxALL, 8);
	s->SetSizeHints(this);
	SetSizer(s);
}

void DirsPreferencesPage::store() {
	settings.apprentice_location = apprentice->GetValue();
}

void DirsPreferencesPage::onApprenticeBrowse(wxCommandEvent&) {
	// browse for appr.exe
	wxFileDialog dlg(this, _("Locate apprentice"), apprentice->GetValue(), _(""), _("Apprentice Executable|appr.exe"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileName fn(dlg.GetPath());
		apprentice->SetValue(fn.GetPath());
	}
}
	
BEGIN_EVENT_TABLE(DirsPreferencesPage, wxPanel)
	EVT_BUTTON     (ID_APPRENTICE_BROWSE, DirsPreferencesPage::onApprenticeBrowse)
END_EVENT_TABLE  ();


// ----------------------------------------------------------------------------- : Preferences page : updates

UpdatePreferencesPage::UpdatePreferencesPage(Window* parent)
	: PreferencesPage(parent)
{
	// init controls
	check_at_startup    = new wxChoice(this, wxID_ANY);
	wxButton* check_now = new wxButton(this, ID_CHECK_UPDATES_NOW, _("Check &Now"));
	// set values
	check_at_startup->Append(_("Always"));                        // 0
	check_at_startup->Append(_("If internet connection exists")); // 1
	check_at_startup->Append(_("Never"));                         // 2
	check_at_startup->SetSelection(settings.check_updates);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		s->Add(new wxStaticText(this, wxID_ANY, _("&Check for new versions at startup:")), 0, wxALL, 8);
		s->Add(check_at_startup, 0, wxALL & ~wxTOP, 8);
		s->Add(check_now,        0, wxALL & ~wxTOP, 8);
		s->Add(new wxStaticText(this, wxID_ANY, _("Checking for updates requires an internet connection.\nWhen no internet connection is found upates are not checked.\n\nNo information is collected when checking for updates.")), 0, wxALL & ~wxTOP, 8);
	SetSizer(s);
}

void UpdatePreferencesPage::store() {
	int sel = check_at_startup->GetSelection();
	if      (sel == 0) settings.check_updates = CHECK_ALWAYS;
	else if (sel == 1) settings.check_updates = CHECK_IF_CONNECTED;
	else               settings.check_updates = CHECK_NEVER;
}

void UpdatePreferencesPage::onCheckUpdatesNow(wxCommandEvent&) {
	check_updates_now(false);
	if (!update_data_found()) {
		wxMessageBox(_("Checking updates failed"), _("Update Check"), wxICON_ERROR | wxOK);
	} else if (!update_available()) {
		wxMessageBox(_("There are no available updates."), _("Update Check"), wxICON_INFORMATION | wxOK);
	} else {
		show_update_dialog(GetParent());
	}
}

BEGIN_EVENT_TABLE(UpdatePreferencesPage, wxPanel)
	EVT_BUTTON      (ID_CHECK_UPDATES_NOW, UpdatePreferencesPage::onCheckUpdatesNow)
END_EVENT_TABLE  ()