#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Tooltip demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    // `tooltipHint` is the live-bound one -- it is a TextCtrl's value in the
    // binding dialog and, at the same time, the hover text of two controls
    // beside it.
    std::string multilineText = "Type something here...";
    std::string password;
    int spinInt = 42;
    float spinFloat = 1.5f;
    Date date { .year = 2026, .month = 2, .day = 22 };
    Time time { .hour = 9, .minute = 30, .second = 0 };
    bool toggle = false;
    float progress = 0.35f;
    std::string tabNote = "Add notes here...";
    bool tabLogging = false;
    Color themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    std::string tooltipHint = "Edit the field above and hover me again.";
    bool tooltipDisabled = false;

    drawControlsUI(multilineText, password, spinInt, spinFloat,
        date, time, toggle, progress, tabNote, tabLogging,
        themeColor).show();
    drawTooltipBinding(tooltipHint, tooltipDisabled).show();

    return app.exec();
}
