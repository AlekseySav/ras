#pragma once
#include <cstdint>
#include <stack>
#include <cstddef>

using byte = uint8_t;
using word = int16_t;

namespace state
{
    inline const char* filename;
    inline int line;
    inline int errors;

    inline std::stack<bool> if_stack;
}

#include "../config.h"
#include "error.h"
