#pragma once
#include <cstdint>
#include <stack>
#include <cstddef>

using byte = uint8_t;
using word = int32_t;

namespace state
{
    inline const char* filename;
    inline int line;
    inline int errors;

    inline std::stack<bool> if_stack;

    inline bool assert_defined; // expr.eval() succeeds only if expr is defined
    inline bool unoptim; // -U specified
}

#include "../config.h"
#include "error.h"
