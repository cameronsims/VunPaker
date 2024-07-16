#pragma once

#include <vunpaker/gui/framework.h>

#define VUNPAKER_NAME "VunPaKer"
#define VUNPAKER_CHILD_NAME "VunPaKer - %ls"
#define VUNPAKER_DEFAULT_WIDTH 720
#define VUNPAKER_DEFAULT_HEIGHT 512

#define VUNPAKER_MENU_FILE "File"
#define VUNPAKER_MENU_VIEW "View"
#define VUNPAKER_MENU_OPTS "Options"

namespace vpk {
    namespace gui {

        typedef struct {
            GLclampf r, g, b, alpha;
        } RGB;

        RGB RGB_create(GLclampf r, GLclampf g, GLclampf b, GLclampf alpha);
        RGB RGB_create(const ImVec4& v);

        GLFWwindow* imgui_init();

        void imgui_setMenubar();

        void imgui_setGLFW(GLFWwindow* window);

        void imgui_setStyle();

        void imgui_update(GLFWwindow* window, const ImVec4& CLEAR_COLOR);

        void imgui_close();

    }
}