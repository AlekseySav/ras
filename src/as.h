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

    // static inline size_t modrm_size(Expr& e, uint8_t disp_size)
    // {
    //     auto tmp = e.eval();
    //     E("bad mod r/m byte", tmp.has_value() == false);

    //     if (disp_size == 0 && tmp.value().type == A_mm && tmp.value().modrm == 6)
    //     {
    //         disp_size = 1;
    //     }
    //     switch (tmp.value().type)
    //     {
    //         case A_mm: return 1 + disp_size;
    //         case A_m0: return 3;
    //         case A_rb:
    //         case A_rw: return 1;
    //     }
    //     E("bad mod r/m byte");
    //     return 0;
    // }

    // static inline void put_modrm(Output& out, Expr& e, uint8_t rr, uint8_t csiz)
    // {
    //     uint8_t modrm = rr << 3;
    //     auto tmp = e.eval();
    //     E("bad mod r/m byte", tmp.has_value() == false);
    //     auto v = tmp.value();
    //     int16_t disp = v.value;
    //     switch (v.type)
    //     {
    //         case A_m0:
    //             modrm |= 6;
    //             csiz = 2;
    //             break;
    //         case A_rb:
    //         case A_rw:
    //             csiz = 0;
    //             modrm |= 0300 | v.value;
    //             break;
    //         case A_mm:
    //             if (csiz == 0 && v.modrm == 6)
    //             {
    //                 csiz = 1;
    //             }
    //             modrm |= (csiz << 6) | v.modrm;
    //             break;
    //     }
    //     out.put_byte(modrm | 6);
    //     for (uint8_t i = 0; i < csiz; i++)
    //     {
    //         out.put_byte(disp & 0xff);
    //         disp >>= 8;
    //     }
    // }
}
