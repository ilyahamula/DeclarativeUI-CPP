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
    // Calendar: Monday-first and Sunday-first side by side, and one date
    // shared by a calendar and a DatePicker.
    Date isoDate = todayDate();
    Date usDate = todayDate();
    std::string calendarStatus = "Pick a day";
    Date sharedDate = todayDate();
    bool calendarDisabled = false;

    // One show() per open on a retained backend.
    drawCalendarGalleryUI(isoDate, usDate, calendarStatus).show();
    drawCalendarBindingUI(sharedDate, calendarDisabled).show();

    return app.exec();
}
