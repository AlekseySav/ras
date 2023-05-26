#include "insn.h"

insn::insn(bool force_run) : line(0), size(0), force_run(force_run), epoch(0)
{}

bool update_insn(ref<insn>& i)
{
    if (i->force_run || state::if_stack.top())
    {
        return i->update();
    }
    return false;
}

void flush_insn(ref<insn>& i, output& out)
{
    if (i->force_run || state::if_stack.top())
    {
        i->flush(out);
    }
}

ref<insn> simple_group(expr_iterator it, byte n, byte d, bool f)
{
    struct _insn : insn
    {
        _insn(byte n) : n(n)
        {
            size = 1;
        }

        void flush(output& out) override
        {
            out.put_byte(n);
        }

        byte n;
    };
    return make_insn<_insn>(n);
}
