#include "demo/all_demos.hpp"
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    // Table demo state: members, so the bound refs outlive the modeless dialogs
    // that read them.
    // The Notes column is editable, which is the other half of why these are
    // refs: an edit has to land somewhere the widgets do not own.
    TableRows m_files {
        { "main.cpp",      "12 KB", "entry point" },
        { "layout.cpp",    "48 KB", "measure/arrange" },
        { "measure.cpp",   "9 KB",  "" },
        { "widgets.hpp",   "31 KB", "public API" },
        { "stacks.hpp",    "7 KB",  "" },
        { "buildable.hpp", "3 KB",  "concepts" },
    };
    int m_selectedRow = 0;
    std::vector<int> m_checkedRows { 1, 3 };
    bool m_tablesDisabled = false;

    // The same widget bound the other way: one rows vector and one key shared by
    // two tables.
    TableRows m_sharedFiles = m_files;
    std::string m_sharedFile = "layout.cpp";
    bool m_mirrorDisabled = false;

public:
    bool OnInit() override
    {
        drawTableUI(m_files, m_selectedRow, m_checkedRows, m_tablesDisabled).show();
        drawTableMirror(m_sharedFiles, m_sharedFile, m_mirrorDisabled).show();
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
