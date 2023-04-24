#pragma once

#include "error.h"
#include <vector>

template<typename T, size_t N>
class pool
{
public:
    pool() : _size{0} {}

    inline T& alloc()
    {
        fatal(_size >= N, "pool: size exceeded");
        return _pool[_size++];
    }

    inline void push(const T& v)
    {
        alloc() = v;
    }

    inline void push(T&& v)
    {
        alloc() = std::move(v);
    }

    inline T pop()
    {
        panic(_size == 0, "pool: pop from empty pool");
        return _pool[--_size];
    }

    inline T& top()
    {
        panic(_size == 0, "pool: top of empty pool");
        return _pool[_size - 1];
    }

    inline void clear()
    {
        _size = 0;
    }

    inline size_t size() const
    {
        return _size;
    }

    inline auto begin() const
    {
        return typename std::vector<T>::const_iterator(_pool);
    }

    inline auto end() const
    {
        return typename std::vector<T>::const_iterator(_pool + _size);
    }

    T& operator[](size_t i)
    {
        panic(i >= _size, "pool: index out of bounds");
        return _pool[i];
    }

private:
    T _pool[N];
    uint32_t _size;
};
