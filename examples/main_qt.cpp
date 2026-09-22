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
    std::vector<std::string> modules { "Core", "Storage" };
    bool moduleListsDisabled = false;
    // T3.3: both start EMPTY, which is the state a placeholder is for.
    std::string searchTerm;
    std::string apiKey;

    // The pickers gallery, now carrying T3.2's CheckListBox: on Qt a
    // QListWidget whose items are ItemIsUserCheckable, with itemChanged
    // connected only after population.
    drawPickersGalleryUI(openPath, savePath, folderPath, lastDialogResult,
        pickersDisabled, enabledPlugins, enabledSummary, searchTerm, apiKey).show();
    // The binding demo: two CheckListBoxes and a ListBox over one
    // std::vector<std::string>. Tick in either list and the others follow.
    drawCheckListBinding(modules, moduleListsDisabled).show();

    return app.exec();
}
