#include "demo/all_demos.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <vector>

namespace
{
    // Owns the GLFW/OpenGL/ImGui lifetime and the frame loop; `drawUI` is invoked
    // once per frame between NewFrame() and Render(), so callers only describe UI.
    void runImGuiApp(const std::function<void()>& drawUI);
}

int main(int argc, char** argv)
{
    // Demo state. Bound by reference, so it has to outlive the frame loop --
    // ImGui rebuilds the tree every frame and reads these live.
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

    runImGuiApp([&]
    {
        drawTodoListUI(todos, newTodo, todoKeys).show();
        drawTreeTableBindingUI(folders, files, folderPick, filePick).show();
    });

    return 0;
}

namespace
{
    void runImGuiApp(const std::function<void()>& drawUI)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // Large enough to host the auto-fit application shell Window
        GLFWwindow* window = glfwCreateWindow(1100, 800, "DeclarativeUI - ImGui", nullptr, nullptr);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 150");

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            drawUI();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
