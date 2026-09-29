#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Demo state: locals of main, so the bound refs outlive the modeless
    // windows that read them for the whole of exec().
    //
    // Background work & stable ids: a worker thread fills the progress bar
    // through postToUi(), and an Expander above two unbound check boxes shows
    // what withId() keeps on ImGui.
    float workProgress = 0.0f;
    std::string workStatus = "Idle";
    bool workBusy = false;
    bool workDetailsOpen = false;
    // The radio/combo binding: three radios and a combo on one int. Each radio
    // names its own value, so the group is the shared int -- not declaration
    // order -- and `choiceLocked` disables the lot.
    int choice = 1;
    bool choiceLocked = false;

    // One show() per open on a retained backend.
    drawWorkerAndIdentityUI(workProgress, workStatus, workBusy, workDetailsOpen).show();
    drawChoiceMirror(choice, choiceLocked).show();

    return app.exec();
}
