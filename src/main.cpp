#include <vunpaker/vpk.h>
#include <vunpaker/file.h>

#include <vunpaker/gui/framework.h>
#include <vunpaker/gui/WindowClass.h>
#include <vunpaker/gui/vpk_glfw.h>
#include <vunpaker/gui/vpk_imgui.h>
#include <vunpaker/gui/winloop.h>

#include <locale>

#include <stdio.h>
#include <stdlib.h>


int wmain(size_t argc, wchar_t** argv) {
    // Initalise GLFW
    vpk::gui::glfw_init();

    // Set local
    std::locale::global(std::locale(""));  // Set the locale for wide characters

    // Create window with graphics context
    GLFWwindow* window = vpk::gui::imgui_init();
    Window::handle_set(window);

    // Get exe
    const wchar_t BASE_DIRECTORY[_MAX_PATH] = L"D:/VunPaKer/temp";
    Window::directory_set(BASE_DIRECTORY);

    // For all files
    for (size_t i = 0; i < argc - 1; i++) {
        // File value
        const wchar_t* const FILE_NAME = argv[i + 1];

        // Get the directory of the file's temporary location
        wchar_t file_directory[_MAX_PATH] = L"";
        vpk::file::extractFileName(BASE_DIRECTORY, FILE_NAME, file_directory);
        vpk::file::read(FILE_NAME, file_directory);
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