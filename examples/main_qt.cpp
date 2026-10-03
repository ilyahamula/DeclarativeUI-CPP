#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>
#include <vector>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Demo state: locals of main, so the bound refs outlive the modeless
    // windows that read them for the whole of exec().
    //
    // SearchField: the gallery (default and custom placeholders, Enter that
    // searches instead of pressing the default button) and a live filter.
    std::string quickSearch;
    std::string fileSearch;
    std::string docTitle = "Untitled";
    std::string searchStatus = "Type in the file search";
    DemoSearchBinding searchBinding;

    // One show() per open on a retained backend.
    drawSearchGalleryUI(quickSearch, fileSearch, docTitle, searchStatus).show();
    drawSearchBindingUI(searchBinding).show();

    return app.exec();
}
