#include <wx/wx.h>

#include "declarative_ui.hpp"

class MainDialog : public wxDialog
{
public:
    MainDialog(wxWindow* parent, wxWindowID id, const wxString& title)
        : wxDialog(parent, id, title, wxDefaultPosition, wxSize(400, 200))
    {
        VStack {
            wxSizerFlags().Border(),
            HStack {
                Button{"Action"},
                TextCtrl{""}
                    .withFlags(wxSizerFlags().Expand())
            },
            Slider { {0, 100}, 50 }
                .withFlags(wxEXPAND),
            HStack {
                StaticText{"Status:"},
                Button{"Close"}
            }
        }.fitTo(this);
        

        // wxButton*     m_btnBrowse = nullptr;
        // wxTextCtrl*   m_txtPath   = nullptr;

        // wxSlider*     m_slider    = nullptr;

        // wxStaticText* m_lblStatus = nullptr;
        // wxButton*     m_btnOk     = nullptr;
        // // --- Controls (Row 1) ---
        // m_btnBrowse = new wxButton(this, wxID_ANY, "Browse...");
        // m_txtPath   = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);

        // // --- Controls (Row 2) ---
        // m_slider = new wxSlider(this, wxID_ANY, 50, 0, 100,
        //                         wxDefaultPosition, wxDefaultSize,
        //                         wxSL_HORIZONTAL);

        // // --- Controls (Row 3) ---
        // m_lblStatus = new wxStaticText(this, wxID_ANY, "Ready");
        // m_btnOk     = new wxButton(this, wxID_OK, "OK");

        // // ===== Layout =====

        // // Main vertical sizer (3 rows)
        // auto* mainSizer = new wxBoxSizer(wxVERTICAL);

        // // Row 1: [Button][TextCtrl expands]
        // auto* row1 = new wxBoxSizer(wxHORIZONTAL);
        // row1->Add(m_btnBrowse, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        // row1->Add(m_txtPath,   0, wxEXPAND); // expand to full remaining width

        // // Row 2: [Slider expands full width]
        // // Use wxEXPAND to stretch horizontally.
        // auto* row2 = new wxBoxSizer(wxHORIZONTAL);
        // row2->Add(m_slider, 1, wxEXPAND); // slider expands across dialog

        // // Row 3: [StaticText expands][Button]
        // auto* row3 = new wxBoxSizer(wxHORIZONTAL);
        // row3->Add(m_lblStatus, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        // row3->Add(m_btnOk,     0, wxALIGN_CENTER_VERTICAL);

        // // Add rows to main sizer with margins
        // mainSizer->Add(row1, 0, wxEXPAND | wxLEFT | wxRIGHT, 40);
        // mainSizer->Add(row2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
        // mainSizer->Add(row3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

        // SetSizerAndFit(mainSizer);

        // // Optional: enforce a reasonable minimum so resizing behaves nicely
        // //SetMinSize(GetSize());
        
        // CentreOnScreen();
    }
};

class DeclarativeApp : public wxApp
{
public:
    bool OnInit() override
    {
        auto* dlg = new MainDialog(nullptr, wxID_ANY, "Declarative UI Dialog");
        dlg->Show();
        return true;
    }
};

// Use explicit main to satisfy environments that expect it while keeping wxApp wiring minimal.
wxIMPLEMENT_APP_NO_MAIN(DeclarativeApp);

int main(int argc, char** argv)
{
    if(!wxEntryStart(argc, argv))
        return -1;

    wxTheApp->CallOnInit();
    int code = wxTheApp->OnRun();
    wxTheApp->OnExit();
    wxEntryCleanup();
    return code;
}
