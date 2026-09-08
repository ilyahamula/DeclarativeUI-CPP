#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // ScrollPanel demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    bool m_optionsDisabled = false;
    bool m_sharedOption = false;
    bool m_listDisabled = false;

public:
    bool OnInit() override
    {
        drawScrollPanelUI(m_optionsDisabled).show();
        drawScrollPanelBinding(m_sharedOption, m_listDisabled).show();
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
