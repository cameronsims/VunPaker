#include <vunpaker/file.h>
#include <vunpaker/vpk.h>

#include <stack>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

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

vpk::file::UnexpectedName::UnexpectedName() { }
const char* vpk::file::UnexpectedName::what() const {
    return "\nUnexpected Name in file\n";
}

uint32_t vpk::file::get2Bytes(FILE* file, vpk::VPK& vpk) {
    constexpr uint32_t SIZE = sizeof(uint16_t);

    // Get 4 bytes worth of information
    uint8_t temp[SIZE] = { 0x00, 0x00 };

    // Get in terms of string
    if (!fscanf_s(file, "%2c", temp, SIZE)) {
        // Unable to read file
        throw vpk::file::UnexpectedToken();
    }

    // Convert bytes to 32-bit signature
    uint32_t value = 0;
    for (size_t i = 0; i < SIZE; i++) {
        value = (value << 8) + temp[SIZE - i - 1];
    }

    // Return the value that was calculated
    return value;
}

uint32_t vpk::file::get4Bytes(FILE* file, vpk::VPK& vpk) {
    // Get 4 bytes worth of information
    uint8_t temp[VPK_SIGNATURE_SIZE] = { 0x00, 0x00, 0x00, 0x00 };

    // Get in terms of string
    if (!fscanf_s(file, "%4c", temp, VPK_SIGNATURE_SIZE)) {
        // Unable to read file
        throw vpk::file::UnexpectedToken();
    }

    // Convert bytes to 32-bit signature
    uint32_t value = 0;
    for (size_t i = 0; i < VPK_SIGNATURE_SIZE; i++) {
        value = (value << 8) + temp[VPK_SIGNATURE_SIZE - i - 1];
    }

    // Return the value that was calculated
    return value;
}

bool vpk::file::nextIsTerminator(FILE* file, const char TERMINATOR[3]) {
    // Check next two characters
    const size_t SIZE = strnlen(TERMINATOR, 3);
    std::stack<char> s;

    // Bool if correct
    // size_t i = 0;
    bool correct = true;
    for (size_t i = 0; i < SIZE; i++) {
        char c = fgetc(file);
        s.push(c);
        correct = correct && (c == TERMINATOR[i]);
    }

    // Return if false
    if (correct) {
        return true;
    }

    // If it is, loop
    while (!s.empty()) {
        ungetc(s.top(), file);
        s.pop();
    }
    return correct;
}

void vpk::file::readNextToken(FILE* file, vpk::VPK& vpk, char** str, const std::function<void(size_t)>& fun) {

    // Read until we get a String
    std::string s;
    char c = 0xFF;
    // Read until we find a null
    while (c != '\0') {
        // Add to back of file
        c = fgetc(file);
        s.push_back(c);
    }

    // We now have the values
    // The format we're reading
    // const char* FOMRAT = "%255";
    const size_t SIZE = s.size();
    (*str) = new char[SIZE];
    (*str)[0] = '\0';
    (*str)[SIZE - 1] = '\0';
    //
    //printf("%s - ", s.c_str());
    strncpy(*str, s.c_str(), SIZE);
    // 
    // int status = fscanf_s(file, format, str, SIZE);
    // if (status == 0 || status == EOF) {
    //     throw vpk::file::UnexpectedToken();
    // }

    fun(SIZE);
}

void vpk::file::getExtension(FILE* file, vpk::VPK& vpk, char** ext, const std::function<void(size_t)>& fun) {
    readNextToken(file, vpk, ext, fun);

#ifdef VPK_PRINT_DIRECTORY
    printf("START OF EXTENSION \"%s\"\n", *ext);
#endif
}

void vpk::file::getDirectory(FILE* file, vpk::VPK& vpk, const wchar_t* const directory, char** dir, const std::function<void(size_t)>& fun) {
    readNextToken(file, vpk, dir, fun);

    #ifdef VPK_PRINT_DIRECTORY
    printf("    START OF DIRECTORY \"%s\"\n", *dir);
    #endif
}

vpk::DirectoryEntry vpk::file::getDirectoryEntry(FILE* file, vpk::VPK& vpk, const wchar_t* const directory, const char* const ext, const char* const dir, char** fnm, const std::function<void(size_t)>& fun) {
    readNextToken(file, vpk, fnm, fun);

    // If the size has no real characters...
    if (strnlen(*fnm, _MAX_FNAME) == 0) {
        // Ignore the rest of the script and return
        throw UnexpectedName();
    }

#ifdef VPK_PRINT_DIRECTORY
    printf("        ENTRY(%s) ", *fnm);
#endif

    // Get file entries
    vpk::DirectoryEntry entry;
    entry.crc = get4Bytes(file, vpk);
    entry.preload_bytes = get2Bytes(file, vpk);
    entry.archive_index = get2Bytes(file, vpk);
    entry.entry_offset = get4Bytes(file, vpk);
    entry.entry_length = get4Bytes(file, vpk);
    fun(sizeof(vpk::DirectoryEntry) + 2*sizeof(char));

    #ifdef VPK_PRINT_DIRECTORY
    printf("%08X %04X %04X %08X %08X", entry.crc, entry.preload_bytes, entry.archive_index, entry.entry_offset, entry.entry_length);
    #endif

    std::wofstream temp;

    {
        // Get Directory
        wchar_t fileLoc[_MAX_PATH] = L"";

        // Convert extension, directory and filename to wchar
        wchar_t wext[_MAX_EXT] = L"",
                wdir[_MAX_DIR] = L"",
                wfnm[_MAX_FNAME] = L"",
                path[_MAX_PATH] = L"";

        mbstowcs(wext, ext, _MAX_EXT);
        mbstowcs(wdir, dir, _MAX_DIR);
        mbstowcs(wfnm, *fnm, _MAX_FNAME);

        // Copy over
        swprintf(path, _MAX_PATH, L"%ls/%ls", directory, wdir);

        // Check last
        const size_t pathn = wcslen(path);
        if (path[pathn - 1] == 0x20) {
            path[pathn - 2] = '\0';
        }

        if (ext[0] != 0x20) {
            swprintf(fileLoc, _MAX_PATH, L"%ls/%ls.%ls", path, wfnm, wext);
        } else {
            swprintf(fileLoc, _MAX_PATH, L"%ls/%ls", path, wfnm);
        }

        // Temporary array for file
        wchar_t tempath[_MAX_PATH] = L"";

        // Set files
        wprintf(L"%ls\n", path);
        const size_t n = wcslen(path);
        for (size_t i = wcslen(directory); i < n; i++) {
            // If end of file
            if (path[i] == '\\' || path[i] == '/' || i == n - 1) {
                // Copy over
                const size_t end = (i == n - 1) ? n : i;
                wcsncpy(tempath, path, end);
                tempath[end] = '\0';
                wprintf(L"  [%ls]\n", tempath);

                if (_wmkdir(tempath) != 0 && errno != EEXIST) {
                    wprintf(L"\nCannot open file \"%ls\"\n", tempath);
                    throw vpk::file::NotFound();
                }

                tempath[0] = '\0';
            }

        }

        // Open a temperary file
        temp = std::wofstream( std::wstring(fileLoc) );
        if (temp.bad() || !temp.is_open()) {
            wprintf(L"\nCannot open file\n");
            throw vpk::file::NotFound();
        }

        // Print name of file in file
        temp << fileLoc;

        // END FILE STREAM
    }


    //lam(TERMINATOR_SIZE);

    // Before we delete anything...
    // Check if we have any preload data
    if (entry.preload_bytes != 0) {
        // The iterator
        uint16_t it2 = entry.preload_bytes;
        printf("READING PRELOAD BYTES:\n");

        // Read this data...
        while (it2 != 0) {
            // Read one byte
            int8_t byte;
            if (fscanf(file, "%1c", &byte) == EOF) {
                throw file::UnexpectedToken();
            }
            // Add byte to file
            printf("%c", byte);
            temp << byte;
            it2--;
        }
    }



    // If there is bytes we need to read...
    if (entry.entry_offset = 0x0000) {
        // How long the file is
        uint32_t it = entry.entry_length;

        // We should add it to the temp file
        while (it != 0) {
            // Read per bytes...
            int8_t byte;
            if (!fscanf_s(file, "%1c", &byte, 1))
                throw UnexpectedToken();
            printf("%1c", byte);
            it--;
        }
    }

    return entry;
}

vpk::file::TerminationType vpk::file::checkTermination(FILE* file, char** ext, char** dir, const std::function<void(size_t)>& fun) {

    // Constant Bytes
    const char C_NULL = 0x00;
    const char C_MAX = 0xFF;

    // If next character is 0
    char c1 = fgetc(file);
    if (c1 == EOF) {
        printf("ERROR: END OF VPK FILE\n");
        throw vpk::file::UnexpectedToken();
    }

    if (c1 == C_NULL) {
        // Set all variables to nullptrs (besides extension
        #ifdef VPK_PRINT_DIRECTORY
        printf("    END OF DIRECTORY \"%s\"\n", *dir);
        #endif

        delete[] * dir;
        *dir = nullptr;
        fun(1);

        // If the next is also 0x00...
        char c2 = fgetc(file);
        if (c2 == EOF) {
            printf("ERROR: END OF VPK FILE\n");
            throw vpk::file::UnexpectedToken();
        }
        if (c2 == C_NULL) {
            #ifdef VPK_PRINT_DIRECTORY
            printf("END OF EXTENSION \"%s\"\n", *ext);
            #endif
            delete[] * ext;
            *ext = nullptr;
            fun(1);

            // Check if it is the end of a segment
            char c3 = fgetc(file);
            if (c3 == EOF) {
                printf("ERROR: END OF VPK FILE\n");
                throw vpk::file::UnexpectedToken();
            }
            if (c3 == C_NULL) {
                fun(1);
                printf("END OF TREE\n");
                return vpk::file::TerminationType::TREE;
            } else {
                ungetc(c3, file);
                return vpk::file::TerminationType::EXTENSION;
            }

        } else {
            // Otherwise, add back
            ungetc(c2, file);
            return vpk::file::TerminationType::DIRECTORY;
        }

    } else {
        // Add back to stream
        ungetc(c1, file);
    }

    return vpk::file::TerminationType::NONE;
}

void vpk::file::extractHeader(FILE* file, vpk::VPK& vpk, const wchar_t* directory) {
    // Get signature and version... (First 5 bits [0,1,2,3,4] of the file)
    vpk::Header_v2& header = vpk.header;

    printf("VPK Header:\n");
    // Signature
    // 0x34 0x12 0xaa 0x55 
    header.signature = get4Bytes(file, vpk);
    printf("  SIGNATURE:      0x%08X\n", vpk.header.signature);

    // Version
    // 0x02/0x01 0x00 0x00 0x00
    header.version = get4Bytes(file, vpk);
    printf("  VPK VERSION:    0x%08X [Version %u]\n", vpk.header.version, vpk.header.version);

    // Tree Size
    // 0x?? 0x?? 0x?? 0x??
    header.tree_size = get4Bytes(file, vpk);
    printf("  TREE SIZE:      0x%08X [%u Bytes]\n", vpk.header.tree_size, vpk.header.tree_size);

    // Convert to proper value
    switch (header.version) {
        case 1:
        {
            // Version one has no specific functionality so we disreguard throw
        } break;

        case 2:
        {
            // Do things specific to the second version of VPK

            // Data Size
            // 0x?? 0x?? 0x?? 0x??
            header.ds_size = get4Bytes(file, vpk);
            printf("  DATA SIZE:      0x%08X [%u Bytes]\n", vpk.header.ds_size, vpk.header.ds_size);

            // Archive Size
            // 0x?? 0x?? 0x?? 0x??
            header.as_size = get4Bytes(file, vpk);
            printf("  ARCHIVE SIZE:   0x%08X [%u Bytes]\n", vpk.header.as_size, vpk.header.as_size);

            // Other Size
            // 0x?? 0x?? 0x?? 0x??
            header.os_size = get4Bytes(file, vpk);
            printf("  OTHER SIZE:     0x%08X [%u Bytes]\n", vpk.header.os_size, vpk.header.os_size);

            // Signature Size
            // 0x28 0x01 0x00 0x00
            header.ss_size = get4Bytes(file, vpk);
            printf("  SIGNATURE SIZE: 0x%08X [%u Bytes]\n", vpk.header.ss_size, vpk.header.ss_size);
        } break;

        // No other versions
        default:
        {
            throw vpk::file::UnknownVersion();
        } break;
    }


}

void vpk::file::extractBody(FILE* file, vpk::VPK& vpk, const wchar_t* directory) {
    // Constant Bytes
    const char C_NULL = 0x00;
    const char C_MAX = 0xFF;

    // TREE TERMINATOR: 0xFF 0xFF 0x00 0x00 0x00
    // FOLDER TERMINAOTR: 0xFF 0xFF 0x00
    // FILE TERMINATOR: 0xFF 0xFF

    const char FOLDER_TERMINATOR[3] = { C_MAX, C_MAX, C_NULL };
    const char TERMINATOR[3] = { C_MAX, C_MAX };
    const size_t TERMINATOR_SIZE = 3;// strnlen(TERMINATOR, 3);

    char* ext = nullptr;    // Extension
    char* dir = nullptr;    // Directory
    char* fnm = nullptr;    // File Name

    // These are the bytes left 
    uint32_t it = vpk.header.tree_size; // Decrease when reading from file

    // This lambda is called every single time we take bytes
    auto lam = [&](size_t bytes) {
        // Set value
        uint32_t BYTES_LEFT = it;
        if (BYTES_LEFT < it) {
            throw vpk::file::UnexpectedToken();
        }

        if (BYTES_LEFT == 0) {
            return;
        }

        if (BYTES_LEFT < bytes) {
            throw vpk::file::UnexpectedToken();
        }

        // If it is ok, run it
        it -= bytes;
        //printf("\n[%u - %u = %u] : ", BYTES_LEFT, bytes, it);
    };

    while (it != 0 || it > vpk.header.tree_size) {
        const uint32_t BYTES_LEFT = it;

        // If there is no extension...
        if (!ext) {
            // Get value
            getExtension(file, vpk, &ext, lam);
        }

        // If there IS an extension
        else {
            // If we detect a terminator character
            if (nextIsTerminator(file, TERMINATOR)) {
                lam(TERMINATOR_SIZE);
                delete[] ext;
                ext = nullptr;
            }
            // If there is no directory
            else if (!dir) {
                getDirectory(file, vpk, directory, &dir, lam);
                
            }
            // If there is a directory...
            else {
                // And we detect a terminator...
                if (nextIsTerminator(file, TERMINATOR)) {
                    lam(TERMINATOR_SIZE);
                    delete[] dir;
                    dir = nullptr;
                }
                // If we don't have a file
                else if (!fnm) {

                    // Catch incase of bad name
                    try {
                        // Expect a file
                        vpk::DirectoryEntry entry = getDirectoryEntry(file, vpk, directory, ext, dir, &fnm, lam);

                        // Expect a terminate
                        if (nextIsTerminator(file, TERMINATOR)) {

                            #ifdef VPK_PRINT_DIRECTORY
                            printf("\n");
                            #endif

                            delete[] fnm;
                            fnm = nullptr;

                            
                            if (checkTermination(file, &ext, &dir, lam) == vpk::file::TerminationType::TREE) {
                                break;
                            }

                        } else {

                            #ifdef VPK_PRINT_DIRECTORY
                            printf(" - END OF VARIABLE NOT DETECTED!\n");
                            #endif

                            throw vpk::file::UnexpectedToken();
                        }

                    } catch (const vpk::file::UnexpectedName& e) {
                        // We shouldn't need anything to do besides catch and throw two characters.
                        
                        // Account for the value that was accidently placed in
                        ungetc(0x00, file);
                        it++;
                        if (fnm != NULL) {
                            delete[] fnm;
                            fnm = nullptr;
                        }
                        
                        // Now print the amount of bytes left
                        printf("%u bytes left\n", it);

                        // Check what level of termination it was
                        if (checkTermination(file, &ext, &dir, lam) == vpk::file::TerminationType::TREE)
                            break;

                        
                    }
                }
            }
        }
    }

}

void vpk::file::extractFooter(FILE* file, vpk::VPK& vpk, const wchar_t* directory) {

    auto read_backwards = [&](char* const dest, size_t size) {
        for (size_t i = size; i != 0; i--) {
            fscanf_s(file, "%1c", &dest[i - 1], 1);
        }
    };

    auto print_hex = [&](const char* const str, size_t size) {
        for (size_t i = size; i != 0; i--) {
            printf("%02X", (uint8_t)str[i - 1]);
        }
    };

    // Get bytes associated with the archive section
    if (vpk.header.as_size != 0) {
        vpk.archive.archive_index = get4Bytes(file, vpk);
        vpk.archive.archive_offset = get4Bytes(file, vpk);
        vpk.archive.archive_size = get4Bytes(file, vpk);
        read_backwards(vpk.archive.md5_checksum, 16);
    }

    // This value should equal 16, due to 48 being the expected value...
    size_t THIRDS = vpk.header.os_size / 3;

    // Get other / misc. bytes
    if (vpk.header.os_size != 0) {
        // We need to read backwards, so loop backwards
        read_backwards(vpk.other.tree_checksum, THIRDS);
        read_backwards(vpk.other.archive_md5_checksum, THIRDS);
        read_backwards(vpk.other.whole_file_checksum, THIRDS);
    }

    // Get signature bytes
    if (vpk.header.ss_size != 0) {
        read_backwards(vpk.signature.public_key, vpk.signature.public_key_size);
        read_backwards(vpk.signature.signature, vpk.signature.signature_size);
    }

    printf("VPK Footer:\n");
    printf("  ARCHIVE:\n    INDEX:  0x%08X   \n    OFFSET: 0x%08X   \n    SIZE:   0x%08X\n    CHKSUM: %s\n", vpk.archive.archive_index,
                                                                                                           vpk.archive.archive_offset,
                                                                                                           vpk.archive.archive_size,
                                                                                                           vpk.archive.md5_checksum);

    printf("  OTHER:\n");

    // PRINT TREECHECKSUM
    printf("    TRECHK: 0x");
    print_hex(vpk.other.tree_checksum, THIRDS);

    // Print MD5 Checkum
    printf("\n    MD5CHK: 0x");
    print_hex(vpk.other.archive_md5_checksum, THIRDS);

    // Print File Checksum
    printf("\n    FILCHK: 0x");
    print_hex(vpk.other.whole_file_checksum, THIRDS);

    printf("\n  SIG:    \n    PUBKEY: %16s (%08x)\n    SIGNAT: %16s (%08x)\n", vpk.signature.public_key,
                                                                               vpk.signature.public_key_size,
                                                                               vpk.signature.signature,
                                                                               vpk.signature.signature_size);
}

void vpk::file::extractFileName(const wchar_t* newDirectory, const wchar_t* fileLocation, wchar_t* buffer) {
    // To be used in strcpy
    const size_t SIZE = wcsnlen(fileLocation, _MAX_PATH);
    size_t start = 0, end = 0;

    // Read through the string
    for (size_t i = 0; i < SIZE; i++) {
        // If it is apart of the directory
        if (fileLocation[i] == '\\' || fileLocation[i] == '/') {
            start = i + 1;
        }
        // If apart of the extension
        else if (fileLocation[i] == '.') {
            end = i;
        }
    }

    // This should never happen
    if (start == 0) {
        throw vpk::file::NotFound();
    }
    
    // Seperate
    const size_t OFFSET = start;
    const size_t AMOUNT = (end > start) ? (end - start) : (SIZE - start);
    const wchar_t* NAME_LOCATION = &fileLocation[OFFSET];
    
    //wchar_t directory_path[_MAX_PATH] = L"";
    wchar_t file_name[_MAX_FNAME] = L"";
    
    //wcsncpy(directory_path, FILE_NAME, start);
    //wprintf(L"%ls\n", NAME_LOCATION);
    
    // If there is an extension
    //if (end > start) {
        //wchar_t ext_name[_MAX_EXT] = L"";
        wcsncpy(file_name, fileLocation + OFFSET, AMOUNT);
        //wcsncpy(ext_name, FILE_NAME + (end + 1)*sizeof(wchar_t), SIZE - end - 1);
    //}
    // If there is no extension
    // else {
        //(file_name, fileLocation + OFFSET, AMOUNT);
    //}

    // Read Directory
    _swprintf(buffer, L"%ls/%ls", newDirectory, file_name);
    //wprintf(L"%ls\n", buffer);
}

void vpk::file::read(const wchar_t* fileName, const wchar_t* directory) {
    // Before we read...
    // Check if the file is in our program already
    if (Window::file_exists(fileName)) {
        // Break and do not mention
        return;
    }
    
    // VPK Input
    FILE* input;
    _wfopen_s(&input, fileName, L"rb");
    if (!input) {
        throw vpk::file::NotFound();
    }

    // Create a file...
    if (_wmkdir(directory) != 0 && errno != EEXIST) {
        throw vpk::file::NotFound();
    }

    // Read VPK Type
    vpk::VPK vpk{};

    // Read 
    vpk::file::extractHeader(input, vpk, directory);

    vpk::file::extractBody(input, vpk, directory);

    if (vpk.header.version == 2)
        vpk::file::extractFooter(input, vpk, directory);

    Window::vpk_add(fileName, vpk);

    fclose(input);
}