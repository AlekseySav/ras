#pragma once
#include "expr/expr.h"
#include "io/output.h"
#include <array>

namespace as
{
    static inline byte get_rw(lexer& lex)
    {
        token t = lex.get();
        error(t != L_sym, "bad mod r/m byte");
        typeinfo ti = symbol::lookup(lval<string>(t)).type;
        error(ti.type != A_rw, "bad mod r/m byte");
        return ti.n;
    }

    static inline byte make_rm(lexer& lex)
    {
        byte n = 010 | get_rw(lex);
        if (lex.tryget(','))
        {
            n = n << 3 | get_rw(lex);
        }
        switch (n)
        {
            case 0136: return 0; // bx,si
            case 0137: return 1; // bx,di
            case 0156: return 2; // bp,si
            case 0157: return 3; // bp,di
            case 0016: return 4; // si
            case 0017: return 5; // di
            case 0015: return 6; // bp
            case 0013: return 7; // bx
        }
        error("bad mod r/m byte");
        return 0;
    }

    static inline void init_builtins()
    {
        static std::array<const char*, 8> rb{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
        static std::array<const char*, 8> rw{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
        static std::array<const char*, 8> sr{"es", "cs", "ss", "ds", "fs", "gs"};
        static pool<expr, 22> regs;

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
