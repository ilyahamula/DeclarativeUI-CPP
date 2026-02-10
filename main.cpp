#include <wx/wx.h>

#include "declarative_ui.hpp"

class MainDialog : public wxDialog
{
public:
    MainDialog(wxWindow* parent, wxWindowID id, const wxString& title)
        : wxDialog(parent, id, title, wxDefaultPosition, wxSize(400, 200))
    {
        VStack {
            LayoutFlags().Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM | wxTOP, 10),
            HStack {
                Button{"Browse..."}
                    .withFlags(LayoutFlags().CenterVertical().Border(wxRIGHT, 15))
                    .onClick([]() {
                        wxMessageBox("Browse button clicked!");
                    }),
                TextCtrl{""}
                    .withFlags(LayoutFlags(1).Expand())
            },
            HGroupBox {
                Slider { { .min = 0, .max = 100, .value = 50 } }
                    .withFlags(LayoutFlags(1).Expand()),
                VGroupBox {
                    LayoutFlags().CenterVertical().Border(wxLEFT),
                    RadioButton{"On"}.withStyle(wxRB_GROUP),
                    RadioButton{"Off"}
                }
            },
            HStack {
                StaticText{"Ready"}
                    .withFlags(LayoutFlags(1).CenterVertical().Border(wxRIGHT)),
                Button{"OK"}
                    .withFlags(LayoutFlags().CenterVertical())
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
