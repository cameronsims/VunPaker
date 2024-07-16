#include <vunpaker/gui/vpk_nfd.h>

#include <vunpaker/gui/framework.h>
#include <vunpaker/file.h>
#include <stdlib.h>

int vpk::gui::get_files() {
    nfdpathset_t pathset;
    nfdresult_t result = NFD_OpenDialogMultiple("vpk", NULL, &pathset);
    if (result != NFD_OKAY) {
        return false;
    }
    
    const size_t PATH_AMOUNT = NFD_PathSet_GetCount(&pathset);
    for (size_t i = 0; i < PATH_AMOUNT; i++) {

        nfdchar_t* path = NFD_PathSet_GetPath(&pathset, i);
        const size_t PATH_SIZE = strnlen(path, _MAX_PATH);

        wchar_t* temp = new wchar_t[PATH_SIZE + 1];
        mbstowcs(temp, path, PATH_SIZE);
        temp[PATH_SIZE] = '\0';
        vpk::file::read(temp);
        delete[] temp;

    }

    // Free all memory
    NFD_PathSet_Free(&pathset);
    return true;
}