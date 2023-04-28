#include "../src/insn/insn.h"


void second_pass(std::vector<ref<insn>>& program, output& out)
{
    symbol& dot = *state::dot;
    bool flag = true, err = false;
    int iter = 0;
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
        fatal(++iter > MAX_ITERATIONS, "too many pass2-iterations");
    }
    if (iter > 10)
    {
        trace("no. iterations: {}", iter);
    }

    state::assert_defined = true;

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
