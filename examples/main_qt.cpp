#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>
#include <vector>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // T3.2 demo state: locals of main, so the bound refs outlive the modeless
    // dialogs that read them for the whole of exec().
    std::string openPath = "examples/main_qt.cpp";
    std::string savePath;
    std::string folderPath;
    std::string lastDialogResult = "(nothing picked yet)";
    bool pickersDisabled = false;
    std::vector<std::string> enabledPlugins { "Formatter", "Debugger" };
    std::string enabledSummary = pluginSummary(enabledPlugins);
    // T3.3: both start EMPTY, which is the state a placeholder is for.
    std::string searchTerm;
    std::string apiKey;
    // T3.4: the shared float and the two halves of the busy flag. `idle` is
    // the complement isDisabled() cannot express -- it binds a bool, not `!bool`.
    float progress = 35.0f;
    bool busy = false;
    bool idle = true;

    // T3.4's gallery: a determinate bar next to a busy one. On Qt the busy
    // half is an empty range and nothing else -- QProgressBar animates the
    // indicator itself and ignores setValue().
    drawIndeterminateProgressUI().show();
    // The pickers gallery it was split out of, for the surrounding context.
    drawPickersGalleryUI(openPath, savePath, folderPath, lastDialogResult,
        pickersDisabled, enabledPlugins, enabledSummary, searchTerm, apiKey).show();
    // The binding demo: a determinate bar sharing a float with a slider, next
    // to a valueless one, with a checkbox picking which half is live.
    drawIndeterminateProgressBinding(progress, busy, idle).show();

    return app.exec();
}
