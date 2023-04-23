#pragma once
#include <new>
#include <utility>

#include "common.h"
#include "storage.h"

template<typename T>
class optional
{
public:
    optional() : _defined{false} {}

    template<typename... Args>
    optional(Args&&... args) : _defined{true}
    {
        new (_value.data()) T(std::forward<Args&&>(args)...);
    }

    explicit operator bool() const
    {
        return _defined;
    }

    bool defined() const
    {
        return _defined;
    }

    T&& extract() &&
    {
        error(!defined(), "value used before its definition");
        return std::move(*_value.data());
    }
private:
    bool _defined;
    storage<T> _value;
};
