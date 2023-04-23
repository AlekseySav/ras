#include "string.h"
#include <vector>

static size_t hash(const char* s)
{
    size_t i = 0;
    while (*s)
    {
        i = i * SYMTAB_PRIME + *s++ - ' ';
    }
    return i;
}

template<size_t N>
class strtab
{
public:
    strtab() : _buckets(N), _string(1) {}

    size_t capacity() const { return _buckets.size(); }

    uint32_t lookup(const char* name)
    {
        size_t n = hash(name);
        for (size_t i = 0; i < capacity(); i++)
        {
            size_t k = (n + i * i) % capacity();
            uint32_t& e = _buckets[k];
            if (e == 0)
            {
                e = _string.size();
                _string.reserve(strlen(name));
                while (*name)
                {
                    _string.push_back(*name++);
                }
                _string.push_back('\0');
                return k;
            }
            if (!strcmp(string(k).data(), name))
            {
                return k;
            }
        }
        error("string table overflow on string <{}>", name);
        return _buckets[0];
    }

    std::vector<uint32_t> _buckets;
    std::vector<char> _string;
};

static strtab<MAX_SYMBOLS> _strtab;

string::string(uint32_t id) : _id(id)
{}

string::string(const char* s) : _id(_strtab.lookup(s))
{}

string::string(const string& s) : _id(s._id)
{}

const char* string::data() const
{
    return _strtab._string.data() + _strtab._buckets[_id];
}

uint32_t string::id() const
{
    return _id;
}

void string::clear_strtab()
{
    _strtab = strtab<MAX_SYMBOLS>();
}
