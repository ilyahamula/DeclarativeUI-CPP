#include "basic_demo.hpp"
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
public:
    bool OnInit() override
    {
        drawUI(m_text, m_sliderValue, m_sliderValue1, m_radioChoice, m_radioChoice1, m_selectedCombo, m_checked, [this]() {
#ifdef USE_LOGGER
            wxMessageBox(Logger::instance().getAll(), "Info", wxOK | wxICON_INFORMATION);
#else
            wxMessageBox(
                "text: " + m_text + "\n"
                "sliderValue: " + std::to_string(m_sliderValue) + "\n"
                "sliderValue1: " + std::to_string(m_sliderValue1) + "\n"
                "radioChoice: " + std::to_string(m_radioChoice) + "\n"
                "radioChoice1: " + std::to_string(m_radioChoice1) + "\n"
                "selectedCombo: " + m_selectedCombo + "\n"
                "checked: " + std::string(m_checked ? "true" : "false"),
                "State", wxOK | wxICON_INFORMATION);
#endif
        }).show();
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
