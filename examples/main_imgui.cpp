#include "demo/all_demos.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <string>

namespace
{
    // Owns the GLFW/OpenGL/ImGui lifetime and the frame loop; `drawUI` is invoked
    // once per frame between NewFrame() and Render(), so callers only describe UI.
    void runImGuiApp(const std::function<void()>& drawUI);
}

int main(int argc, char** argv)
{
    // Window demo state. Bound by reference, so it has to outlive the frame
    // loop -- ImGui rebuilds the tree every frame and reads these live.
    std::string selectedFile = "main.cpp";
    TableRows files {
        { "main.cpp",   "2 KB",  "entry point" },
        { "layout.cpp", "31 KB", "measure/arrange" },
        { "widgets.hpp", "18 KB", "public API" },
        { "engine.hpp", "9 KB",  "" },
    };
    int selectedRow = 0;
    std::string notes = "Drag a sash, then resize the window: the panes take the room.";
    bool shellDisabled = false;
    bool shellOpen = true;
    std::string shellStatus = "(the shell is still open)";
    bool panelDisabled = false;

    runImGuiApp([&]
    {
        // The shell is a Window: resizable by default, shown against a bool
        // that its own Close button and the control panel both write.
        drawAppShellUI(selectedFile, files, selectedRow, notes, shellDisabled,
            shellOpen, shellStatus).show(shellOpen);
        // Fixed(), and shown with the plain show() -- the panel stays up so the
        // shell's onClose() report is readable after the shell has gone.
        //
        // Only on ImGui does re-ticking the box bring the shell back: the frame
        // loop calls show() again. wx and Qt destroyed the frame.
        drawWindowBinding(shellOpen, shellStatus, panelDisabled).show();
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
