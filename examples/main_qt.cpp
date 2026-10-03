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
    // VirtualList: a million generated log lines beside a ListBox, and a
    // list whose row, count and revision are caller-owned.
    int logCount = 1000000;
    int logRow = -1;
    std::string languagePick = "C++";
    std::string listStatus = "Pick a log line";
    DemoVirtualList virtualList;

    // One show() per open on a retained backend.
    drawVirtualListGalleryUI(logCount, logRow, languagePick, listStatus).show();
    drawVirtualListBindingUI(virtualList).show();

    return app.exec();
}
