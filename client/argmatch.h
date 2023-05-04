#pragma once
#include <vector>

/*
 * usage:
 *      argmatch match(argc, argv);
 *      if (match(<pattern>))
 */

class argmatch
{
public:
    argmatch(int argc, char** argv) : _i(1), _argc(argc), _argv(argv)
    {}

    bool done() const { return _i == _argc; }
    const char* at(int n) const { return _at[n]; }

    bool operator()(const char* pattern)
    {
        char* p = _argv[_i];
        int adjust = 1;
        _at.clear();
        while (*pattern)
        {
            if (*pattern == '$')
            {
                if (_i + adjust >= _argc) return false;
                _at.push_back(_argv[_i + adjust++]);
                pattern++;
                continue;
            }
            if (*pattern == '*')
            {
                pattern++;
                _at.push_back(p);
                while (*p != *pattern)
                {
                    if (*p++ == '\0') return false;
                }
                if (*p)
                {
                    *p = '\0';
                    p++, pattern++;
                }
                continue;
            }
            if (*p++ != *pattern++) return false;
        }
        _i += adjust;
        return true;
    }

private:
    int _i, _argc;
    char** _argv;
    std::vector<const char*> _at;
};
