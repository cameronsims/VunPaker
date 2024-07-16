#pragma once

#include <vunpaker/gui/framework.h>

namespace vpk {
    namespace gui {

        static void glfw_errorCallback(int error, const char* description);

        void glfw_init();

        const char* glfw_glslVersion();

        void glfw_close(GLFWwindow* window);

    }
}