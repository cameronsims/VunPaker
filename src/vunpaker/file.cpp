#include <vunpaker/file.h>

#include <locale>
#include <codecvt>

vpk::FileUnexpectedToken::FileUnexpectedToken() { }
const char* vpk::FileUnexpectedToken::what() const {
    return "\nUnexpected Token in file\n";
}



vpk::FileUnknownVersion::FileUnknownVersion() { }
const char* vpk::FileUnknownVersion::what() const {
    return "\nCould not determine VPK version in file.\n";
}



vpk::FileNotFound::FileNotFound() { }
const char* vpk::FileNotFound::what() const  {
    return "\nCould not load/save to file\n";
}