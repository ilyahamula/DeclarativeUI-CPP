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

    // setPosition places where the window opens; the user can still move it
    // afterwards. On ImGui these are host-window-relative, on wx and Qt
    // desktop-absolute -- the two dialogs land side by side either way.
    drawExpanderUI(basicOpen, advancedOpen, networkOpen, logging,
        level, retries, proxy, sectionsDisabled).setPosition({ 40, 40 }).show();
    drawExpanderBinding(detailsOpen, bindingDisabled).setPosition({ 500, 40 }).show();

    return app.exec();
}
