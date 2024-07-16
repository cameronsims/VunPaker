#include <vunpaker/vpk.h>
#include <vunpaker/file.h>

#include <stack>

#include <stdio.h>
#include <stdlib.h>

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

    fun(s.size() + 1);
}

void vpk::file::getExtension(FILE* file, vpk::VPK& vpk, char** ext, const std::function<void(size_t)>& fun) {
    readNextToken(file, vpk, ext, fun);

    #ifdef VPK_PRINT_DIRECTORY
    printf("START OF EXTENSION \"%s\"\n", *ext);
    #endif
}

void vpk::file::getDirectory(FILE* file, vpk::VPK& vpk, char** dir, const std::function<void(size_t)>& fun) {
    readNextToken(file, vpk, dir, fun);

    #ifdef VPK_PRINT_DIRECTORY
    printf("    START OF DIRECTORY \"%s\"\n", *dir);
    #endif
}

vpk::DirectoryEntry vpk::file::getDirectoryEntry(FILE* file, vpk::VPK& vpk, char** fnm, const std::function<void(size_t)>& fun) {
    readNextToken(file, vpk, fnm, fun);

    // Get file entries
    vpk::DirectoryEntry entry;
    entry.crc = get4Bytes(file, vpk);
    entry.preload_bytes = get2Bytes(file, vpk);
    entry.archive_index = get2Bytes(file, vpk);
    entry.entry_offset = get4Bytes(file, vpk);
    entry.entry_length = get4Bytes(file, vpk);

    fun(sizeof(vpk::DirectoryEntry));

    #ifdef VPK_PRINT_DIRECTORY
    printf("        ENTRY(%s) %08X %04X %04X %08X %08X", *fnm, entry.crc, entry.preload_bytes, entry.archive_index, entry.entry_offset, entry.entry_length);
    #endif

    // If there is bytes we need to read...
    if (entry.entry_offset = 0x0000) {
        // How long the file is
        uint32_t it = entry.entry_length;

        // We should add it to the temp file
        while (it != 0) {
            // Read per bytes...
            char byte;
            fscanf_s(file, "%1c", &byte, 1);
            printf("%1c", byte);
            it--;
        }
    }

    return entry;
}

void vpk::file::checkTermination(FILE* file, char** ext, char** dir, const std::function<void(size_t)>& fun) {

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

        delete[] *dir;
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
            delete[] *ext;
            *ext = nullptr;
            fun(1);
        } else {
            // Otherwise, add back
            ungetc(c2, file);
        }

    } else {
        // Add back to stream
        ungetc(c1, file);
    }
}


void vpk::file::extractHeader(FILE* file, vpk::VPK& vpk) {
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
        case 1: {
            // Version one has no specific functionality so we disreguard throw
        } break;

        case 2: {
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
        default: {
            throw vpk::file::UnknownVersion();
        } break;
    }


}

void vpk::file::extractBody(FILE* file, vpk::VPK& vpk) {
    // Constant Bytes
    const char C_NULL = 0x00;
    const char C_MAX = 0xFF;
    const char FOLDER_TERMINATOR[3] = { C_MAX, C_MAX, C_NULL };
    const char TERMINATOR[3] = { C_MAX, C_MAX };

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

        // If it is ok, run it
        it -= bytes;
    };

    while (0 <= it && it <= vpk.header.tree_size) {
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
                lam(strnlen(TERMINATOR, 3) + 1);
                delete[] ext;
                ext = nullptr;
            }
            // If there is no directory
            else if (!dir) {
                getDirectory(file, vpk, &dir, lam);
            }
            // If there is a directory...
            else {
                // And we detect a terminator...
                if (nextIsTerminator(file, TERMINATOR)) {
                    lam(strnlen(TERMINATOR, 3) + 1);
                    delete[] dir;
                    dir = nullptr;
                }
                // If we don't have a file
                else if (!fnm) {
                    // Expect a file
                    vpk::DirectoryEntry entry = getDirectoryEntry(file, vpk, &fnm, lam);

                    // Expect a terminate
                    if (nextIsTerminator(file, TERMINATOR)) {

                        #ifdef VPK_PRINT_DIRECTORY
                        printf("\n");
                        #endif


                        lam(strnlen(TERMINATOR, 2));
                        delete[] fnm;
                        fnm = nullptr;

                        checkTermination(file, &ext, &dir, lam);

                    } else {
                        #ifdef VPK_PRINT_DIRECTORY
                        printf(" - END OF VARIABLE NOT DETECTED!\n");
                        #endif

                        throw vpk::file::UnexpectedToken();
                    }
                }
            }
        }
    }

    // Eat last null
    char c = fgetc(file);
    if (c != C_NULL) {
        throw vpk::file::UnexpectedToken();
    }
}

void vpk::file::extractFooter(FILE* file, vpk::VPK& vpk) {
    
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
        read_backwards(vpk.other.tree_checksum,        THIRDS);
        read_backwards(vpk.other.archive_md5_checksum, THIRDS);
        read_backwards(vpk.other.whole_file_checksum,  THIRDS);
    }

    // Get signature bytes
    if (vpk.header.ss_size != 0) {
        read_backwards(vpk.signature.public_key, vpk.signature.public_key_size);
        read_backwards(vpk.signature.signature,  vpk.signature.signature_size);
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

    printf("\n  SIG:    \n    PUBKEY: %16s (%08x)\n    SIGNAT: %16s (%08x)\n",                               vpk.signature.public_key,                       
                                                                                                           vpk.signature.public_key_size,                
                                                                                                           vpk.signature.signature,       
                                                                                                           vpk.signature.signature_size);
}