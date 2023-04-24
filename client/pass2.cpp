#include "../src/insn/insn.h"


void second_pass(std::vector<ref<insn>>& program, output& out)
{
    symbol& dot = *symbol::dot;
    bool flag = true, err = false;
    while (flag && !err)
    {
        flag = false;
        for (ref<insn>& r : program)
        {
            state::filename = r->filename;
            state::line = r->line;
            err = safe_run(flag = update_insn(r) || flag) || err;
            dot.value += r->size;
        }
        // trace(". = {}", dot.value);
        err = safe_run(error(state::if_stack.size() != 1, "missed .endif")) || err;
    }

    for (ref<insn>& r : program)
    {
        if (err) break;
        state::filename = r->filename;
        state::line = r->line;
        panic(update_insn(r), "insn was changed after optimizer pass");
        err = safe_run(flush_insn(r, out)) || err;
        dot.value += r->size;
    }
}
