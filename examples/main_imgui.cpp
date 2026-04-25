#include "basic_demo.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

int main(int argc, char** argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "DeclarativeUI - ImGui", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    std::string m_text = "Initial text";
    float m_sliderValue = 50.5f;
    int m_sliderValue1 = 25;
    int m_radioChoice1 = 0;
    int m_radioChoice = 0;
    std::string m_selectedCombo = "Goodbye";
    bool m_checked = false;
    bool m_toggle = false;
    bool showPopup = false;

    std::string m_multilineText = "Type something here...";
    std::string m_password;
    int m_spinInt = 42;
    float m_spinFloat = 1.5f;
    Date m_date { .year = 2026, .month = 2, .day = 22 };
    Time m_time { .hour = 9, .minute = 30, .second = 0 };
    bool showControlsPopup = false;
    float m_progress = 0.35f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

//         drawUI(m_text, m_sliderValue, m_sliderValue1, m_radioChoice, m_radioChoice1, m_selectedCombo, m_checked,
//             [&showPopup]() {
//                 showPopup = true;
//             }).show();

//         if (showPopup)
//         {
//             ImGui::OpenPopup("Message");

//             ImGui::SetNextWindowSizeConstraints(ImVec2(300, 0), ImVec2(FLT_MAX, FLT_MAX));
//             if (ImGui::BeginPopupModal("Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
//             {
// #ifdef USE_LOGGER
//                 ImGui::TextUnformatted(Logger::instance().getAll().c_str());
// #else
//                 ImGui::Text("text: %s", m_text.c_str());
//                 ImGui::Text("sliderValue: %.2f", m_sliderValue);
//                 ImGui::Text("sliderValue1: %d", m_sliderValue1);
//                 ImGui::Text("radioChoice: %d", m_radioChoice);
//                 ImGui::Text("radioChoice1: %d", m_radioChoice1);
//                 ImGui::Text("selectedCombo: %s", m_selectedCombo.c_str());
//                 ImGui::Text("checked: %s", m_checked ? "true" : "false");
// #endif
//                 ImGui::Separator();

//                 if (ImGui::Button("OK", ImVec2(120, 0)))
//                 {
//                     ImGui::CloseCurrentPopup();
//                     showPopup = false;
//                 }

//                 ImGui::EndPopup();
//             }
//         }

        drawControlsUI(m_multilineText, m_password, m_spinInt, m_spinFloat, m_date, m_time, m_toggle, m_progress,
            [&showControlsPopup]() {
                showControlsPopup = true;
            }).show();

        if (showControlsPopup)
        {
            ImGui::OpenPopup("Controls State");

            ImGui::SetNextWindowSizeConstraints(ImVec2(300, 0), ImVec2(FLT_MAX, FLT_MAX));
            if (ImGui::BeginPopupModal("Controls State", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("multilineText: %s", m_multilineText.c_str());
                ImGui::Text("password: %s", m_password.c_str());
                ImGui::Text("spinInt: %d", m_spinInt);
                ImGui::Text("spinFloat: %.2f", m_spinFloat);
                ImGui::Text("date: %04d-%02d-%02d", m_date.year, m_date.month, m_date.day);
                ImGui::Text("time: %02d:%02d:%02d", m_time.hour, m_time.minute, m_time.second);
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                    showControlsPopup = false;
                }

                ImGui::EndPopup();
            }
        }

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
    return 0;
}
