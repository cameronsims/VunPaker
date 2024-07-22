#pragma once

#include "framework.h"
#include "../vpk_structs.h"

#include <vector>

class Window {
public:
    static void create();
    static bool condition(GLFWwindow* window);

    static void directory_get(wchar_t* directory);
    static void directory_set(const wchar_t* const directory);

    static GLFWwindow* handle_get();
    static void handle_set(GLFWwindow* window);

    static void      vpk_add(const wchar_t* const str, const vpk::VPK& vpk);
    static vpk::VPK& vpk_get(size_t i);
    static void      vpk_removeAll();

    static bool file_exists(const wchar_t* const name);
private:
    static std::vector<std::wstring> m_names;
    static std::vector<vpk::VPK>     m_vpks;
    static wchar_t m_directory[_MAX_PATH];
    static GLFWwindow* m_handle;
};