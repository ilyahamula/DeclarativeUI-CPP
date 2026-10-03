#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // SearchField: the gallery (default and custom placeholders, Enter that
    // searches instead of pressing the default button) and a live filter.
    std::string m_quickSearch;
    std::string m_fileSearch;
    std::string m_docTitle = "Untitled";
    std::string m_searchStatus = "Type in the file search";
    DemoSearchBinding m_searchBinding;

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawSearchGalleryUI(m_quickSearch, m_fileSearch, m_docTitle, m_searchStatus).show();
        drawSearchBindingUI(m_searchBinding).show();
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
