#pragma once

#include "framework.h"
#include "../vpk_structs.h"

#include <vector>

class Window {
public:
    static void create();
    static bool condition(GLFWwindow* window);

    static GLFWwindow* handle_get();
    static void handle_set(GLFWwindow* window);

    static void      vpk_add(const wchar_t* const str, const vpk::VPK& vpk);
    static vpk::VPK& vpk_get(size_t i);
    static void      vpk_removeAll();
private:
    static std::vector<std::wstring> m_names;
    static std::vector<vpk::VPK>     m_vpks;
    static GLFWwindow* m_handle;
};