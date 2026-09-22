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
    // T3.2 demo state. Bound by reference, so it has to outlive the frame
    // loop -- ImGui rebuilds the tree every frame and reads these live.
    std::string openPath = "examples/main_imgui.cpp";
    std::string savePath;
    std::string folderPath;
    std::string lastDialogResult = "(nothing picked yet)";
    bool pickersDisabled = false;
    std::vector<std::string> enabledPlugins { "Formatter", "Debugger" };
    std::string enabledSummary = pluginSummary(enabledPlugins);
    std::vector<std::string> modules { "Core", "Storage" };
    bool moduleListsDisabled = false;
    // T3.3: both start EMPTY, which is the state a placeholder is for.
    std::string searchTerm;
    std::string apiKey;

    runImGuiApp([&]
    {
        // The pickers gallery, now carrying T3.2's CheckListBox: a list with a
        // checkbox on every row, drawn on ImGui as Checkbox rows inside the
        // same child region BeginListBox gives the plain list.
        drawPickersGalleryUI(openPath, savePath, folderPath, lastDialogResult,
            pickersDisabled, enabledPlugins, enabledSummary, searchTerm, apiKey).show();
        // The binding demo: two CheckListBoxes and a ListBox over one
        // std::vector<std::string>.
        drawCheckListBinding(modules, moduleListsDisabled).show();
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
