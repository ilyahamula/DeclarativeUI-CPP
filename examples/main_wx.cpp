#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Text gallery state: members, so the bound refs outlive the modeless
    // dialog that reads them.
    std::string m_editableText = "Type here -- this one is editable.";
    bool m_fieldsDisabled = false;

public:
    bool OnInit() override
    {
        drawTextUI(m_editableText, m_fieldsDisabled).show();
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
