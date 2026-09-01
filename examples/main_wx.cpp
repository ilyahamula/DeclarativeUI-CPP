#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // TreeView demo state: members, so the bound refs outlive the modeless
    // dialogs that read them.
    std::string m_selectedFile = "src/main.cpp";
    std::vector<std::string> m_selectedModules { "include" };
    bool m_treesDisabled = false;

    // The same widget bound the other way: one path shared by two trees.
    std::string m_sharedPath = "src/engine";
    bool m_mirrorDisabled = false;

public:
    bool OnInit() override
    {
        drawTreeUI(m_selectedFile, m_selectedModules, m_treesDisabled).show();
        drawTreeViewMirror(m_sharedPath, m_mirrorDisabled).show();
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
