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
    // The todo list (VForEach): rows follow this vector -- add and remove
    // rows, tick them, and only the changed rows are rebuilt.
    std::vector<DemoTodo> todos { { "Write the ForEach demo", true }, { "Try removing a row", false } };
    std::string newTodo;
    std::string todoKeys = "Enter adds, Escape clears";
    // Bound TreeView items and Table rows (an Observable, so polled by its
    // change counter): the buttons change these, both controls follow.
    std::vector<TreeItem> folders { { "Documents", { { "Invoices" }, { "Letters" } }, true }, { "Pictures", { { "2026" } } } };
    Observable<TableRows> files { TableRows { { "readme.txt", "12" }, { "budget.csv", "48" }, { "notes.md", "7" } } };
    std::string folderPick = "Documents/Letters";
    std::string filePick = "budget.csv";

    // One show() per open on a retained backend.
    drawTodoListUI(todos, newTodo, todoKeys).show();
    drawTreeTableBindingUI(folders, files, folderPick, filePick).show();

    return app.exec();
}
