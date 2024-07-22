#pragma once

#include "vpk_structs.h"

#include <functional>

//#define VPK_PRINT_DIRECTORY

namespace vpk {

    namespace file {

        class NotFound : std::exception {
        public:
            NotFound();
            const char* what() const;
        };

        class UnknownVersion : std::exception {
        public:
            UnknownVersion();
            const char* what() const;
        };

        class UnexpectedToken : std::exception {
        public:
            UnexpectedToken();
            const char* what() const;
        };

        class UnexpectedName : UnexpectedToken {
        public:
            UnexpectedName();
            const char* what() const;
        };

        uint32_t get2Bytes(FILE* file, vpk::VPK& vpk);

        uint32_t get4Bytes(FILE* file, vpk::VPK& vpk);

        bool nextIsTerminator(FILE* file, const char TERMINATOR[3]);



        void readNextToken(FILE* file, vpk::VPK& vpk, char** str, const std::function<void(size_t)>& fun);

        void getExtension(FILE* file, vpk::VPK& vpk, char** ext, const std::function<void(size_t)>& fun);

        void getDirectory(FILE* file, vpk::VPK& vpk, const wchar_t* const directory, char** dir, const std::function<void(size_t)>& fun);

        vpk::DirectoryEntry getDirectoryEntry(FILE* file, vpk::VPK& vpk, const wchar_t* const directory, const char* const ext, const char* const dir, char** fnm, const std::function<void(size_t)>& fun);


        enum class TerminationType {
            NONE = 0,
            DIRECTORY = 1,
            EXTENSION = 2,
            TREE = 3
        };

        TerminationType checkTermination(FILE* file, char** ext, char** dir, const std::function<void(size_t)>& fun);



        void extractHeader(FILE* file, vpk::VPK& vpk, const wchar_t* directory);

        void extractBody(FILE* file, vpk::VPK& vpk, const wchar_t* directory);

        void extractFooter(FILE* file, vpk::VPK& vpk, const wchar_t* directory);

        void extractFileName(const wchar_t* newDirectory, const wchar_t* fileLocation, wchar_t* buffer);

        void read(const wchar_t* directory, const wchar_t* fileName);

    }
}