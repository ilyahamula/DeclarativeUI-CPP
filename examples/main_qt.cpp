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
    // Spinner: the gallery (default, sized, logo, stopped) and a running
    // flag shared by two spinners, a CheckBox and a ToggleButton.
    bool spinnerBusy = true;
    std::string spinnerStatus = "Working...";

    // One show() per open on a retained backend.
    drawSpinnerGalleryUI().show();
    drawSpinnerBindingUI(spinnerBusy, spinnerStatus).show();

    return app.exec();
}
