#include "tests.h"

TEST(insn, assign)
{
    START();
    LEXER(lex, "i = $20+5; j = i+1; a:");
    ref<insn> i = assign_insn(lex);
    ASSERT_TRUE(i->update());
    ASSERT_EQ(symbol::lookup("i").type.type, A_im);
    ASSERT_EQ(symbol::lookup("i").value, 25);
    ASSERT_FALSE(i->update());
    ref<insn> j = assign_insn(lex);
    ASSERT_TRUE(j->update());
    ASSERT_EQ(symbol::lookup("j").type.type, A_im);
    ASSERT_EQ(symbol::lookup("j").value, 26);
    ASSERT_FALSE(j->update());
    symbol::lookup("i").value = 1;
    ASSERT_TRUE(j->update());
    ASSERT_EQ(symbol::lookup("j").type.type, A_im);
    ASSERT_EQ(symbol::lookup("j").value, 2);
    dot.value = 90;
    ref<insn> a = label_insn(lex);
    ASSERT_TRUE(a->update());
    ASSERT_EQ(symbol::lookup("a").type.type, A_m0);
    ASSERT_EQ(symbol::lookup("a").value, 90);
    dot.value = 9;
    ASSERT_EQ(symbol::lookup("a").type.type, A_m0);
    ASSERT_EQ(symbol::lookup("a").value, 90);
    ASSERT_TRUE(a->update());
    ASSERT_EQ(symbol::lookup("a").type.type, A_m0);
    ASSERT_EQ(symbol::lookup("a").value, 9);
}

TEST(insn, ifelse)
{
    START();
    LEXER(lex,
"/*.mut*/ i, j, k;"
"i=0;j=0;k=0;"
"/*.if*/ 1;"
"   i = 1;"
"   j = 1;"
"/*.if*/ k!=0;"
"   k = 2;"
"/*.else*/"
"   k = 3;"
"/*.endif*/"
"/*.else*/"
"/*.if*/ 2;"
"/*.error*/ <.if 2 error>"
"/*.endif*/"
"   i = 2;"
"/*.endif*/"
"   j = 2;"
);
    ref<insn> mut = pseudo_mut(lex);
    ref<insn> i_0 = assign_insn(lex);
    ref<insn> j_0 = assign_insn(lex);
    ref<insn> k_0 = assign_insn(lex);
    ref<insn> if_1 = pseudo_if(lex);
    ref<insn> i_1 = assign_insn(lex);
    ref<insn> j_1 = assign_insn(lex);
    ref<insn> if_k = pseudo_if(lex);
    ref<insn> k_2 = assign_insn(lex);
    ref<insn> else_k = pseudo_else(lex);
    ref<insn> k_3 = assign_insn(lex);
    ref<insn> endif_k = pseudo_endif(lex);
    ref<insn> else_1 = pseudo_else(lex);
    ref<insn> if_2 = pseudo_if(lex);
    ref<insn> err = pseudo_error(lex);
    ref<insn> endif_2 = pseudo_endif(lex);
    ref<insn> i_2 = assign_insn(lex);
    ref<insn> endif_1 = pseudo_endif(lex);
    ref<insn> j_2 = assign_insn(lex);
    update_insn(mut);
    update_insn(i_0);
    update_insn(j_0);
    update_insn(k_0);
    ASSERT_EQ(symbol::lookup("i").value, 0);
    ASSERT_EQ(symbol::lookup("j").value, 0);
    ASSERT_EQ(symbol::lookup("k").value, 0);
    update_insn(if_1);
    update_insn(i_1);
    update_insn(j_1);
    ASSERT_EQ(symbol::lookup("i").value, 1);
    ASSERT_EQ(symbol::lookup("j").value, 1);
    ASSERT_EQ(symbol::lookup("k").value, 0);
    update_insn(if_k);
    update_insn(k_2);
    ASSERT_EQ(symbol::lookup("i").value, 1);
    ASSERT_EQ(symbol::lookup("j").value, 1);
    ASSERT_EQ(symbol::lookup("k").value, 0);
    update_insn(else_k);
    update_insn(k_3);
    update_insn(endif_k);
    ASSERT_EQ(symbol::lookup("i").value, 1);
    ASSERT_EQ(symbol::lookup("j").value, 1);
    ASSERT_EQ(symbol::lookup("k").value, 3);
    update_insn(else_1);
    update_insn(if_2);
    update_insn(err);
    update_insn(endif_2);
    update_insn(i_2);
    ASSERT_EQ(symbol::lookup("i").value, 1);
    ASSERT_EQ(symbol::lookup("j").value, 1);
    ASSERT_EQ(symbol::lookup("k").value, 3);
    update_insn(endif_1);
    update_insn(j_2);
    ASSERT_EQ(symbol::lookup("i").value, 1);
    ASSERT_EQ(symbol::lookup("j").value, 2);
    ASSERT_EQ(symbol::lookup("k").value, 3);
}

TEST(insn, put)
{
    START();
    OUTPUT(ss, out);
    LEXER(lex,
"/*.byte*/ '1, '2, '3;"
"/*.word*/ '9|'7<<8;"
"/*.ascii*/ <hello>"
"/*.fill*/ 3, '#;"
"/*.fill*/ 2;"
".=3;"
"/*.align*/ 2;"
"/*.align*/ 8;"
"/*.align*/ 4;"
);
    ref<insn> byte = pseudo_byte(lex);
    ref<insn> word = pseudo_word(lex);
    ref<insn> hello = pseudo_ascii(lex);
    ref<insn> fill = pseudo_fill(lex);
    ref<insn> fill2 = pseudo_fill(lex);
    ref<insn> set_dot = assign_insn(lex);
    ref<insn> a2 = pseudo_align(lex);
    ref<insn> a8 = pseudo_align(lex);
    ref<insn> a4 = pseudo_align(lex);
    update_insn(byte);
    ASSERT_EQ(byte->size, 3);
    update_insn(word);
    ASSERT_EQ(word->size, 2);
    update_insn(hello);
    ASSERT_EQ(hello->size, 5);
    update_insn(fill);
    ASSERT_EQ(fill->size, 3);
    update_insn(fill2);
    ASSERT_EQ(fill2->size, 2);
    update_insn(set_dot);
    ASSERT_EQ(symbol::dot->value, 3);
    update_insn(a2);
    symbol::dot->value += a2->size;
    ASSERT_EQ(symbol::dot->value, 4);
    update_insn(a8);
    symbol::dot->value += a8->size;
    ASSERT_EQ(symbol::dot->value, 8);
    update_insn(a4);
    symbol::dot->value += a4->size;
    ASSERT_EQ(symbol::dot->value, 8);
    flush_insn(byte, out);
    flush_insn(word, out);
    flush_insn(hello, out);
    flush_insn(fill, out);
    flush_insn(fill2, out);
    ASSERT_OUT(ss.str(), "12397hello###\0\0");
    flush_insn(a2, out);
    ASSERT_OUT(ss.str(), "12397hello###\0\0\0");
    flush_insn(a8, out);
    ASSERT_OUT(ss.str(), "12397hello###\0\0\0\0\0\0\0");
    flush_insn(a4, out);
    ASSERT_OUT(ss.str(), "12397hello###\0\0\0\0\0\0\0");
}
