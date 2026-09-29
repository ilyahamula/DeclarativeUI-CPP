#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Demo state: locals of main, so the bound refs outlive the modeless
    // windows that read them for the whole of exec().
    //
    // The application shell: its menu, tool bar and context menu open their
    // dialogs through ShowAction(), and `renameOpen` is the one flag of theirs
    // the caller owns (About's is keyed by title inside the framework).
    std::string selectedFile = "main.cpp";
    TableRows files {
        { "main.cpp",   "2 KB",  "entry point" },
        { "layout.cpp", "31 KB", "measure/arrange" },
        { "widgets.hpp", "18 KB", "public API" },
        { "engine.hpp", "9 KB",  "" },
    };
    int selectedRow = 0;
    std::string notes = "Try Help > About, the About tool, or right-click a file > Rename...";
    bool shellDisabled = false;
    bool shellOpen = true;
    std::string shellStatus = "(the shell is still open)";
    bool wordWrap = true;
    bool renameOpen = false;
    // The feedback form: Button::onClickShow(), the Button spelling of the
    // same thing. The `open` bool is the single truth about whether the modal
    // is up; the counter is what makes onClose()'s "exactly once" readable.
    std::string deleteFile = "main.cpp";
    std::string deleteAnswer = "(no answer yet)";
    std::string confirmReport = "(the modal has not been closed yet)";
    int confirmCloseCount = 0;
    bool confirmOpen = false;
    bool deleteFormDisabled = false;

    // One show() per open on a retained backend. The dialogs the shell opens
    // are shown from its menu, tool and context-menu callbacks.
    drawAppShellUI(selectedFile, files, selectedRow, notes, shellDisabled,
        shellOpen, shellStatus, wordWrap, renameOpen).show(shellOpen);
    // A form that asks and an application-modal box that answers, opened by
    // onClickShow(). Qt gives the box Qt::ApplicationModal and then show() --
    // never exec() -- so the call still returns immediately.
    drawDeleteFormUI(deleteFile, deleteAnswer, confirmReport, deleteFormDisabled,
        confirmOpen, confirmCloseCount).show();

    return app.exec();
}
