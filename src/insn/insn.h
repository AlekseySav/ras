#pragma once
#include "../expr/expr_iterator.h"
#include "../io/output.h"
#include <memory>

struct insn
{
    insn(bool false_run = false);

    virtual ~insn() {}
    virtual bool update() { return false; }
    virtual void flush(Output& out) {}

    int line;
    word size;
    bool force_run; // run even if state::active = false
};

template<typename T>
using ref = std::unique_ptr<T>;

typedef ref<insn> insn_factory(expr_iterator it, byte n);

bool update_insn(ref<insn>& i);
void flush_insn(ref<insn>& i, Output& out);

template<typename T, typename... Args>
ref<insn> make_insn(Args&&... args) { return ref<insn>(new T(std::forward<Args&&>(args)...)); }

ref<insn> assign_insn(expr_iterator it, byte n = 0);
ref<insn> label_insn(expr_iterator it, byte n = 0);

ref<insn> pseudo_error(expr_iterator it, byte n = 0);
ref<insn> pseudo_mut(expr_iterator it, byte n = 0);
ref<insn> pseudo_if(expr_iterator it, byte n = 0);
ref<insn> pseudo_else(expr_iterator it, byte n = 0);
ref<insn> pseudo_endif(expr_iterator it, byte n = 0);
ref<insn> pseudo_byte(expr_iterator it, byte n = 0);
ref<insn> pseudo_word(expr_iterator it, byte n = 0);
ref<insn> pseudo_ascii(expr_iterator it, byte n = 0);
ref<insn> pseudo_fill(expr_iterator it, byte n = 0);
ref<insn> pseudo_align(expr_iterator it, byte n = 0);

ref<insn> opcode_addsub(expr_iterator it, byte n);
