#pragma once
#include <ostream>

#include "../lib/common.h"

class output
{
public:
    output(std::ostream& os);
    void put_word(word w);
    void put_byte(word w);  /* [-128, 255] */
    void put_ub(word w);    /* [0, 255] */
    void put_ib(word w);    /* [-128, 127] */
    void put_db(word w);    /* [.-128, .+127] */
    void put_dw(word w);

    static bool is_byte(word w); /* [-128, 255] */
    static bool ub(word w); /* [-128, 127] */
    static bool ib(word w); /* [0, 255] */
    static bool db(word w); /* [.-128, .+127] */
private:
    template<typename T>
    void put(T value)
    {
        _os.write((char*)&value, sizeof(value));
    }

    std::ostream& _os;
};
