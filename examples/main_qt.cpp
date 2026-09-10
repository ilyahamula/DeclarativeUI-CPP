#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Expander demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    bool basicOpen = true;
    bool advancedOpen = false;
    bool networkOpen = true;
    bool logging = true;
    int level = 2;
    int retries = 3;
    std::string proxy = "proxy.local:8080";
    bool sectionsDisabled = false;
    bool detailsOpen = false;
    bool bindingDisabled = false;

    drawExpanderUI(basicOpen, advancedOpen, networkOpen, logging,
        level, retries, proxy, sectionsDisabled).show();
    drawExpanderBinding(detailsOpen, bindingDisabled).show();

    return app.exec();
}
