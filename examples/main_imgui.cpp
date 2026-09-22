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
    // T3.3: both start EMPTY, which is the state a placeholder is for.
    std::string searchTerm;
    std::string apiKey;
    // T3.4: the shared float and the two halves of the busy flag. `idle` is
    // the complement isDisabled() cannot express -- it binds a bool, not `!bool`.
    float progress = 35.0f;
    bool busy = false;
    bool idle = true;

    runImGuiApp([&]
    {
        // T3.4's gallery: a determinate bar next to a busy one. On ImGui the
        // busy half is a NEGATIVE fraction fed from GetTime(), which the
        // per-frame rebuild makes free -- the band's position IS the number.
        drawIndeterminateProgressUI().show();
        // The pickers gallery it was split out of, for the surrounding context.
        drawPickersGalleryUI(openPath, savePath, folderPath, lastDialogResult,
            pickersDisabled, enabledPlugins, enabledSummary, searchTerm, apiKey).show();
        // The binding demo: a determinate bar sharing a float with a slider,
        // next to a valueless one, with a checkbox picking which half is live.
        drawIndeterminateProgressBinding(progress, busy, idle).show();
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
