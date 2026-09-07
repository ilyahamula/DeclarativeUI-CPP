#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Grid demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    std::string m_name = "Ada Lovelace";
    std::string m_email = "ada@example.com";
    std::string m_password = "secret";
    bool m_formDisabled = false;
    std::string m_shared = "shared";
    bool m_gridDisabled = false;

public:
    bool OnInit() override
    {
        drawAccountFormUI(m_name, m_email, m_password, m_formDisabled).show();
        drawGridMirror(m_shared, m_gridDisabled).show();
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
