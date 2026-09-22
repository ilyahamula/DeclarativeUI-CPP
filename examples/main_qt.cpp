#include "demo/all_demos.hpp"

#include <QApplication>

#include <functional>
#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // T3.6 demo state: locals of main, so the bound refs outlive the modeless
    // dialogs that read them for the whole of exec(). The two `open` bools are
    // the single truth about whether their dialog is up; the two counters are
    // what make onClose()'s "exactly once" readable.
    std::string deleteFile = "main.cpp";
    std::string deleteAnswer = "(no answer yet)";
    std::string confirmReport = "(the modal has not been closed yet)";
    int confirmCloseCount = 0;
    bool confirmOpen = false;
    bool deleteFormDisabled = false;
    bool detailsOpen = false;
    std::string detailsNote = "Shared by the panel and the dialog.";
    std::string detailsReport = "(the dialog has not been closed yet)";
    int detailsCloseCount = 0;
    bool panelDisabled = false;
    // Controls-gallery state, for context beside the new dialogs.
    std::string multilineText = "Type something here...";
    std::string galleryPassword;
    int spinInt = 42;
    float spinFloat = 1.5f;
    Date date { .year = 2026, .month = 9, .day = 22 };
    Time time { .hour = 9, .minute = 30, .second = 0 };
    bool toggle = false;
    float galleryProgress = 0.35f;
    std::string tabNote = "Add notes here...";
    bool tabLogging = false;
    Color themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    // The one piece of per-backend wiring T3.6 needs, and the whole of it.
    // A QDialog is destroyed when it closes and its engine session goes with it,
    // so on a retained backend OPENING one is a show() call, not merely a flag:
    // these run from a button click and from the binding panel's check box,
    // which is long after the engine's measure pass and as safe as any other
    // event handler. (The ImGui main sets the flag and nothing else -- there
    // show() is the frame.)
    std::function<void()> openConfirm = [&] {
        confirmOpen = true;
        drawConfirmDeleteUI(confirmOpen, deleteFile, deleteAnswer, confirmReport,
            confirmCloseCount).show(confirmOpen);
    };
    std::function<void()> openDetails = [&] {
        detailsOpen = true;
        drawDetailsUI(detailsOpen, detailsNote, detailsReport, detailsCloseCount)
            .show(detailsOpen);
    };

    // T3.6's gallery: a form that asks, and an application-modal box that
    // answers. Qt gives the box Qt::ApplicationModal and then show() -- never
    // exec() -- so Modal() locks the rest of the application out while the call
    // still returns immediately.
    drawDeleteFormUI(deleteFile, deleteAnswer, confirmReport, deleteFormDisabled,
        openConfirm).show();
    // The binding demo: a dialog's open flag as an ordinary bound bool, shared
    // with a check box and a toggle in this panel. Unticking either one closes
    // the dialog; ticking it opens a fresh one.
    drawDialogBinding(detailsOpen, detailsNote, detailsReport, panelDisabled,
        openDetails).show();
    // The controls gallery, for context beside the new dialogs.
    drawControlsUI(multilineText, galleryPassword, spinInt, spinFloat, date, time,
        toggle, galleryProgress, tabNote, tabLogging, themeColor).show();

    return app.exec();
}
