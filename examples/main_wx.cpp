#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // Window demo state: members, so the bound refs outlive the modeless
    // windows that read them.
    std::string m_selectedFile = "main.cpp";
    TableRows m_files {
        { "main.cpp",    "2 KB",  "entry point" },
        { "layout.cpp",  "31 KB", "measure/arrange" },
        { "widgets.hpp", "18 KB", "public API" },
        { "engine.hpp",  "9 KB",  "" },
    };
    int m_selectedRow = 0;
    std::string m_notes = "Drag a sash, then resize the window: the panes take the room.";
    bool m_shellDisabled = false;
    bool m_shellOpen = true;
    std::string m_shellStatus = "(the shell is still open)";
    bool m_panelDisabled = false;
    bool m_wordWrap = true;

public:
    bool OnInit() override
    {
        // The shell is a Window -- a wxFrame, which is the thing a wxDialog
        // cannot be, and the only thing a wxMenuBar attaches to. Resizable by
        // default, shown against a bool that its own Close button, its File >
        // Close item and the control panel all write.
        drawAppShellUI(m_selectedFile, m_files, m_selectedRow, m_notes, m_shellDisabled,
            m_shellOpen, m_shellStatus, m_wordWrap).show(m_shellOpen);
        // Fixed(), and shown with the plain show() -- the panel stays up so the
        // shell's onClose() report is readable after the shell has gone, and so
        // the app does not exit with its last top-level window.
        drawWindowBinding(m_shellOpen, m_shellStatus, m_panelDisabled, m_wordWrap).show();
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
