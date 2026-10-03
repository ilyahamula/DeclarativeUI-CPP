#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // VirtualList: a million generated log lines beside a ListBox. Custom
    // composites: user structs/classes standing for a group of widgets.
    int m_logCount = 1000000;
    int m_logRow = -1;
    std::string m_languagePick = "C++";
    std::string m_listStatus = "Pick a log line";
    DemoCustomComposites m_composites;

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawVirtualListGalleryUI(m_logCount, m_logRow, m_languagePick, m_listStatus).show();
        drawCustomCompositesUI(m_composites).show();
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
