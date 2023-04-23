#include "tests.h"
#include <array>

TEST(strtab, all)
{
    string::clear_strtab();
    string a = "hello", b = "ac";
    ASSERT_EQ(b.data() - a.data(), 6);
    string::clear_strtab();

    const size_t N = MAX_SYMBOLS / 2;
    std::array<string, N> str;
    int counter = 0;

    for (string& s : str)
        s = std::to_string(counter++).c_str();

    for (size_t i = 0; i < N; i++)
        for (size_t j = i + 1; j < N; j++)
            ASSERT_NE(str[i].id(), str[j].id());

    for (size_t i = 0; i < N; i++)
        ASSERT_EQ(str[i].id(), string(std::to_string(i).c_str()).id());
    
    counter = 0;
    for (string& s : str)
        ASSERT_STREQ(s.data(), std::to_string(counter++).c_str());
}
