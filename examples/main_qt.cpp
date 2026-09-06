#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Spacer demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    bool rowsDisabled = false;
    std::string caption = "shared";
    bool rowLocked = false;

    drawLayoutPrimitivesUI(rowsDisabled).show();
    drawSpacerDisableBinding(caption, rowLocked).show();

    return app.exec();
}
