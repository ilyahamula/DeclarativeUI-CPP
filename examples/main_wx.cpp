#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // Calendar: Monday-first and Sunday-first side by side, and one date
    // shared by a calendar and a DatePicker.
    Date m_isoDate = todayDate();
    Date m_usDate = todayDate();
    std::string m_calendarStatus = "Pick a day";
    Date m_sharedDate = todayDate();
    bool m_calendarDisabled = false;

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawCalendarGalleryUI(m_isoDate, m_usDate, m_calendarStatus).show();
        drawCalendarBindingUI(m_sharedDate, m_calendarDisabled).show();
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
