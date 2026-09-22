#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // T3.5 demo state: members, so the bound refs outlive the modeless dialogs
    // that read them. The scale/align dialog needs one flag -- neither a
    // picture nor a label has a value of its own to share, so isDisabled() is
    // the binding those two read-only widgets can offer. The Account form's
    // three strings are what make its right-aligned labels a real form rather
    // than a ruler.
    bool m_displaysDisabled = false;
    std::string m_accountName = "Ada Lovelace";
    std::string m_accountEmail = "ada@example.com";
    std::string m_accountPassword = "analytical";
    bool m_accountLocked = false;
    // Controls-gallery state, for the Preview picture that now carries Fit.
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

public:
    bool OnInit() override
    {
        // T3.5's gallery: one file in one frame under all four scale modes,
        // and one band under all three alignments. wx is the backend that
        // composes the picture on wxEVT_SIZE -- a wxStaticBitmap draws its
        // bitmap at the top-left and clips, so it can do Stretch and no more.
        drawScaleAndAlignUI(m_displaysDisabled).show();
        // withAlign() in a real form: the labels stretch across column 0 of
        // the Grid, so their colons line up against the fields.
        drawAccountFormUI(m_accountName, m_accountEmail, m_accountPassword, m_accountLocked).show();
        // The controls gallery, whose Preview picture now carries Fit.
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
