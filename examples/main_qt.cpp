#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Text gallery state: locals of main, so the bound refs outlive the
    // modeless dialog that reads them for the whole of exec().
    std::string editableText = "Type here -- this one is editable.";
    bool fieldsDisabled = false;

    drawTextUI(editableText, fieldsDisabled).show();

    return app.exec();
}
