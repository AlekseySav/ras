#include "insn.h"

insn::insn(bool force_run) : line(0), size(0), force_run(force_run)
{}

bool update_insn(ref<insn>& i)
{
    if (i->force_run || state::if_stack.top())
    {
        return i->update();
    }
    return false;
}

void flush_insn(ref<insn>& i, Output& out)
{
    if (i->force_run || state::if_stack.top())
    {
        i->flush(out);
    }
}
