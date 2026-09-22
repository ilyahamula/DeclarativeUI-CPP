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
    // T3.6 demo state. Bound by reference, so it has to outlive the frame loop
    // -- ImGui rebuilds the tree every frame and reads these live. The two
    // `open` bools are the single truth about whether their dialog is up; the
    // two counters are what make onClose()'s "exactly once" readable.
    std::string deleteFile = "main.cpp";
    std::string deleteAnswer = "(no answer yet)";
    std::string confirmReport = "(the modal has not been closed yet)";
    int confirmCloseCount = 0;
    bool confirmOpen = false;
    bool deleteFormDisabled = false;
    bool detailsOpen = false;
    std::string detailsNote = "Shared by the panel and the dialog.";
    std::string detailsReport = "(the dialog has not been closed yet)";
    int detailsCloseCount = 0;
    bool panelDisabled = false;
    // Controls-gallery state, for context beside the new dialogs.
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

    // The one piece of per-backend wiring T3.6 needs, and the whole of it.
    // On an immediate backend show() IS the frame: the loop below calls it every
    // frame, so asking for a dialog is nothing more than setting its flag. (The
    // retained mains hand these lambdas a show() call as well, because wx and Qt
    // destroyed the native dialog when it closed.)
    auto openConfirm = [&confirmOpen] { confirmOpen = true; };
    auto openDetails = [&detailsOpen] { detailsOpen = true; };

    runImGuiApp([&]
    {
        // T3.6's gallery: a form that asks, and an application-modal box that
        // answers. On ImGui the box is a BeginPopupModal rather than a plain
        // window -- nothing else dims the windows behind it and refuses them
        // input -- and it is still non-blocking, because a popup is drawn, not
        // run.
        drawDeleteFormUI(deleteFile, deleteAnswer, confirmReport, deleteFormDisabled,
            openConfirm).show();
        // The binding demo: a dialog's open flag as an ordinary bound bool,
        // shared with a check box and a toggle in this panel.
        drawDialogBinding(detailsOpen, detailsNote, detailsReport, panelDisabled,
            openDetails).show();
        // Both flag-shown dialogs are called EVERY FRAME, open or not: the
        // wrapper draws nothing while the flag is clear and reports the close
        // once. This is what makes re-opening free here and a show() call on wx
        // and Qt.
        drawDetailsUI(detailsOpen, detailsNote, detailsReport, detailsCloseCount)
            .show(detailsOpen);
        // Drawn last, and from OUTSIDE every window: OpenPopup and
        // BeginPopupModal have to meet in one ID scope, and this is it.
        drawConfirmDeleteUI(confirmOpen, deleteFile, deleteAnswer, confirmReport,
            confirmCloseCount).show(confirmOpen);
        // The controls gallery, for context beside the new dialogs.
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
