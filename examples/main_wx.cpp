#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // The sign-up form: focus (isFocused, onFocus/onBlur) and validity
    // (isInvalid) on the three text fields.
    DemoSignUp m_signUp;
    // Bound TreeView items and Table rows (an Observable, so polled by its
    // change counter): the buttons change these, both controls follow.
    std::vector<TreeItem> m_folders { { "Documents", { { "Invoices" }, { "Letters" } }, true }, { "Pictures", { { "2026" } } } };
    Observable<TableRows> m_files { TableRows { { "readme.txt", "12" }, { "budget.csv", "48" }, { "notes.md", "7" } } };
    std::string m_folderPick = "Documents/Letters";
    std::string m_filePick = "budget.csv";

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawSignUpFormUI(m_signUp).show();
        drawTreeTableBindingUI(m_folders, m_files, m_folderPick, m_filePick).show();
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
