#include "demo/all_demos.hpp"
#include <wx/wx.h>

#include <string>
#include <vector>

class DeclarativeApp : public wxApp
{
    // Demo state: members, so the bound refs outlive the modeless windows that
    // read them.
    //
    // The todo list (VForEach): rows follow this vector -- add and remove
    // rows, tick them, and only the changed rows are rebuilt.
    std::vector<DemoTodo> m_todos { { "Write the ForEach demo", true }, { "Try removing a row", false } };
    std::string m_newTodo;
    std::string m_todoKeys = "Enter adds, Escape clears";
    // RadioGroup sharing its index with a slider and a spin box.
    int m_level = 2;
    bool m_levelDisabled = false;
    std::string m_lastPick = "Nothing picked yet";

public:
    bool OnInit() override
    {
        // One show() per open on a retained backend.
        drawTodoListUI(m_todos, m_newTodo, m_todoKeys).show();
        drawRadioGroupMirror(m_level, m_levelDisabled, m_lastPick).show();
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
