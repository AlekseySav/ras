#include "tests.h"

TEST(cexpr, basic)
{
    START();
    cexpr c;
    ASSERT_EQ(c.eval(), 0);
    // 1+3*[5--2]>>1
    c.compile({{L_num, 1}, '+', {L_num, 3}, '*', '[', {L_num, 5}, '-', '-', {L_num, 2}, ']', L_shr, {L_num, 1}});
    ASSERT_EQ(c.eval(), 11);
}
