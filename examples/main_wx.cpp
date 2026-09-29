#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // The toast panel: every value a Toast is built from, each caller-owned.
    // The duration has two holders (a slider and a spin box).
    std::string m_toastMessage = "Settings applied";
    int m_toastStyle = 0;
    int m_toastDurationMs = 2500;
    bool m_toastLocked = false;
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
        // One show() per open on a retained backend. Each toast is a window of
        // its own that is shown without being activated.
        drawToastBinding(m_toastMessage, m_toastStyle, m_toastDurationMs, m_toastLocked).show();
        // A form that asks and an application-modal box that answers, opened
        // by onClickShow(), plus a Save button that answers with a toast. wx
        // holds a wxWindowDisabler for as long as the box is up, while show()
        // still returns immediately.
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
