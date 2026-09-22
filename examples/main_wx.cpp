#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // T3.6 demo state: members, so the bound refs outlive the modeless dialogs
    // that read them. The two `open` bools are the single truth about whether
    // their dialog is up; the two counters are what make onClose()'s "exactly
    // once" readable.
    std::string m_deleteFile = "main.cpp";
    std::string m_deleteAnswer = "(no answer yet)";
    std::string m_confirmReport = "(the modal has not been closed yet)";
    int m_confirmCloseCount = 0;
    bool m_confirmOpen = false;
    bool m_deleteFormDisabled = false;
    bool m_detailsOpen = false;
    std::string m_detailsNote = "Shared by the panel and the dialog.";
    std::string m_detailsReport = "(the dialog has not been closed yet)";
    int m_detailsCloseCount = 0;
    bool m_panelDisabled = false;
    // Controls-gallery state, for context beside the new dialogs.
    std::string m_multilineText = "Type something here...";
    std::string m_galleryPassword;
    int m_spinInt = 42;
    float m_spinFloat = 1.5f;
    Date m_date { .year = 2026, .month = 9, .day = 22 };
    Time m_time { .hour = 9, .minute = 30, .second = 0 };
    bool m_toggle = false;
    float m_galleryProgress = 0.35f;
    std::string m_tabNote = "Add notes here...";
    bool m_tabLogging = false;
    Color m_themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    // The one piece of per-backend wiring T3.6 needs, and the whole of it.
    // A wxDialog is destroyed when it closes and its engine session goes with
    // it, so on a retained backend OPENING one is a show() call, not merely a
    // flag: these run from a button click and from the binding panel's check
    // box, which is long after the engine's measure pass and as safe as any
    // other event handler. (The ImGui main sets the flag and nothing else --
    // there show() is the frame.)
    void openConfirm()
    {
        m_confirmOpen = true;
        drawConfirmDeleteUI(m_confirmOpen, m_deleteFile, m_deleteAnswer,
            m_confirmReport, m_confirmCloseCount).show(m_confirmOpen);
    }

    void openDetails()
    {
        m_detailsOpen = true;
        drawDetailsUI(m_detailsOpen, m_detailsNote, m_detailsReport,
            m_detailsCloseCount).show(m_detailsOpen);
    }

public:
    bool OnInit() override
    {
        // T3.6's gallery: a form that asks, and an application-modal box that
        // answers. wx holds a wxWindowDisabler for as long as the box is up, so
        // Modal() locks the rest of the application out while show() still
        // returns immediately.
        drawDeleteFormUI(m_deleteFile, m_deleteAnswer, m_confirmReport,
            m_deleteFormDisabled, [this] { openConfirm(); }).show();
        // The binding demo: a dialog's open flag as an ordinary bound bool,
        // shared with a check box and a toggle in this panel. Unticking either
        // one closes the dialog; ticking it opens a fresh one.
        drawDialogBinding(m_detailsOpen, m_detailsNote, m_detailsReport,
            m_panelDisabled, [this] { openDetails(); }).show();
        // The controls gallery, for context beside the new dialogs.
        drawControlsUI(m_multilineText, m_galleryPassword, m_spinInt, m_spinFloat,
            m_date, m_time, m_toggle, m_galleryProgress, m_tabNote, m_tabLogging,
            m_themeColor).show();
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
