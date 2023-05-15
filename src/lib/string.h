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

    inline bool operator==(string s) { return id() == s.id(); }
    inline bool operator!=(string s) { return id() != s.id(); }

    static void clear_strtab();
private:
    uint32_t _id;
};
