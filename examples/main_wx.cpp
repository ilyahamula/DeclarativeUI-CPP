#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // Spinner: the gallery (default, sized, logo, stopped) and a running
    // flag shared by two spinners, a CheckBox and a ToggleButton.
    bool m_spinnerBusy = true;
    std::string m_spinnerStatus = "Working...";

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawSpinnerGalleryUI().show();
        drawSpinnerBindingUI(m_spinnerBusy, m_spinnerStatus).show();
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
