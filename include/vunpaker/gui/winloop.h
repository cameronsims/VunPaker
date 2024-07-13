#pragma once

#include <vunpaker/gui/framework.h>

namespace vpk {
    namespace gui {
        /// Call function when refreshing
        typedef void(*CreateFunction_t)(void);
        
        /// Conditions to loop values
        typedef bool(*LoopConditionFunction_t)(GLFWwindow*);

        void loop(GLFWwindow* window, CreateFunction_t create, LoopConditionFunction_t condition);
    }
}