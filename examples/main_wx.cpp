#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    std::string m_text = "Initial text";
    float m_sliderValue = 50.5f;
    int m_sliderValue1 = 25;
    int m_radioChoice1 = 0;
    int m_radioChoice = 0;
    std::string m_selectedCombo = "Goodbye";
    bool m_checked = false;
    bool m_toggle = false;

    std::string m_multilineText = "Type something here...";
    std::string m_password;
    int m_spinInt = 42;
    float m_spinFloat = 1.5f;
    Date m_date { .year = 2026, .month = 2, .day = 22 };
    Time m_time { .hour = 9, .minute = 30, .second = 0 };
    float m_progress = 0.35f;
    std::string m_tabNote = "Add notes here...";
    bool m_tabLogging = false;
    Color m_themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    float value = 1.0f;
    bool checked = false;

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

    void DrawUI()
    {
        drawControlsUI(m_multilineText, m_password, m_spinInt,
            m_spinFloat, m_date, m_time, m_toggle, m_progress, m_tabNote, m_tabLogging, m_themeColor, [this]() {
            MessageBox("Controls State",
                "multilineText: " + m_multilineText + "\n"
                "password: " + m_password + "\n"
                "spinInt: " + std::to_string(m_spinInt) + "\n"
                "spinFloat: " + std::to_string(m_spinFloat) + "\n"
                "date: " + std::to_string(m_date.year) + "-"
                         + std::to_string(m_date.month) + "-"
                         + std::to_string(m_date.day) + "\n"
                "time: " + std::to_string(m_time.hour) + ":"
                         + std::to_string(m_time.minute) + ":"
                         + std::to_string(m_time.second))
                .withStyle(MessageBoxStyle::Info)
                .withButtons(MessageBoxButtons::OK)
                .show();
        },
        [this]() {
            MessageBox("Question", "Do you like this framework?")
                .withStyle(MessageBoxStyle::Question)
                .withButtons(MessageBoxButtons::YesNo)
                .show();
        }).show();
    }

public:
    bool OnInit() override
    {
        Bind(wxEVT_IDLE, &DeclarativeApp::OnIdle, this);
        drawGroupBoxDisabling(m_deliveryName, m_deliveryEmail, m_copies, m_express, m_groupDisabled).show();
        drawStackAndTabDisabling(m_tabNote2, m_level, m_verboseLogging, m_pageDisabled, m_panelDisabled).show();
        drawGroupDisabledByCheckBox(m_boundCount, m_countLocked).show();
        return true;
    }

    void OnIdle(wxIdleEvent& event)
    {
        wxApp::OnIdle(event);
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
