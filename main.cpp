#include "declarative_ui.hpp"

#ifdef USE_WX
#include <wx/wx.h>

namespace
{
    auto drawUI()
    {
        return VStack {
            LayoutFlags().Expand().Border(Side::All, 10),
            HStack {
                Button{"Browse..."}
                    .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 15))
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
                    LayoutFlags().CenterVertical().Border(Side::Left),
                    RadioButton{"On"}.withStyle(wxRB_GROUP),
                    RadioButton{"Off"}
                }
            },
            HStack {
                StaticText{"Ready"}
                    .withFlags(LayoutFlags(1).CenterVertical().Border(Side::Right)),
                Button{"OK"}
                    .withFlags(LayoutFlags().CenterVertical())
            },
            HGroupBox { "Combo Group",
                ComboBox{ {"Hello", "Goodbye", "Nihao" }, "G" }
                    .withFlags(LayoutFlags(1).CenterVertical().Border(Side::Right)),
                CheckBox{}
                    .withFlags(LayoutFlags().CenterVertical())
            }
        };
    }
}

class MainDialog : public wxDialog
{
public:
    MainDialog(wxWindow* parent, wxWindowID id, const wxString& title)
        : wxDialog(parent, id, title, wxDefaultPosition, wxSize(400, 200))
    {
        drawUI().fitTo(this);
    }
};

class DeclarativeApp : public wxApp
{
public:
    bool OnInit() override
    {
        //auto* dlg = new MainDialog(nullptr, wxID_ANY, "Declarative UI Dialog");
        //dlg->Show();
        auto dlg = new wxDialog(nullptr, wxID_ANY, "Declarative UI Dialog", wxDefaultPosition, wxSize(400, 200));
        drawUI().fitTo(dlg);
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
#elif defined(USE_QT)
int main(int argc, char** argv)
{
    return 0;
}
#elif defined(USE_IMGUI)
int main(int argc, char** argv)
{
    return 0;
}
#endif
