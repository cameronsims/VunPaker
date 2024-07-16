#include <vunpaker/gui/WindowClass.h>

#include <vunpaker/vpk.h>
#include <vunpaker/file.h>

#include <vunpaker/gui/framework.h>
#include <vunpaker/gui/WindowClass.h>
#include <vunpaker/gui/vpk_glfw.h>
#include <vunpaker/gui/vpk_imgui.h>
#include <vunpaker/gui/winloop.h>

bool Window::condition(GLFWwindow* window) {
    return !glfwWindowShouldClose(window);
}

void Window::create() {

    // Set menubar
    vpk::gui::imgui_setMenubar();

    // Loop for all loaded VPKs
    for (size_t i = 0; i < m_vpks.size(); i++) {

        // Set string to name of window instance
        char window_name[_MAX_PATH + 13];
        sprintf(window_name, VUNPAKER_CHILD_NAME, m_names[i].c_str());

        // Begin Window Handle
        ImGui::Begin(window_name, nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar);

        if (ImGui::TreeNode("root")) {
            ImGui::Text("contents");
            ImGui::TreePop();
        }

        // Stop writing to window instance
        ImGui::End();
    }
}

GLFWwindow* Window::handle_get() {
    return m_handle;
}

void Window::handle_set(GLFWwindow* window) {
    m_handle = window;
}

void Window::vpk_add(const wchar_t* const str, const vpk::VPK& vpk) {
    // Push the name back into the value
    m_names.push_back(str);

    // Push the VPK's values
    m_vpks.push_back(vpk);
}

vpk::VPK& Window::vpk_get(size_t i) {
    return m_vpks[i];
}

void Window::vpk_removeAll() {
    m_vpks.clear();
    m_names.clear();
}

std::vector<std::wstring> Window::m_names;
std::vector<vpk::VPK>     Window::m_vpks;
GLFWwindow* Window::m_handle = nullptr;