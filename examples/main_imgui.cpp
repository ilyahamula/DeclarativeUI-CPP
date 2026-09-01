#include "demo/all_demos.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <functional>

namespace
{
    // Owns the GLFW/OpenGL/ImGui lifetime and the frame loop; `drawUI` is invoked
    // once per frame between NewFrame() and Render(), so callers only describe UI.
    void runImGuiApp(const std::function<void()>& drawUI);
}

int main(int argc, char** argv)
{
    // Table demo state. Bound by reference, so it has to outlive the frame loop
    // -- ImGui rebuilds the tree every frame and reads these live.
    // The Notes column is editable, which is the other half of why these are
    // refs: an edit has to land somewhere the widgets do not own.
    TableRows files {
        { "main.cpp",      "12 KB", "entry point" },
        { "layout.cpp",    "48 KB", "measure/arrange" },
        { "measure.cpp",   "9 KB",  "" },
        { "widgets.hpp",   "31 KB", "public API" },
        { "stacks.hpp",    "7 KB",  "" },
        { "buildable.hpp", "3 KB",  "concepts" },
    };
    int selectedRow = 0;
    std::vector<int> checkedRows { 1, 3 };
    bool tablesDisabled = false;

    // The same widget bound the other way: one rows vector and one key shared by
    // two tables.
    TableRows sharedFiles = files;
    std::string sharedFile = "layout.cpp";
    bool mirrorDisabled = false;

    runImGuiApp([&]
    {
        drawTableUI(files, selectedRow, checkedRows, tablesDisabled).show();
        drawTableMirror(sharedFiles, sharedFile, mirrorDisabled).show();
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

        // Large enough to host the auto-fit Controls Demo dialog (~832x520)
        GLFWwindow* window = glfwCreateWindow(1000, 700, "DeclarativeUI - ImGui", nullptr, nullptr);
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
