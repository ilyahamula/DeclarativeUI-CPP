#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Window demo state: locals of main, so the bound refs outlive the modeless
    // windows that read them for the whole of exec().
    std::string selectedFile = "main.cpp";
    TableRows files {
        { "main.cpp",    "2 KB",  "entry point" },
        { "layout.cpp",  "31 KB", "measure/arrange" },
        { "widgets.hpp", "18 KB", "public API" },
        { "engine.hpp",  "9 KB",  "" },
    };
    int selectedRow = 0;
    std::string notes = "Drag a sash, then resize the window: the panes take the room.";
    bool shellDisabled = false;
    bool shellOpen = true;
    std::string shellStatus = "(the shell is still open)";
    bool panelDisabled = false;

    // The shell is a Window -- a QMainWindow, so T2.2's menu bar has somewhere
    // to attach. Resizable by default, shown against a bool that its own Close
    // button and the control panel both write.
    drawAppShellUI(selectedFile, files, selectedRow, notes, shellDisabled,
        shellOpen, shellStatus).show(shellOpen);
    // Fixed(), and shown with the plain show() -- the panel stays up so the
    // shell's onClose() report is readable after the shell has gone, and so the
    // app does not exit with its last window.
    drawWindowBinding(shellOpen, shellStatus, panelDisabled).show();

    return app.exec();
}
