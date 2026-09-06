#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Spacer demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    bool m_rowsDisabled = false;
    std::string m_caption = "shared";
    bool m_rowLocked = false;

public:
    bool OnInit() override
    {
        drawLayoutPrimitivesUI(m_rowsDisabled).show();
        drawSpacerDisableBinding(m_caption, m_rowLocked).show();
        return true;
    }
};

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
