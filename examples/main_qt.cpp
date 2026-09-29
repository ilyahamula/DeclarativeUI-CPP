#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Demo state: locals of main, so the bound refs outlive the modeless
    // windows that read them for the whole of exec().
    //
    // The RichText panel: the links write these, the fields read them back,
    // and `richLocked` disables both texts at once.
    std::string richLastLink = "(no link clicked yet)";
    int richLinkClicks = 0;
    bool richLocked = false;
    // The radio/combo binding: three radios and a combo on one int. Each radio
    // names its own value, so the group is the shared int -- not declaration
    // order -- and `choiceLocked` disables the lot.
    int choice = 1;
    bool choiceLocked = false;

    // One show() per open on a retained backend. RichText is a plain QWidget
    // that paints the shared layout -- no QLabel, no HTML.
    drawRichTextBinding(richLastLink, richLinkClicks, richLocked).show();
    drawChoiceMirror(choice, choiceLocked).show();

    return app.exec();
}
