#pragma once
#include <fmt/core.h>
#include <fmt/color.h>

#include "common.h"

class Error {};     /* caught by insn parser */
class Fatal {};     /* caught by client */

template<typename F>
bool safe_run(F&& f)
{
    try {
        f();
    }
    catch (Error) {
        return true;
    }
    return false;
}

#define safe_run(code) safe_run([&]{code;})

template<typename... Args>
void trace(Args&&... args)
{
#ifdef _DEBUG
    fmt::print(stderr, "{}    ", fmt::styled("[trace]", fmt::emphasis::bold));
    fmt::println(stderr, std::forward<Args&&>(args)...);
#endif
}

template<typename... Args>
void panic(bool cond, Args&&... args)
{
#ifdef _DEBUG
    if (!cond) return;
    fmt::print(stderr, "{} ", fmt::styled("ras panic:", fmt::emphasis::bold | fg(fmt::color::red)));
    fmt::println(stderr, std::forward<Args&&>(args)...);
    exit(1);
#endif
}

template<typename... Args>
void error(Args&&... args)
{
    state::errors++;
    fmt::print(stderr, "{}:{}: {} ", state::filename, state::line, fmt::styled("error:", fg(fmt::color::red)));
    fmt::println(stderr, std::forward<Args&&>(args)...);
    if (state::errors < MAX_ERRORS)
    {
        throw Error{};
    }
    throw Fatal{};
}

template<typename... Args>
void fatal(Args&&... args)
{
    state::errors++;
    fmt::print(stderr, "{} ", fmt::styled("fatal error:", fmt::emphasis::bold | fg(fmt::color::red)));
    fmt::println(stderr, std::forward<Args&&>(args)...);
    throw Fatal{};
}

template<typename... Args>
void error(bool cond, Args&&... args)
{
    if (cond) error(std::forward<Args&&>(args)...);
}

template<typename... Args>
void fatal(bool cond, Args&&... args)
{
    if (cond) fatal(std::forward<Args&&>(args)...);
}
