#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Tooltip demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    // `m_tooltipHint` is the live-bound one -- it is a TextCtrl's value in the
    // binding dialog and, at the same time, the hover text of two controls
    // beside it.
    std::string m_multilineText = "Type something here...";
    std::string m_password;
    int m_spinInt = 42;
    float m_spinFloat = 1.5f;
    Date m_date { .year = 2026, .month = 2, .day = 22 };
    Time m_time { .hour = 9, .minute = 30, .second = 0 };
    bool m_toggle = false;
    float m_progress = 0.35f;
    std::string m_tabNote = "Add notes here...";
    bool m_tabLogging = false;
    Color m_themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    std::string m_tooltipHint = "Edit the field above and hover me again.";
    bool m_tooltipDisabled = false;

public:
    bool OnInit() override
    {
        drawControlsUI(m_multilineText, m_password, m_spinInt, m_spinFloat,
            m_date, m_time, m_toggle, m_progress, m_tabNote, m_tabLogging,
            m_themeColor).show();
        drawTooltipBinding(m_tooltipHint, m_tooltipDisabled).show();
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
