#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // TreeView demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    std::string selectedFile = "src/main.cpp";
    std::vector<std::string> selectedModules { "include" };
    bool treesDisabled = false;
    drawTreeUI(selectedFile, selectedModules, treesDisabled).show();

    // The same widget bound the other way: one path shared by two trees.
    std::string sharedPath = "src/engine";
    bool mirrorDisabled = false;
    drawTreeViewMirror(sharedPath, mirrorDisabled).show();

    return app.exec();
}
