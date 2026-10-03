#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // EditableCombo: the gallery (free text vs the list-only ComboBox) and a
    // shared text with a bound, growing list of suggestions.
    std::string m_comboFont = "Arial";
    std::string m_comboCity;
    std::string m_comboSize = "Medium";
    std::string m_comboStatus = "Pick or type";
    DemoComboBinding m_comboBinding;

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawEditableComboGalleryUI(m_comboFont, m_comboCity, m_comboSize, m_comboStatus).show();
        drawEditableComboBindingUI(m_comboBinding).show();
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
