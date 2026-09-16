#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // T3.1 demo state: locals of main, so the bound refs outlive the modeless
    // dialogs that read them for the whole of exec().
    std::string openPath = "examples/main_qt.cpp";
    std::string savePath;
    std::string folderPath;
    std::string lastDialogResult = "(nothing picked yet)";
    bool pickersDisabled = false;
    std::string sharedPath = "include/widgets.hpp";
    bool boundPickersDisabled = false;

    // The pickers gallery: FilePicker in all three modes and the one-shot
    // FileDialog. On Qt the picker is a QLineEdit + QToolButton composite --
    // there is no native picker control -- and Browse runs the BLOCKING
    // QFileDialog, so the control flow matches wx exactly.
    drawPickersGalleryUI(openPath, savePath, folderPath, lastDialogResult,
        pickersDisabled).show();
    // The binding demo: two FilePickers and a TextCtrl over one string. Pick in
    // either picker, or type in the field, and the other two follow.
    drawFilePickerBinding(sharedPath, boundPickersDisabled).show();

    return app.exec();
}
