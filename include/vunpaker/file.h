#pragma once

#include "vpk_structs.h"

#include <functional>

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

        uint32_t get2Bytes(FILE* file, vpk::VPK& vpk);

        uint32_t get4Bytes(FILE* file, vpk::VPK& vpk);

        bool nextIsTerminator(FILE* file, const char TERMINATOR[3]);



        void readNextToken(FILE* file, vpk::VPK& vpk, char** str, const std::function<void(size_t)>& fun);

        void getExtension(FILE* file, vpk::VPK& vpk, char** ext, const std::function<void(size_t)>& fun);

        void getDirectory(FILE* file, vpk::VPK& vpk, char** dir, const std::function<void(size_t)>& fun);

        vpk::DirectoryEntry getDirectoryEntry(FILE* file, vpk::VPK& vpk, char** fnm, const std::function<void(size_t)>& fun);

        void checkTermination(FILE* file, char** ext, char** dir, const std::function<void(size_t)>& fun);



        void extractHeader(FILE* file, vpk::VPK& vpk);

        void extractBody(FILE* file, vpk::VPK& vpk);

        void extractFooter(FILE* file, vpk::VPK& vpk);

        void read(const wchar_t* fileName);

    }
}