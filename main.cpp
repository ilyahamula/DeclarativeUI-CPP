#include "declarative_ui.hpp"

namespace
{
    auto drawUI()
    {
        return Dialog {
            "Declarative UI Dialog",
            VStack {
                LayoutFlags().Expand().Border(Side::All, 10),
                HStack {
                    Button{"Browse..."}
                        .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 15))
                        .onClick([]() {
                            //wxMessageBox("Browse button clicked!");
                        }),
                    TextCtrl{"Some text"}
                        .withFlags(LayoutFlags(1).Expand())
                },
                HGroupBox {
                    Slider { { .min = 0, .max = 100, .value = 50 } }
                        .withFlags(LayoutFlags(1).Expand()),
                    VGroupBox {
                        LayoutFlags().CenterVertical().Border(Side::Left),
                        RadioButton{"On"}.withStyle(4), // wxRB_GROUP
                        RadioButton{"Off"}
                    }
                },
                HStack {
                    StaticText{"Ready"}
                        .withFlags(LayoutFlags(1).CenterVertical().Border(Side::Right)),
                    Button{"OK"}
                        .withFlags(LayoutFlags().CenterVertical())
                },
                HGroupBox { "Combo Group",
                    ComboBox{ {"Hello", "Goodbye", "Nihao" }, "G" }
                        .withFlags(LayoutFlags(1).CenterVertical().Border(Side::Right)),
                    CheckBox{}
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
public:
    bool OnInit() override
    {
        drawUI().show();
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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello ImGui");
        ImGui::Text("Simple ImGui window example");
        static float slider_value = 0.5f;
        ImGui::SliderFloat("Slider", &slider_value, 0.0f, 1.0f);
        static bool check = false;
        ImGui::Checkbox("Check me", &check);
        if (ImGui::Button("OK"))
            ImGui::OpenPopup("Clicked");
        if (ImGui::BeginPopup("Clicked"))
        {
            ImGui::Text("Button was clicked!");
            ImGui::EndPopup();
        }
        ImGui::End();

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
