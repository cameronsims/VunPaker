#pragma once

#include <vunpaker/gui/glfw.h>
#include <vunpaker/gui/imgui.h>

vpk::gui::RGB vpk::gui::RGB_create(GLclampf r, GLclampf g, GLclampf b, GLclampf alpha) {
    RGB clr;
    clr.alpha = alpha;
    clr.r = r * alpha;
    clr.g = g * alpha;
    clr.b = b * alpha;
    return clr;
}

vpk::gui::RGB vpk::gui::RGB_create(const ImVec4& v) {
    return RGB_create(v.x, v.y, v.z, v.w);
}

GLFWwindow* vpk::gui::imgui_init() {
    // Create the window
    GLFWwindow* window = glfwCreateWindow(VUNPAKER_DEFAULT_WIDTH, VUNPAKER_DEFAULT_HEIGHT, VUNPAKER_NAME, NULL, NULL);
    if (window == NULL) {
        throw 1;
    }

    // Link ImGui to GLFW
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    vpk::gui::imgui_setStyle();
    vpk::gui::imgui_setGLFW(window);

    return window;
}

void vpk::gui::imgui_setGLFW(GLFWwindow* window) {
    // Get the GLSL version
    const char* glsl_version = glfw_glslVersion();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void vpk::gui::imgui_setStyle() {
    // Set IO
    ImGuiIO& io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

void vpk::gui::imgui_update(GLFWwindow* window, const ImVec4& CLEAR_COLOR) {

    // Colour Struct
    RGB clr = RGB_create(CLEAR_COLOR);

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clr.r, clr.g, clr.b, clr.alpha);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
    glfwSwapBuffers(window);
}

void vpk::gui::imgui_close() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}