#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Splitter demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    std::string m_selectedFile;
    TableRows m_rows {
        { "layout.cpp",  "18 KB", "engine"   },
        { "measure.cpp", "9 KB",  "engine"   },
        { "splitter.hpp","6 KB",  "new"      },
    };
    int m_selectedRow = -1;
    std::string m_notes = "Notes for the selected file.";
    bool m_shellDisabled = false;
    int m_divider = 200;
    bool m_splittersDisabled = false;

public:
    bool OnInit() override
    {
        drawAppShellUI(m_selectedFile, m_rows, m_selectedRow, m_notes, m_shellDisabled).show();
        drawSplitterBinding(m_divider, m_splittersDisabled).show();
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
