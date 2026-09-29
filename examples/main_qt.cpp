#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Demo state: locals of main, so the bound refs outlive the modeless
    // windows that read them for the whole of exec().
    //
    // The toast panel: every value a Toast is built from, each caller-owned.
    // The duration has two holders (a slider and a spin box).
    std::string toastMessage = "Settings applied";
    int toastStyle = 0;
    int toastDurationMs = 2500;
    bool toastLocked = false;
    // The feedback form: Button::onClickShow(), the Button spelling of the
    // same thing. The `open` bool is the single truth about whether the modal
    // is up; the counter is what makes onClose()'s "exactly once" readable.
    std::string deleteFile = "main.cpp";
    std::string deleteAnswer = "(no answer yet)";
    std::string confirmReport = "(the modal has not been closed yet)";
    int confirmCloseCount = 0;
    bool confirmOpen = false;
    bool deleteFormDisabled = false;

    // One show() per open on a retained backend. Each toast is a frameless
    // Qt::ToolTip window that never takes focus.
    drawToastBinding(toastMessage, toastStyle, toastDurationMs, toastLocked).show();
    // A form that asks and an application-modal box that answers, opened by
    // onClickShow(), plus a Save button that answers with a toast. Qt gives
    // the box Qt::ApplicationModal and then show() -- never exec() -- so the
    // call still returns immediately.
    drawDeleteFormUI(deleteFile, deleteAnswer, confirmReport, deleteFormDisabled,
        confirmOpen, confirmCloseCount).show();

    return app.exec();
}
