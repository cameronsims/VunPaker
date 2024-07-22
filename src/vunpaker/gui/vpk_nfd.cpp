#include <vunpaker/gui/vpk_nfd.h>

#include <vunpaker/gui/framework.h>
#include <vunpaker/gui/WindowClass.h>
#include <vunpaker/file.h>
#include <stdlib.h>

int vpk::gui::get_files() {
    nfdpathset_t pathset;
    nfdresult_t result = NFD_OpenDialogMultiple("vpk", NULL, &pathset);
    if (result != NFD_OKAY) {
        return false;
    }
    
    wchar_t newDirectory[_MAX_PATH] = L"";

    wchar_t directory[_MAX_PATH];
    Window::directory_get(directory);
    
    const size_t PATH_AMOUNT = NFD_PathSet_GetCount(&pathset);
    for (size_t i = 0; i < PATH_AMOUNT; i++) {

        nfdchar_t* path = NFD_PathSet_GetPath(&pathset, i);
        const size_t PATH_SIZE = strnlen(path, _MAX_PATH);

        // Get the directory of the file's temporary location
        wchar_t file_directory[_MAX_PATH];
        
        // Get name 
        wchar_t* name = new wchar_t[PATH_SIZE + 1];
        mbstowcs(name, path, PATH_SIZE);
        name[PATH_SIZE] = '\0';

        vpk::file::extractFileName(newDirectory, name, file_directory);
        vpk::file::read(name, directory);
        delete[] name;

    }

    // Free all memory
    NFD_PathSet_Free(&pathset);
    return true;
}