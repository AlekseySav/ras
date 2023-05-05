#include "client.h"

void second_pass(output& out)
{
    /*
     * optimizer pass
     */
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
            state::dot().value += r->size;
        }
        err = safe_run(error(state::if_stack.size() != 1, "missed .endif")) || err;
        fatal(++iter > MAX_ITERATIONS, "too many pass2-iterations");
    }
    if (iter > 10)
    {
        trace("no. iterations: {}", iter);
    }

    /*
     * flush pass
     */
    state::assert_defined = true;
    for (ref<insn>& r : program)
    {
        if (err) break;
        state::filename = r->filename;
        state::line = r->line;
        panic(update_insn(r), "insn was changed after optimizer pass");
        err = safe_run(flush_insn(r, out)) || err;
        state::dot().value += r->size;
    }
}
