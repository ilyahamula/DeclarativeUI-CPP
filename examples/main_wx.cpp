#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Expander demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    bool m_basicOpen = true;
    bool m_advancedOpen = false;
    bool m_networkOpen = true;
    bool m_logging = true;
    int m_level = 2;
    int m_retries = 3;
    std::string m_proxy = "proxy.local:8080";
    bool m_sectionsDisabled = false;
    bool m_detailsOpen = false;
    bool m_bindingDisabled = false;

public:
    bool OnInit() override
    {
        // setPosition places where the window opens; the user can still move
        // it afterwards. On ImGui these are host-window-relative, on wx and Qt
        // desktop-absolute -- the two dialogs land side by side either way.
        drawExpanderUI(m_basicOpen, m_advancedOpen, m_networkOpen, m_logging,
            m_level, m_retries, m_proxy, m_sectionsDisabled).setPosition({ 40, 40 }).show();
        drawExpanderBinding(m_detailsOpen, m_bindingDisabled).setPosition({ 500, 40 }).show();
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
