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
        for (section& s : program)
        {
            for (ref<insn>& r : s.code)
            {
                state::filename = r->filename;
                state::line = r->line;
                err = safe_run(flag = update_insn(r) || flag) || err;
                state::dot().value += r->size;
            }
        }
        err = safe_run(error(state::if_stack.size() != 1, "missed .endif")) || err;
        if (++iter > MAX_ITERATIONS)
        {
            // too many iterations, speed up
            state::unoptim = true;
        }
    }
    if (iter > 10)
    {
        trace("no. iterations: {}", iter);
    }

    /*
     * flush pass
     */
    state::assert_defined = true;
    for (section& s : program)
    {
        for (ref<insn>& r : s.code)
        {
            if (err) break;
            state::filename = r->filename;
            state::line = r->line;
            if (r->size > 0)
            {
                add_listing();
            }
            panic(update_insn(r), "insn was changed after optimizer pass");
            err = safe_run(flush_insn(r, out)) || err;
            state::dot().value += r->size;
        }
    }
}
