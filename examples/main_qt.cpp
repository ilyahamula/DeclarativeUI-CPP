#include "demo/all_demos.hpp"

#include <QApplication>

namespace
{
    void DrawUI()
    {
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

        drawControlsUI(multilineText, password, spinInt,
            spinFloat, date, time, toggle, progress, tabNote, tabLogging, themeColor, [&]() {
                MessageBox("Controls State",
                    "multilineText: " + multilineText + "\n"
                    "password: " + password + "\n"
                    "spinInt: " + std::to_string(spinInt) + "\n"
                    "spinFloat: " + std::to_string(spinFloat) + "\n"
                    "date: " + std::to_string(date.year) + "-"
                             + std::to_string(date.month) + "-"
                             + std::to_string(date.day) + "\n"
                    "time: " + std::to_string(time.hour) + ":"
                             + std::to_string(time.minute) + ":"
                             + std::to_string(time.second))
                    .withStyle(MessageBoxStyle::Info)
                    .withButtons(MessageBoxButtons::OK)
                    .show();
            },
            [&]() {
                MessageBox("Question", "Do you like this framework?")
                    .withStyle(MessageBoxStyle::Question)
                    .withButtons(MessageBoxButtons::YesNo)
                    .show();
            }).show();
    }
} // namespace


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    float value = 50.5f;
    bool checked = false;
    drawProgressBarBindedToSlider(value, checked).show();

    // Selection demo state: locals of main, so the bound refs outlive the
    // modeless dialog that reads them for the whole of exec().
    std::string favouriteLanguage = "Rust";
    std::vector<int> selectedTags { 1, 3 };
    std::string comboChoice = "C++";
    bool listsDisabled = false;
    drawSelectionUI(favouriteLanguage, selectedTags, comboChoice, listsDisabled).show();

    // ListBox value-binding demo state.
    std::string mirroredListChoice = "Rust";
    bool listMirrorDisabled = false;
    drawListBoxMirror(mirroredListChoice, listMirrorDisabled).show();

    return app.exec();
}
