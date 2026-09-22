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
    // T3.5 demo state. Bound by reference, so it has to outlive the frame loop
    // -- ImGui rebuilds the tree every frame and reads these live. The
    // scale/align dialog needs one flag -- neither a picture nor a label has a
    // value of its own to share, so isDisabled() is
    // the binding those two read-only widgets can offer. The Account form's
    // three strings are what make its right-aligned labels a real form rather
    // than a ruler.
    bool displaysDisabled = false;
    std::string accountName = "Ada Lovelace";
    std::string accountEmail = "ada@example.com";
    std::string accountPassword = "analytical";
    bool accountLocked = false;
    // Controls-gallery state, for the Preview picture that now carries Fit.
    std::string multilineText = "Type something here...";
    std::string galleryPassword;
    int spinInt = 42;
    float spinFloat = 1.5f;
    Date date { .year = 2026, .month = 9, .day = 22 };
    Time time { .hour = 9, .minute = 30, .second = 0 };
    bool toggle = false;
    float galleryProgress = 0.35f;
    std::string tabNote = "Add notes here...";
    bool tabLogging = false;
    Color themeColor { .r = 0.26f, .g = 0.59f, .b = 0.98f, .a = 1.0f };

    runImGuiApp([&]
    {
        // T3.5's gallery: one file in one frame under all four scale modes,
        // and one band under all three alignments. On ImGui both are drawn by
        // hand -- the picture on the window draw list inside a clip rect,
        // the label by offsetting the cursor across the frame's slack.
        drawScaleAndAlignUI(displaysDisabled).show();
        // withAlign() in a real form: the labels stretch across column 0 of
        // the Grid, so their colons line up against the fields.
        drawAccountFormUI(accountName, accountEmail, accountPassword, accountLocked).show();
        // The controls gallery, whose Preview picture now carries Fit.
        drawControlsUI(multilineText, galleryPassword, spinInt, spinFloat, date, time,
            toggle, galleryProgress, tabNote, tabLogging, themeColor).show();
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
