#include <vunpaker/file.h>
#include <vunpaker/vpk.h>

#include <locale>
#include <codecvt>

#include <vunpaker/gui/WindowClass.h>

vpk::file::UnexpectedToken::UnexpectedToken() { }
const char* vpk::file::UnexpectedToken::what() const {
    return "\nUnexpected Token in file\n";
}



vpk::file::UnknownVersion::UnknownVersion() { }
const char* vpk::file::UnknownVersion::what() const {
    return "\nCould not determine VPK version in file.\n";
}



vpk::file::NotFound::NotFound() { }
const char* vpk::file::NotFound::what() const  {
    return "\nCould not load/save to file\n";
}

void vpk::file::read(const wchar_t* fileName) {
    // VPK Input
    FILE* input;
    _wfopen_s(&input, fileName, L"rb");
    if (!input) {
        throw vpk::file::NotFound();
    }

    printf("%ls\n", fileName);

    // Read VPK Type
    vpk::VPK vpk{};

    // Read 
    vpk::file::extractHeader(input, vpk);
    vpk::file::extractBody(input, vpk);
    if (vpk.header.version == 2)
        vpk::file::extractFooter(input, vpk);

    Window::vpk_add(fileName, vpk);

    fclose(input);
}