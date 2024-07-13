#pragma once

#include "vpk_structs.h"

namespace vpk {


    class FileNotFound : std::exception {
    public:
        FileNotFound();
        const char* what() const;
    };

    class FileUnknownVersion : std::exception {
    public:
        FileUnknownVersion();
        const char* what() const;
    };

    class FileUnexpectedToken : std::exception {
    public:
        FileUnexpectedToken();
        const char* what() const;
    };


    DirectoryNode read_vpk(std::istream& in);




}