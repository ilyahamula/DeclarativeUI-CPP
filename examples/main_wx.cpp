#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // Background work & stable ids: a worker thread fills the progress bar
    // through postToUi(), and an Expander above two unbound check boxes shows
    // what withId() keeps on ImGui.
    float m_workProgress = 0.0f;
    std::string m_workStatus = "Idle";
    bool m_workBusy = false;
    bool m_workDetailsOpen = false;
    bool m_workHideStatusBar = false;
    // The radio/combo binding: three radios and a combo on one int. Each radio
    // names its own value, so the group is the shared int -- not declaration
    // order -- and `choiceLocked` disables the lot.
    int m_choice = 1;
    bool m_choiceLocked = false;
    // The combo's choices, bound: "Add a colour" appends here.
    ItemList m_colours { "Red", "Green", "Blue" };

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawWorkerAndIdentityUI(m_workProgress, m_workStatus, m_workBusy, m_workDetailsOpen, m_workHideStatusBar).show();
        drawChoiceMirror(m_choice, m_choiceLocked, m_colours).show();
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
