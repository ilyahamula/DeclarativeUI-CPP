#include <wx/wx.h>

#include "declarative_ui.hpp"

class MainDialog : public wxDialog
{
public:
    MainDialog(wxWindow* parent, wxWindowID id, const wxString& title)
        : wxDialog(parent, id, title, wxDefaultPosition, wxSize(400, 200))
    {
        VSizer {
            wxSizerFlags().Border(),
            HSizer {
                Button{"Action"},
                TextCtrl{""}
                    .withFlags(wxSizerFlags().Expand())
            },
            Slider { {0, 100}, 50 }
                .withFlags(wxEXPAND),
            HSizer {
                StaticText{"Status:"},
                Button{"Close"}
            }
        }.fitTo(this);
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
