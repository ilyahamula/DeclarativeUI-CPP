#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // T3.2 demo state: members, so the bound refs outlive the modeless dialogs
    // that read them.
    std::string m_openPath = "examples/main_wx.cpp";
    std::string m_savePath;
    std::string m_folderPath;
    std::string m_lastDialogResult = "(nothing picked yet)";
    bool m_pickersDisabled = false;
    std::vector<std::string> m_enabledPlugins { "Formatter", "Debugger" };
    std::string m_enabledSummary = pluginSummary(m_enabledPlugins);
    std::vector<std::string> m_modules { "Core", "Storage" };
    bool m_moduleListsDisabled = false;
    // T3.3: both start EMPTY, which is the state a placeholder is for.
    std::string m_searchTerm;
    std::string m_apiKey;

public:
    bool OnInit() override
    {
        // The pickers gallery, now carrying T3.2's CheckListBox: on wx a native
        // wxCheckListBox, whose Check() setter deliberately fires no event, so
        // the ref-sync push cannot re-enter the user's onChange.
        drawPickersGalleryUI(m_openPath, m_savePath, m_folderPath, m_lastDialogResult,
            m_pickersDisabled, m_enabledPlugins, m_enabledSummary, m_searchTerm, m_apiKey).show();
        // The binding demo: two CheckListBoxes and a ListBox over one
        // std::vector<std::string>.
        drawCheckListBinding(m_modules, m_moduleListsDisabled).show();
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
