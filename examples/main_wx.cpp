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
    // T3.3: both start EMPTY, which is the state a placeholder is for.
    std::string m_searchTerm;
    std::string m_apiKey;
    // T3.4: the shared float and the two halves of the busy flag. `m_idle` is
    // the complement isDisabled() cannot express -- it binds a bool, not `!bool`.
    float m_progress = 35.0f;
    bool m_busy = false;
    bool m_idle = true;

public:
    bool OnInit() override
    {
        // T3.4's gallery: a determinate bar next to a busy one. wx is the one
        // backend that needs a clock of its own for the busy half, since wxGTK
        // advances the marquee once per Pulse() call.
        drawIndeterminateProgressUI().show();
        // The pickers gallery it was split out of, for the surrounding context.
        drawPickersGalleryUI(m_openPath, m_savePath, m_folderPath, m_lastDialogResult,
            m_pickersDisabled, m_enabledPlugins, m_enabledSummary, m_searchTerm, m_apiKey).show();
        // The binding demo: a determinate bar sharing a float with a slider,
        // next to a valueless one, with a checkbox picking which half is live.
        drawIndeterminateProgressBinding(m_progress, m_busy, m_idle).show();
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
