#pragma once
#include "expr/expr.h"
#include "io/output.h"
#include <array>

namespace as
{
    const byte OPSIZE = 0x66, ADSIZE = 0x67;

    static inline byte get_rr(lexer& lex, byte type)
    {
        token t = lex.get();
        error(t != L_sym, "bad mod r/m byte");
        typeinfo ti = symbol::lookup(lval<string>(t)).type;
        error(ti.type != type, "bad mod r/m byte");
        return ti.n;
    }

    static inline byte make_rm16(lexer& lex)
    {
        byte n = 010 | get_rr(lex, A_rw);
        if (lex.tryget(','))
        {
            n = n << 3 | get_rr(lex, A_rw);
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

    static inline std::pair<byte, byte> make_rm32(lexer& lex)
    {
        byte base = get_rr(lex, A_rl);
        if (base != 4 && lex.touch() != ',') return {base, 0};
        // MODRM + SIB
        byte index = 4, scale = 0;
        if (lex.tryget(','))
        {
            index = get_rr(lex, A_rl);
            if (lex.tryget(','))
            {
                token t = lex.get();
                error(t != L_num || lval<word>(t) > 3, "bad sib byte");
                scale = lval<word>(t);
            }
        }
        return {4, base | index << 3 | scale << 6};
    }

    static inline typeinfo make_rm(lexer& lex)
    {
        token t = lex.touch();
        error(t != L_sym, "bad mod r/m byte");
        if (symbol::lookup(lval<string>(t)).type.type == A_rl)
        {
            auto[mr, sib] = make_rm32(lex);
            return typeinfo(A_mw, mr, sib);
        }
        return typeinfo(A_mb, make_rm16(lex));
    }

    static inline void init_builtins()
    {
        static std::array<const char*, 8> rb{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
        static std::array<const char*, 8> rw{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
        static std::array<const char*, 8> rl{"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
        static std::array<const char*, 8> sr{"es", "cs", "ss", "ds", "fs", "gs"};
        static pool<expr, 30> regs;

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
        init(A_rl, rl);
        init(A_sr, sr);

        symbol::lookup(string(".")).make_mutable();
    }

    static inline byte modrm_type(expr& e)
    {
        typeinfo ti = e.type();
        if (ti.type != A_m0) return ti.type;
        word disp = e.eval();
        if (disp >= 0 && disp <= 0xffff) return A_mb;
        return A_mw;
    }

    static inline byte modrm_size(expr& e, bool disponly = false)
    {
        word disp = e.eval();
        typeinfo ti = e.type();

        word dw = disp == 0 ? 0 : (disp <= 255 && disp >= 0) ? 1 : 2;
        word dl = disp == 0 ? 0 : (disp <= 255 && disp >= 0) ? 1 : 4;

        if (dw == 0 && ti.type == A_mb && ti.n == 6) dw = 1;
        if (dl == 0 && ti.type == A_mw && ti.n == 5) dl = 1;

        switch (ti.type)
        {
            case A_mb: return 1 + dw;
            case A_mw: return 1 + dl + (ti.n == 4 ? 1 : 0);
            case A_m0: return (disponly ? 0 : 1) + (disp >= 0 && disp <= 0xffff ? 2 : 4);
            case A_rb:
            case A_rw: return 1;
        }
        error("bad mod r/m byte");
        return 0;
    }

    static inline void put_modrm(output& out, expr& e, byte rr, bool disponly = false)
    {
        byte modrm = rr << 3;
        word disp = e.eval();
        typeinfo ti = e.type();
        byte csiz = modrm_size(e) - 1;
        if (ti.type == A_mw && ti.n == 4)
        {
            csiz--;
        }
        switch (ti.type)
        {
            case A_m0: modrm |= 5 + (modrm_type(e) == A_mb); break;
            case A_rb:
            case A_rw:
            case A_rl: modrm |= 0300 | ti.n; break;
            case A_mb: modrm |= (csiz << 6) | ti.n; break;
            case A_mw: modrm |= ((csiz == 4 ? 2 : csiz) << 6) | ti.n; break;
        }
        if (!disponly)
        {
            out.put_byte(modrm);
            if (ti.type == A_mw && ti.n == 4)
            {
                out.put_byte(ti.x);
            }
        }
        for (byte i = 0; i < csiz; i++)
        {
            out.put_byte(disp & 0xff);
            disp >>= 8;
        }
    }
}
