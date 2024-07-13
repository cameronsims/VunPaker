#include <vunpaker/vpk.h>
#include <vunpaker/file.h>

#include <vunpaker/gui/framework.h>
#include <vunpaker/gui/glfw.h>
#include <vunpaker/gui/imgui.h>
#include <vunpaker/gui/winloop.h>

#include <stack>
#include <functional>

#include <stdio.h>
#include <stdlib.h>

bool window_condition(GLFWwindow*);
void window_create();

uint32_t vpk_get2Bytes(const std::wstring& fileName, FILE* file, vpk::VPK& vpk) {
    constexpr uint32_t SIZE = sizeof(uint16_t);

    // Get 4 bytes worth of information
    uint8_t temp[SIZE] = { 0x00, 0x00 };

    // Get in terms of string
    if (!fscanf_s(file, "%2c", temp, SIZE)) {
        // Unable to read file
        throw vpk::FileUnexpectedToken();
    }

    // Convert bytes to 32-bit signature
    uint32_t value = 0;
    for (size_t i = 0; i < SIZE; i++) {
        value = (value << 8) + temp[SIZE - i - 1];
    }

    // Return the value that was calculated
    return value;
}

uint32_t vpk_get4Bytes(const std::wstring& fileName, FILE* file, vpk::VPK& vpk) {
    // Get 4 bytes worth of information
    uint8_t temp[VPK_SIGNATURE_SIZE] = { 0x00, 0x00, 0x00, 0x00 };

    // Get in terms of string
    if (!fscanf_s(file, "%4c", temp, VPK_SIGNATURE_SIZE)) {
        // Unable to read file
        throw vpk::FileUnexpectedToken();
    }

    // Convert bytes to 32-bit signature
    uint32_t value = 0;
    for (size_t i = 0; i < VPK_SIGNATURE_SIZE; i++) {
        value = (value << 8) + temp[VPK_SIGNATURE_SIZE - i - 1];
    }

    // Return the value that was calculated
    return value;
}

bool vpk_nextIsTerminator(FILE* file, const char TERMINATOR[3]) {
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

void vpk_extractHeader(const std::wstring& fileName, FILE* file, vpk::VPK& vpk) {
    // Get signature and version... (First 5 bits [0,1,2,3,4] of the file)
    vpk::VPKHeader_v2& header = vpk.header;

    printf("VPK Header:\n");
    // Signature
    // 0x34 0x12 0xaa 0x55 
    header.signature = vpk_get4Bytes(fileName, file, vpk);
    printf("  SIGNATURE:      0x%08X\n", vpk.header.signature);

    // Version
    // 0x02/0x01 0x00 0x00 0x00
    header.version = vpk_get4Bytes(fileName, file, vpk);
    printf("  VPK VERSION:    0x%08X [Version %u]\n", vpk.header.version, vpk.header.version);

    // Tree Size
    // 0x?? 0x?? 0x?? 0x??
    header.tree_size = vpk_get4Bytes(fileName, file, vpk);
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
            header.ds_size = vpk_get4Bytes(fileName, file, vpk);
            printf("  DATA SIZE:      0x%08X [%u Bytes]\n", vpk.header.ds_size, vpk.header.ds_size);

            // Archive Size
            // 0x?? 0x?? 0x?? 0x??
            header.as_size = vpk_get4Bytes(fileName, file, vpk);
            printf("  ARCHIVE SIZE:   0x%08X [%u Bytes]\n", vpk.header.as_size, vpk.header.as_size);

            // Other Size
            // 0x?? 0x?? 0x?? 0x??
            header.os_size = vpk_get4Bytes(fileName, file, vpk);
            printf("  OTHER SIZE:     0x%08X [%u Bytes]\n", vpk.header.os_size, vpk.header.os_size);

            // Signature Size
            // 0x28 0x01 0x00 0x00
            header.ss_size = vpk_get4Bytes(fileName, file, vpk);
            printf("  SIGNATURE SIZE: 0x%08X [%u Bytes]\n", vpk.header.ss_size, vpk.header.ss_size);
        } break;

        // No other versions
        default: {
            throw vpk::FileUnknownVersion();
        } break;
    }

    
}

void vpk_readNextToken(const std::wstring& fileName, FILE* file, vpk::VPK& vpk, char** str, const std::function<void(size_t)>& fun) {

    // Read until we get a String
    std::string s;
    char c = 0xFF;
    // Read until we find a null
    while (c != '\0') {
        // Add to back of file
        c = fgetc(file);
        s.push_back( c );
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
    //     throw vpk::FileUnexpectedToken();
    // }

    fun(s.size() + 1);
}

void vpk_getExtension     (const std::wstring& fileName, FILE* file, vpk::VPK& vpk, char** ext, const std::function<void(size_t)>& fun) {
    vpk_readNextToken(fileName, file, vpk, ext, fun);
    printf("START OF EXTENSION \"%s\"\n", *ext);
}

void vpk_getDirectory     (const std::wstring& fileName, FILE* file, vpk::VPK& vpk, char** dir, const std::function<void(size_t)>& fun) {
    vpk_readNextToken(fileName, file, vpk, dir, fun);
    printf("    START OF DIRECTORY \"%s\"\n", *dir);
}

void vpk_getDirectoryEntry(const std::wstring& fileName, FILE* file, vpk::VPK& vpk, char** fnm, const std::function<void(size_t)>& fun) {
    vpk_readNextToken(fileName, file, vpk, fnm, fun);

    // Get file entries
    vpk::VPKDirectoryEntry entry;
    entry.crc = vpk_get4Bytes(fileName, file, vpk);
    entry.preload_bytes = vpk_get2Bytes(fileName, file, vpk);
    entry.archive_index = vpk_get2Bytes(fileName, file, vpk);
    entry.entry_offset = vpk_get4Bytes(fileName, file, vpk);
    entry.entry_length = vpk_get4Bytes(fileName, file, vpk);

    fun(sizeof(vpk::VPKDirectoryEntry));
    printf("        ENTRY(%s) %08X %04X %04X %08X %08X", *fnm, entry.crc, entry.preload_bytes, entry.archive_index, entry.entry_offset, entry.entry_length);
}

void vpk_extractBody(const std::wstring& fileName, FILE* file, vpk::VPK& vpk) {
    const char FOLDER_TERMINATOR[3] = { 0xFF, 0xFF, 0x00 };
    const char TERMINATOR[3] = { 0xFF, 0xFF };

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
            throw vpk::FileUnexpectedToken();
        }

        // If it is ok, run it
        it -= bytes;
    };

    while (0 <= it && it <= vpk.header.tree_size) {
        const uint32_t BYTES_LEFT = it;

        // If there is no extension...
        if (!ext) {
            // Get value
            vpk_getExtension(fileName, file, vpk, &ext, lam);
        }
        
        // If there IS an extension
        else {
            // If we detect a terminator character
            if (vpk_nextIsTerminator(file, TERMINATOR)) {
                lam(strnlen(TERMINATOR, 3) + 1);
                delete[] ext;
                ext = nullptr;
            }
            // If there is no directory
            else if (!dir) {
                vpk_getDirectory(fileName, file, vpk, &dir, lam);
            }
            // If there is a directory...
            else {
                // And we detect a terminator...
                if (vpk_nextIsTerminator(file, TERMINATOR)) {
                    lam(strnlen(TERMINATOR, 3) + 1);
                    delete[] dir;
                    dir = nullptr;
                }
                // If we don't have a file
                else if (!fnm) {
                    // Expect a file
                    vpk_getDirectoryEntry(fileName, file, vpk, &fnm, lam);

                    // Expect a terminate
                    if (vpk_nextIsTerminator(file, TERMINATOR)) {
                        printf("\n");
                        lam(strnlen(TERMINATOR, 2));

                        delete[] fnm;
                        fnm = nullptr;

                        // If next character is 0
                        char c1 = fgetc(file);
                        if (c1 == EOF) {
                            printf("ERROR: END OF VPK FILE\n");
                            throw vpk::FileUnexpectedToken();
                        }

                        if (c1 == 0x00) {
                            // Set all variables to nullptrs (besides extension
                            printf("    END OF DIRECTORY \"%s\"\n", dir);
                            delete[] dir;
                            dir = nullptr;
                            lam(1);

                            // If the next is also 0x00...
                            char c2 = fgetc(file);
                            if (c2 == EOF) {
                                printf("ERROR: END OF VPK FILE\n");
                                throw vpk::FileUnexpectedToken();
                            }
                            if (c2 == 0x00) {
                                printf("END OF EXTENSION \"%s\"\n", ext);
                                delete[] ext;
                                ext = nullptr;
                                lam(1);
                            } else {
                                // Otherwise, add back
                                ungetc(c2, file);
                            }

                        } else {
                            // Add back to stream
                            ungetc(c1, file);
                        }


                    } else {
                        printf(" - END OF VARIABLE NOT DETECTED!\n");
                        throw vpk::FileUnexpectedToken();
                    }
                }
            }
        }
    }

}

int wmain(size_t argc, wchar_t** argv) {
    // Initalise GLFW
    vpk::gui::glfw_init();

    // Create window with graphics context
    GLFWwindow* window = vpk::gui::imgui_init();
    
    // Read input
    if (argc > 1) {
        FILE* f;
        fopen_s(&f, "./log.txt", "w");
        if (!f) {
            throw vpk::FileNotFound();
        }

        // For all files
        for (size_t i = 0; i < argc - 1; i++) {
            wchar_t* FILE_NAME = argv[i + 1];

            // VPK Input
            FILE* input;
            _wfopen_s(&input, FILE_NAME, L"rb");
            

            if (!input) {
                throw vpk::FileNotFound();
            }

            printf("%ls\n", FILE_NAME);

            // Wait until a null terminator has been found...

            // 0x00 0x00 FILETYPE.DIRECTORYTREE/NAME INFO 0xFF 0xFF
            const char START = 0x00;
            const char END = 0xFF;

            // Read VPK Type
            vpk::VPK vpk{};
            
            vpk_extractHeader(FILE_NAME, input, vpk);
            vpk_extractBody(FILE_NAME, input, vpk);
            // Extract foot

            fclose(input);
        }
        
        fclose(f);
    }
    
    

    vpk::gui::loop(window, window_create, window_condition);

    // Cleanup
    vpk::gui::imgui_close();
    vpk::gui::glfw_close(window);

    return 0;
}





bool window_condition(GLFWwindow* window) {
    return !glfwWindowShouldClose(window);
}

void window_create() {
    ImGui::Begin(VUNPAKER_CHILD_NAME);
    if (ImGui::TreeNode("root")) {
        ImGui::Text("contents");
        ImGui::TreePop();
    }
    ImGui::End();
}





// These are in the root source file due to external symbols being painful...
// They have been placed at the bottom to make the file look prettier
#include "vunpaker/gui/imgui.cpp"
#include "vunpaker/gui/glfw.cpp"
#include "vunpaker/gui/winloop.cpp"