#include "declarative_ui.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

namespace
{
    auto drawUI(std::string& text,
        float& sliderValue,
        int& radioChoice,
        std::string& selectedCombo,
        bool& checked,
        std::function<void()> onButtonClick = []() {})
    {
        return Dialog {
            "Declarative UI Dialog",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 10),
                HStack {
                    Button{"Browse..."}
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 15))
                        .onClick(std::move(onButtonClick)),
                    TextCtrl{text}
                        .withFlags(LayoutFlags(1).Expand())
                },
                HGroupBox {
                    Slider { { .min = 0.0f, .max = 65.0f }, sliderValue }
                        .withFlags(LayoutFlags(1).Expand()),
                    VGroupBox {
                        LayoutFlags().CenterVertical().Border(Side::Left),
                        RadioButton{radioChoice, "On"},
                        RadioButton{radioChoice, "Off"}
                    }
                },
                HStack {
                    StaticText{"Ready"}
                        .withFlags(LayoutFlags(1).CenterVertical().Border(Side::Right)),
                    Button{"OK"}
                        .withFlags(LayoutFlags().CenterVertical())
                },
                HGroupBox { "Combo Group",
                    ComboBox{ {"Hello", "Goodbye", "Nihao" }, selectedCombo }
                        .withFlags(LayoutFlags(1).CenterVertical().Border(Side::Right)),
                    CheckBox{checked, "Check me!"}
                        .withFlags(LayoutFlags().CenterVertical())
                }
            }
        };
    }
}

#ifdef USE_WX
#include <wx/wx.h>

class DeclarativeApp : public wxApp
{
    std::string m_text = "Initial text";
    float m_sliderValue = 50.5f;
    int m_radioChoice = 0;
    std::string m_selectedCombo = "Goodbye";
    bool m_checked = false;
public:
    bool OnInit() override
    {
        drawUI(m_text, m_sliderValue, m_radioChoice, m_selectedCombo, m_checked, []() {
#ifdef USE_LOGGER
            wxMessageBox(Logger::instance().getAll(), "Info", wxOK | wxICON_INFORMATION);
#endif
        }).show();
        return true;
    }
};

// Use explicit main to satisfy environments that expect it while keeping wxApp wiring minimal.
wxIMPLEMENT_APP_NO_MAIN(DeclarativeApp);

int main(int argc, char** argv)
{
    if(!wxEntryStart(argc, argv))
        return -1;

    wxTheApp->CallOnInit();
    int code = wxTheApp->OnRun();
    wxTheApp->OnExit();
    wxEntryCleanup();
    return code;
}
#elif defined(USE_QT)
int main(int argc, char** argv)
{
    return 0;
}
#elif defined(USE_IMGUI)
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
    int m_radioChoice = 0;
    std::string m_selectedCombo = "Goodbye";
    bool m_checked = false;
    bool showPopup = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawUI(m_text, m_sliderValue, m_radioChoice, m_selectedCombo, m_checked,
            [&showPopup]() {
                showPopup = true;  
            }).show();

#ifdef USE_LOGGER
        Logger::instance().stopLogging();
#endif
        if (showPopup)
        {
            ImGui::OpenPopup("Message");

            ImGui::SetNextWindowSizeConstraints(ImVec2(600, 0), ImVec2(FLT_MAX, FLT_MAX));
            if (ImGui::BeginPopupModal("Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                #ifdef USE_LOGGER
                ImGui::TextUnformatted(Logger::instance().getAll().c_str());
                #elif
                ImGui::Text("Button was clicked!");
                #endif
                ImGui::Separator();
            
                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                    showPopup = false;
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
#endif
