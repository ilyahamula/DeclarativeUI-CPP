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
    // Slider orientation and ticks: vertical channel faders with a SpinBox
    // each, and a ticked float master.
    DemoMixer mixer;
    // EditableCombo: the gallery (free text vs the list-only ComboBox).
    std::string comboFont = "Arial";
    std::string comboCity;
    std::string comboSize = "Medium";
    std::string comboStatus = "Pick or type";

    // One show() per open on a retained backend.
    drawEditableComboGalleryUI(comboFont, comboCity, comboSize, comboStatus).show();
    drawMixerUI(mixer).show();

    return app.exec();
}
