#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // ScrollPanel demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    bool optionsDisabled = false;
    bool sharedOption = false;
    bool listDisabled = false;

    drawScrollPanelUI(optionsDisabled).show();
    drawScrollPanelBinding(sharedOption, listDisabled).show();

    return app.exec();
}
