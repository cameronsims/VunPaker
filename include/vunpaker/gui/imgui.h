#pragma once

#include <vunpaker/gui/framework.h>

#define VUNPAKER_NAME "VunPaKer"
#define VUNPAKER_CHILD_NAME "VunPaKer - %s"
#define VUNPAKER_DEFAULT_WIDTH 1280
#define VUNPAKER_DEFAULT_HEIGHT 720

namespace vpk {
    namespace gui {

        typedef struct {
            GLclampf r, g, b, alpha;
        } RGB;

        RGB RGB_create(GLclampf r, GLclampf g, GLclampf b, GLclampf alpha);
        RGB RGB_create(const ImVec4& v);

        GLFWwindow* imgui_init();

        void imgui_setGLFW(GLFWwindow* window);

        void imgui_setStyle();

        void imgui_update(GLFWwindow* window, const ImVec4& CLEAR_COLOR);

        void imgui_close();

    }
}