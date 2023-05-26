#pragma once
#include "../expr/expr_iterator.h"
#include "../io/output.h"
#include <memory>

struct insn
{
    insn(bool force_run = false);

    virtual ~insn() {}
    virtual bool update() { return epoch++ == 0; }
    virtual void flush(output& out) {}

    int line;
    const char* filename;
    word size;
    bool force_run; // run even if state::active = false
    int epoch; // for default update
};

template<typename T>
using ref = std::unique_ptr<T>;

typedef ref<insn> (*insn_factory)(expr_iterator it, byte n, byte d, bool f);

bool update_insn(ref<insn>& i);
void flush_insn(ref<insn>& i, output& out);

template<typename T, typename... Args>
ref<insn> make_insn(Args&&... args) { return ref<insn>(new T(std::forward<Args&&>(args)...)); }

ref<insn> assign_insn(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> label_insn(expr_iterator it, byte n = 0, byte d = 0, bool f = true);

ref<insn> pseudo_error(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_mut(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_if(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_else(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_endif(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_byte(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_word(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_long(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_ascii(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_fill(expr_iterator it, byte n = 0, byte d = 0, bool f = true);
ref<insn> pseudo_align(expr_iterator it, byte n = 0, byte d = 0, bool f = true);

ref<insn> simple_group(expr_iterator it, byte n = 0, byte d = 0, bool f = true);

optional<ref<insn>> opcode(const char* name, expr_iterator it); /* defined by opcodes.yaml */
