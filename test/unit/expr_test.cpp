#include "tests.h"

TEST(expr, basic)
{
    expr e;
    START();
    LEXER(lex, "bx, 2(bp), 123, $123;");
    e = expr(lex);
    ASSERT_EQ(e.type().type, A_rw);
    ASSERT_EQ(e.type().n, 3);
    ASSERT_EQ(lex.get(), ',');
    e = expr(lex);
    ASSERT_EQ(e.type().type, A_mm);
    ASSERT_EQ(e.type().n, 6);
    ASSERT_EQ(e.eval(), 2);
    ASSERT_EQ(lex.get(), ',');
    e = expr(lex);
    ASSERT_EQ(e.type().type, A_m0);
    ASSERT_EQ(e.eval(), 123);
    ASSERT_EQ(lex.get(), ',');
    e = expr(lex);
    ASSERT_EQ(e.type().type, A_im);
    ASSERT_EQ(e.eval(), 123);
    ASSERT_EQ(lex.get(), ';');
}

TEST(expr, syms)
{
    expr e1, e2, e3, e4;
    START();
    LEXER(lex, "2(bp), hello+7; $4; hello+9;");
    e1 = expr(lex);
    symbol& s = symbol::lookup("hello");
    s.make_mutable();
    s.assign(e1);
    ASSERT_EQ(lex.get(), ',');
    e2 = expr(lex);
    ASSERT_EQ(lex.get(), ';');
    e3 = expr(lex);
    ASSERT_EQ(lex.get(), ';');
    e4 = expr(lex);
    ASSERT_EQ(lex.get(), ';');
    ASSERT_EQ(e1.type().type, A_mm);
    ASSERT_EQ(e1.type().n, 6);
    ASSERT_EQ(e1.eval(), 2);
    s.update();
    ASSERT_EQ(e2.type().type, A_mm);
    ASSERT_EQ(e2.type().n, 6);
    ASSERT_EQ(e2.eval(), 9);
    s.assign(e3);
    ASSERT_EQ(e3.type().type, A_im);
    ASSERT_EQ(e3.eval(), 4);
    s.update();
    ASSERT_EQ(e2.type().type, A_im);
    ASSERT_EQ(e2.eval(), 11);

    ASSERT_EQ(e4.type().type, A_im);
    ASSERT_EQ(e4.eval(), 4 + 9);
}
