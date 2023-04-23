#pragma once
#include <ostream>

#include "../lib/common.h"

class Output
{
public:
    Output(std::ostream& os);
    void put_word(word w);
    void put_byte(word w);  /* [-128, 255] */
    void put_ub(word w);    /* [0, 255] */
    void put_ib(word w);    /* [-128, 127] */
private:
    template<typename T>
    void put(T value)
    {
        _os.write((char*)&value, sizeof(value));
    }

    std::ostream& _os;
};
