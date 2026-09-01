#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Container-disabling demo state: locals of main, so the bound refs
    // outlive the modeless dialogs that read them for the whole of exec().
    std::string deliveryName = "Ada Lovelace";
    std::string deliveryEmail = "ada@example.com";
    int copies = 1;
    bool express = false;
    bool groupDisabled = false;
    drawGroupBoxDisabling(deliveryName, deliveryEmail, copies, express, groupDisabled).show();

    std::string tabNote = "Notes...";
    int level = 3;
    bool verboseLogging = false;
    bool pageDisabled = false;
    bool panelDisabled = false;
    drawStackAndTabDisabling(tabNote, level, verboseLogging, pageDisabled, panelDisabled).show();

    // Container value-binding demo state.
    int boundCount = 40;
    bool countLocked = false;
    drawGroupDisabledByCheckBox(boundCount, countLocked).show();

    return app.exec();
}
