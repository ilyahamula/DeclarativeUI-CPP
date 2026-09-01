#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Table demo state: locals of main, so the bound refs outlive the modeless
    // dialogs that read them for the whole of exec().
    // The Notes column is editable, which is the other half of why these are
    // refs: an edit has to land somewhere the widgets do not own.
    TableRows files {
        { "main.cpp",      "12 KB", "entry point" },
        { "layout.cpp",    "48 KB", "measure/arrange" },
        { "measure.cpp",   "9 KB",  "" },
        { "widgets.hpp",   "31 KB", "public API" },
        { "stacks.hpp",    "7 KB",  "" },
        { "buildable.hpp", "3 KB",  "concepts" },
    };
    int selectedRow = 0;
    std::vector<int> checkedRows { 1, 3 };
    bool tablesDisabled = false;
    drawTableUI(files, selectedRow, checkedRows, tablesDisabled).show();

    // The same widget bound the other way: one rows vector and one key shared by
    // two tables.
    TableRows sharedFiles = files;
    std::string sharedFile = "layout.cpp";
    bool mirrorDisabled = false;
    drawTableMirror(sharedFiles, sharedFile, mirrorDisabled).show();

    return app.exec();
}
