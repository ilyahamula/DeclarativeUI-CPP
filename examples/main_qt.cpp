#include "demo/all_demos.hpp"

#include <QApplication>

#include <string>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // T3.5 demo state: locals of main, so the bound refs outlive the modeless
    // dialogs that read them for the whole of exec(). The scale/align dialog
    // needs one flag -- neither a picture nor a label has a value of its own
    // to share, so isDisabled() is
    // the binding those two read-only widgets can offer. The Account form's
    // three strings are what make its right-aligned labels a real form rather
    // than a ruler.
    bool displaysDisabled = false;
    std::string accountName = "Ada Lovelace";
    std::string accountEmail = "ada@example.com";
    std::string accountPassword = "analytical";
    bool accountLocked = false;
    // Controls-gallery state, for the Preview picture that now carries Fit.
    std::string multilineText = "Type something here...";
    std::string galleryPassword;
    int spinInt = 42;
    float spinFloat = 1.5f;
    Date date { .year = 2026, .month = 9, .day = 22 };
    Time time { .hour = 9, .minute = 30, .second = 0 };
    bool toggle = false;
    float galleryProgress = 0.35f;
    std::string tabNote = "Add notes here...";
    bool tabLogging = false;
    Color themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    // T3.5's gallery: one file in one frame under all four scale modes, and
    // one band under all three alignments. On Qt the picture is composed into
    // a frame-sized QPixmap on resizeEvent -- a QLabel scales only by
    // setScaledContents(), which is Stretch and nothing else.
    drawScaleAndAlignUI(displaysDisabled).show();
    // withAlign() in a real form: the labels stretch across column 0 of the
    // Grid, so their colons line up against the fields.
    drawAccountFormUI(accountName, accountEmail, accountPassword, accountLocked).show();
    // The controls gallery, whose Preview picture now carries Fit.
    drawControlsUI(multilineText, galleryPassword, spinInt, spinFloat, date, time,
        toggle, galleryProgress, tabNote, tabLogging, themeColor).show();

    return app.exec();
}
