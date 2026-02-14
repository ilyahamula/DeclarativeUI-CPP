#include "declarative_ui.hpp"

namespace
{
    auto drawUI()
    {
        return Dialog {
            "Declarative UI Dialog",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 10),
                HStack {
                    Button{"Browse..."}
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 15))
                        .onClick([]() {
                            //wxMessageBox("Browse button clicked!");
                        }),
                    TextCtrl{""}
                        .withFlags(LayoutFlags(1).Expand())
                },
                HGroupBox {
                    Slider { { .min = 0, .max = 100, .value = 50 } }
                        .withFlags(LayoutFlags(1).Expand()),
                    VGroupBox {
                        LayoutFlags().CenterVertical().Border(Side::Left),
                        RadioButton{"On"}.withStyle(4), // wxRB_GROUP
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
            }
        };
    }
}

#ifdef USE_WX
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
public:
    bool OnInit() override
    {
        drawUI().show();
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
