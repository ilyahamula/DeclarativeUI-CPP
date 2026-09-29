#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // The application shell: its menu, tool bar and context menu open their
    // dialogs through ShowAction(), and `m_renameOpen` is the one flag of theirs
    // the caller owns (About's is keyed by title inside the framework).
    std::string m_selectedFile = "main.cpp";
    TableRows m_files {
        { "main.cpp",   "2 KB",  "entry point" },
        { "layout.cpp", "31 KB", "measure/arrange" },
        { "widgets.hpp", "18 KB", "public API" },
        { "engine.hpp", "9 KB",  "" },
    };
    int m_selectedRow = 0;
    std::string m_notes = "Try Help > About, the About tool, or right-click a file > Rename...";
    bool m_shellDisabled = false;
    bool m_shellOpen = true;
    std::string m_shellStatus = "(the shell is still open)";
    bool m_wordWrap = true;
    bool m_renameOpen = false;
    // The feedback form: Button::onClickShow(), the Button spelling of the
    // same thing. The `open` bool is the single truth about whether the modal
    // is up; the counter is what makes onClose()'s "exactly once" readable.
    std::string m_deleteFile = "main.cpp";
    std::string m_deleteAnswer = "(no answer yet)";
    std::string m_confirmReport = "(the modal has not been closed yet)";
    int m_confirmCloseCount = 0;
    bool m_confirmOpen = false;
    bool m_deleteFormDisabled = false;

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend. The dialogs the shell
        // opens are shown from its menu, tool and context-menu callbacks.
        drawAppShellUI(m_selectedFile, m_files, m_selectedRow, m_notes, m_shellDisabled,
            m_shellOpen, m_shellStatus, m_wordWrap, m_renameOpen).show(m_shellOpen);
        // A form that asks and an application-modal box that answers, opened
        // by onClickShow(). wx holds a wxWindowDisabler for as long as the box
        // is up, while show() still returns immediately.
        drawDeleteFormUI(m_deleteFile, m_deleteAnswer, m_confirmReport,
            m_deleteFormDisabled, m_confirmOpen, m_confirmCloseCount).show();
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
