#include "../src/lib/optional.h"
#include <iostream>

#include "../src/io/lex.h"
#include "../src/expr/expr.h"

int main()
{
    token t;
    lexer lex(std::cin);

    return state::errors != 0;
}
