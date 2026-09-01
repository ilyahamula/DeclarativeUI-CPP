#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Container-disabling demo state: members, so the bound refs outlive the
    // modeless dialogs that read them.
    std::string m_deliveryName = "Ada Lovelace";
    std::string m_deliveryEmail = "ada@example.com";
    int m_copies = 1;
    bool m_express = false;
    bool m_groupDisabled = false;

    std::string m_tabNote2 = "Notes...";
    int m_level = 3;
    bool m_verboseLogging = false;
    bool m_pageDisabled = false;
    bool m_panelDisabled = false;

    // Container value-binding demo state.
    int m_boundCount = 40;
    bool m_countLocked = false;

public:
    bool OnInit() override
    {
        drawGroupBoxDisabling(m_deliveryName, m_deliveryEmail, m_copies, m_express, m_groupDisabled).show();
        drawStackAndTabDisabling(m_tabNote2, m_level, m_verboseLogging, m_pageDisabled, m_panelDisabled).show();
        drawGroupDisabledByCheckBox(m_boundCount, m_countLocked).show();
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
