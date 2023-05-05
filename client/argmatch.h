#pragma once
#include <vector>

/*
 * usage:
 *      argmatch match(argc, argv, [preargs]);
 *      if (match(<pattern>))
 */

class argmatch
{
public:
    argmatch(int argc, char** argv, std::vector<std::string>&& preargs = {})
    : _i(0), _argc(argc - 1), _argv(argv + 1), _preargs(std::move(preargs))
    {}

    bool done() const { return _i == argc(); }
    const char* at(int n) const { return _at[n]; }

    bool operator()(const char* pattern)
    {
        char* p = arg(_i);
        int adjust = 1;
        _at.clear();
        while (*pattern)
        {
            if (*pattern == '$')
            {
                if (_i + adjust >= argc()) return false;
                _at.push_back(arg(_i + adjust++));
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
            if (*pattern == '\\')
            {
                pattern++;
            }
            if (*p++ != *pattern++) return false;
        }
        _i += adjust;
        return true;
    }

private:
    int argc() const
    {
        return _argc + _preargs.size();
    }

    char* arg(int i)
    {
        return i < _preargs.size() ? _preargs[i].data() : _argv[i - _preargs.size()];
    }

private:
    int _i, _argc;
    char** _argv;
    std::vector<const char*> _at;
    std::vector<std::string> _preargs;
};
