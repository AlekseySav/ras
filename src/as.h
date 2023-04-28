#pragma once
#include "expr/expr.h"
#include "io/output.h"
#include <array>

namespace as
{
    static inline uint8_t make_rm(typeinfo ti)
    {
        if (ti.type == A_modrm) return ti.n;
        error(ti.type != A_rw, "bad mod r/m byte");
        switch (ti.n)
        {
            case 3: return 7; // bx
            case 5: return 6; // bp
            case 6: return 4; // si
            case 7: return 5; // di
        }
        error("bad mod r/m byte");
        return 0;
    }

    static inline void init_builtins()
    {
        static std::array<const char*, 8> rb{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
        static std::array<const char*, 8> rw{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
        static std::array<const char*, 8> sr{"es", "cs", "ss", "ds", "fs", "gs"};
        static std::array<const char*, 8> mr{"bx_si", "bx_di", "bp_si", "bp_di"};
        static pool<expr, 26> regs;

        if (regs.size())
        {
            regs.clear();
        }

        auto init = [](byte type, const std::array<const char*, 8>& a)
        {
            byte n = 0;
            for (const char* rr : a)
            {
                if (rr == nullptr) continue;
                expr& e = regs.alloc();
                e = expr(typeinfo(type, n++));
                symbol& s = symbol::lookup(string(rr));
                s.assign(e);
                s.update();
            }
        };

        init(A_rb, rb);
        init(A_rw, rw);
        init(A_sr, sr);
        init(A_modrm, mr);

        symbol::lookup(string(".")).make_mutable();
    }

    static inline byte modrm_size(expr& e)
    {
        word disp = e.eval();
        typeinfo ti = e.type();

        word disp_size = disp == 0 ? 0 : (disp <= 255 && disp >= 0) ? 1 : 2;

        if (disp_size == 0 && ti.type == A_mm && ti.n == 6)
        {
            disp_size = 1;
        }
        switch (ti.type)
        {
            case A_mm: return 1 + disp_size;
            case A_m0: return 3;
            case A_rb:
            case A_rw: return 1;
        }
        error("bad mod r/m byte");
        return 0;
    }

    static inline void put_modrm(output& out, expr& e, byte rr)
    {
        byte modrm = rr << 3;
        word disp = e.eval();
        typeinfo ti = e.type();
        byte csiz = modrm_size(e) - 1;
        switch (ti.type)
        {
            case A_m0: modrm |= 6; break;
            case A_rb:
            case A_rw: modrm |= 0300 | ti.n; break;
            case A_mm: modrm |= (csiz << 6) | ti.n; break;
        }
        out.put_byte(modrm);
        for (byte i = 0; i < csiz; i++)
        {
            out.put_byte(disp & 0xff);
            disp >>= 8;
        }
    }
}
