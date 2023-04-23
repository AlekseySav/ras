#include "tests.h"

TEST(lexer, basic)
{
    START();
    token t;
    LEXER(lex, "<<>>==!=<=>=!$%&()*+,-/:;< =>?[]^|hello 123\n");

    ASSERT_EQ(lex.get(), L_shl);
    ASSERT_EQ(lex.get(), L_shr);
    ASSERT_EQ(lex.get(), L_equ);
    ASSERT_EQ(lex.get(), L_neq);
    ASSERT_EQ(lex.get(), L_leq);
    ASSERT_EQ(lex.get(), L_geq);
    ASSERT_EQ(lex.get(), '!');
    ASSERT_EQ(lex.get(), '$');
    ASSERT_EQ(lex.get(), '%');
    ASSERT_EQ(lex.get(), '&');
    ASSERT_EQ(lex.get(), '(');
    ASSERT_EQ(lex.get(), ')');
    ASSERT_EQ(lex.get(), '*');
    ASSERT_EQ(lex.get(), '+');
    ASSERT_EQ(lex.get(), ',');
    ASSERT_EQ(lex.get(), '-');
    ASSERT_EQ(lex.get(), '/');
    ASSERT_EQ(lex.get(), ':');
    ASSERT_EQ(lex.get(), ';');
    ASSERT_EQ(lex.get(), '<');
    ASSERT_EQ(lex.get(), '=');
    ASSERT_EQ(lex.get(), '>');
    ASSERT_EQ(lex.get(), '?');
    ASSERT_EQ(lex.get(), '[');
    ASSERT_EQ(lex.get(), ']');
    ASSERT_EQ(lex.get(), '^');
    ASSERT_EQ(lex.get(), '|');
    ASSERT_EQ(t = lex.get(), L_sym);
    ASSERT_SEQ(lval<string>(t), "hello");
    ASSERT_EQ(t = lex.get(), L_num);
    ASSERT_EQ(lval<word>(t), 123);
    ASSERT_EQ(lex.get(), ';');
}
