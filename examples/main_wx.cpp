#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // The RichText panel: the links write these, the fields read them back,
    // and `m_richLocked` disables both texts at once.
    std::string m_richLastLink = "(no link clicked yet)";
    int m_richLinkClicks = 0;
    bool m_richLocked = false;
    // The radio/combo binding: three radios and a combo on one int. Each radio
    // names its own value, so the group is the shared int -- not declaration
    // order -- and `choiceLocked` disables the lot.
    int m_choice = 1;
    bool m_choiceLocked = false;

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend. RichText is an owner-drawn
        // wxPanel painting the shared layout.
        drawRichTextBinding(m_richLastLink, m_richLinkClicks, m_richLocked).show();
        drawChoiceMirror(m_choice, m_choiceLocked).show();
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
