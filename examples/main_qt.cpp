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
    // RadioGroup sharing its index with a slider and a spin box.
    int level = 2;
    bool levelDisabled = false;
    std::string lastPick = "Nothing picked yet";

    // One show() per open on a retained backend.
    drawTodoListUI(todos, newTodo).show();
    drawRadioGroupMirror(level, levelDisabled, lastPick).show();

    return app.exec();
}
