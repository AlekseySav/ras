#pragma once
#include "../lib/common.h"

class string
{
public:
    string() = default;
    string(uint32_t id);
    string(const char* s);
    string(const string& s);

    const char* data() const;
    uint32_t id() const;

    static void clear_strtab();
private:
    uint32_t _id;
};
