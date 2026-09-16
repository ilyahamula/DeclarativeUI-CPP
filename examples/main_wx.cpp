#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>

class DeclarativeApp : public wxApp
{
    // T3.1 demo state: members, so the bound refs outlive the modeless dialogs
    // that read them.
    std::string m_openPath = "examples/main_wx.cpp";
    std::string m_savePath;
    std::string m_folderPath;
    std::string m_lastDialogResult = "(nothing picked yet)";
    bool m_pickersDisabled = false;
    std::string m_sharedPath = "include/widgets.hpp";
    bool m_boundPickersDisabled = false;

public:
    bool OnInit() override
    {
        // The pickers gallery: FilePicker in all three modes and the one-shot
        // FileDialog. wx is the one backend with a native picker control
        // (wxFilePickerCtrl / wxDirPickerCtrl), asked for with USE_TEXTCTRL so
        // a typed path commits exactly as a picked one does.
        drawPickersGalleryUI(m_openPath, m_savePath, m_folderPath, m_lastDialogResult,
            m_pickersDisabled).show();
        // The binding demo: two FilePickers and a TextCtrl over one string.
        drawFilePickerBinding(m_sharedPath, m_boundPickersDisabled).show();
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
