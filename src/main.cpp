#include <vunpaker/vpk.h>
#include <vunpaker/file.h>

#include <vunpaker/gui/framework.h>
#include <vunpaker/gui/WindowClass.h>
#include <vunpaker/gui/vpk_glfw.h>
#include <vunpaker/gui/vpk_imgui.h>
#include <vunpaker/gui/winloop.h>

#include <stdio.h>
#include <stdlib.h>


int wmain(size_t argc, wchar_t** argv) {
    // Initalise GLFW
    vpk::gui::glfw_init();

    // Create window with graphics context
    GLFWwindow* window = vpk::gui::imgui_init();
    Window::handle_set(window);

    // Get exe
    wchar_t directory[_MAX_PATH] = L"D:\\VunPaker\\temp";

    // Read input
    if (argc > 1) {
        // For all files
        for (size_t i = 0; i < argc - 1; i++) {
            vpk::file::read(argv[i + 1]);
        }
        
    }
    
    // Loop the program until we want to leave
    vpk::gui::loop(window, Window::create, Window::condition);

    // Cleanup
    vpk::gui::imgui_close();
    vpk::gui::glfw_close(window);

    return 0;
}





// These are in the root source file due to external symbols being painful...
// They have been placed at the bottom to make the file look prettier
// I do not want to / cannot edit library files to not that they are "extern"s

#include "vunpaker/gui/vpk_imgui.cpp"
#include "vunpaker/gui/vpk_glfw.cpp"
#include "vunpaker/gui/winloop.cpp"