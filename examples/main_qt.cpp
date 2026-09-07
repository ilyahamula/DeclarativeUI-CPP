#include "demo/all_demos.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Grid demo state: locals of main, so the bound refs outlive the
    // modeless dialogs that read them for the whole of exec().
    std::string name = "Ada Lovelace";
    std::string email = "ada@example.com";
    std::string password = "secret";
    bool formDisabled = false;
    std::string shared = "shared";
    bool gridDisabled = false;

    drawAccountFormUI(name, email, password, formDisabled).show();
    drawGridMirror(shared, gridDisabled).show();

    return app.exec();
}
