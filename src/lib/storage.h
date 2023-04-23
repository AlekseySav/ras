#pragma once
#include <type_traits>

template<typename T>
class storage
{
public:
    T* data()
    {
        return std::launder(reinterpret_cast<T*>(_value.__data));
    }
private:
    std::aligned_storage_t<sizeof(T)> _value;
};
