#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Splitter demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    std::string selectedFile;
    TableRows rows {
        { "layout.cpp",  "18 KB", "engine" },
        { "measure.cpp", "9 KB",  "engine" },
        { "splitter.hpp","6 KB",  "new"    },
    };
    int selectedRow = -1;
    std::string notes = "Notes for the selected file.";
    bool shellDisabled = false;
    int divider = 200;
    bool splittersDisabled = false;

    drawAppShellUI(selectedFile, rows, selectedRow, notes, shellDisabled).show();
    drawSplitterBinding(divider, splittersDisabled).show();

    return app.exec();
}
