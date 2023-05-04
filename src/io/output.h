#pragma once
#include <ostream>

#include "../lib/common.h"

class output
{
public:
    output(std::ostream& os) : _os(&os) {}

    /* size=1 */
    void put_byte(word w);  /* [-128, 255] */
    void put_ub(word w);    /* [0, 255] */
    void put_ib(word w);    /* [-128, 127] */

    /* size=2 */
    void put_word(word w);  /* [-32768, 65535] */
    void put_uw(word w);    /* [0, 65535] */
    void put_iw(word w);    /* [-32768, 32767] */

    /* size=4 */
    void put_long(word w);  /* any */

    template<size_t S>
    void put(word value)
    {
        _os->write((char*)&value, S);
    }

private:
    std::ostream* _os;
};
