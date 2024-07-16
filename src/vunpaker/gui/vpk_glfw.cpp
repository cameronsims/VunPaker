#pragma once

#include <vunpaker/gui/vpk_glfw.h>
#include <stdio.h>

static void vpk::gui::glfw_errorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void vpk::gui::glfw_init() {
    // Set error handler
    glfwSetErrorCallback(glfw_errorCallback);

    // Create instance
    int code = glfwInit();
    if (!code) {
        throw 1;
    }

}

const char* vpk::gui::glfw_glslVersion() {
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    //const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    return "#version 100";
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    //const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    return "#version 150";
#else
    // GL 3.0 + GLSL 130
    //const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    return "#version 130";
#endif
}

void vpk::gui::glfw_close(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}