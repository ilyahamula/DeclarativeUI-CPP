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
    // Demo state. Bound by reference, so it has to outlive the frame loop --
    // ImGui rebuilds the tree every frame and reads these live.
    //
    // The application shell: its menu, tool bar and context menu open their
    // dialogs through ShowAction(), and `renameOpen` is the one flag of theirs
    // the caller owns (About's is keyed by title inside the framework).
    std::string selectedFile = "main.cpp";
    TableRows files {
        { "main.cpp",   "2 KB",  "entry point" },
        { "layout.cpp", "31 KB", "measure/arrange" },
        { "widgets.hpp", "18 KB", "public API" },
        { "engine.hpp", "9 KB",  "" },
    };
    int selectedRow = 0;
    std::string notes = "Try Help > About, the About tool, or right-click a file > Rename...";
    bool shellDisabled = false;
    bool shellOpen = true;
    std::string shellStatus = "(the shell is still open)";
    bool wordWrap = true;
    bool renameOpen = false;
    // The feedback form: Button::onClickShow(), the Button spelling of the
    // same thing. The `open` bool is the single truth about whether the modal
    // is up; the counter is what makes onClose()'s "exactly once" readable.
    std::string deleteFile = "main.cpp";
    std::string deleteAnswer = "(no answer yet)";
    std::string confirmReport = "(the modal has not been closed yet)";
    int confirmCloseCount = 0;
    bool confirmOpen = false;
    bool deleteFormDisabled = false;

    runImGuiApp([&]
    {
        // Shown against its flag every frame, as a Window from the caller's own
        // loop always is. The dialogs it opens are NOT called from here: a
        // show() from a callback is kept up by the framework until it closes.
        drawAppShellUI(selectedFile, files, selectedRow, notes, shellDisabled,
            shellOpen, shellStatus, wordWrap, renameOpen).show(shellOpen);
        // A form that asks and an application-modal box that answers, opened
        // by onClickShow(). On ImGui the box is a BeginPopupModal rather than a
        // plain window, and it is still non-blocking: a popup is drawn, not run.
        drawDeleteFormUI(deleteFile, deleteAnswer, confirmReport, deleteFormDisabled,
            confirmOpen, confirmCloseCount).show();
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
