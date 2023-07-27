# ras

## build

- build:   `$ build/config ras release && make`
- clear:   `$ build/clear`
- tests:   `$ build/run-tests`
- install: `# build/install`

## dependencies
- gtest
- gtest_main
- fmt

## syntax

- tokens

symbols and numbers are represented identical to C, except in **ras** symbols might contain `.`

strings are defined in `<`, `>` brackets, e.g. `<hello>`

single character is defined with `'` at the beginning and is treated as a number with the value of the appropriate ascii code, e.g. `'!` will be treated the same as `0x21`

double characters are supported and are defined `"`, e.g. `"#!` is `0x2122`

escape characters (may be used in both strings and character tokens):
```
\n  \r  \b  \e  \s  \t  \0  \d  \>  \\
```
where `\d` is ascii DEL (0x7f), `\s` is space, `\>` is `>`, other characters means the same as in C


- **ras** uses **AT&T** asm opcode syntax, i.e
```
opcode src..., dst
```

- both symbolic and numeric labels are supported

- define constant with `name = value`

- [reference](http://www.mathemainzel.info/files/x86asmref.html) to supported instruction set

- all 54 **register** names (listed below) are reserved
```
    es   cs   ss   ds   fs   gs 
    al   cl   dl   bl   ah   ch   dh   bh
    ax   cx   dx   bx   sp   bp   si   di
    eax  ecx  edx  ebx  esp  ebp  esi  edi
    cr0  cr1  cr2  cr3  cr4  cr5  cr6  cr7
    dr0  dr1  dr2  dr3  dr4  dr5  dr6  dr7
    tr0  tr1  tr2  tr3  tr4  tr5  tr6  tr7
```

- special symbols

`.`: after each instruction it is adjusted by size of that instruction (i.e. it has the same meaning `$` in **nasm**, though it can be adjusted explicitly to reserve logical space, or set to a different value)

`.bits`: if set to 16, **ras** it is in 16-bit segment, if set to 32, **ras** assumes it is in 32-bit segment (defaults to 16)

- **ras** uses **AT&T** value syntax, e.g.
```
    ax          register value
    1234        memory value
    $1234       immediate value
```
- **16-bit mod r/m** syntax

```
disp(base)
disp(index)
disp(base,index)
```
where
`disp` is an integer expression (may be omitted),
`base` refers to `bx` or `bp`,
`index` refers to `si` or `di`

- **32-bit mod r/m** syntax

```
disp(base)
disp(base,index)
disp(base,index,scale)
```
where `base` and `index` are any 32-bit registers,
`scale` is a constant in range [0, 3].

- **ras** expression syntax mimics C expressions, except ternary operator is not supported

- pseudo instruction syntax

```
syntax                          notes
.error <msg>                    trigger an error
.mut name [, name]...           make given symbols mutable
.if expr
.else
.endif
.byte expr [, expr]...
.word expr [, expr]...          '.word' may be omitted (*)
.long expr [, expr]...
.ascii <string>                 '.ascii' may be omitted (*)
.fill count, [, val]            same to '.byte val' count times
.align align                    make '.' aligned to align
```

(*) e.g.
```
    123; 5; 6
```
is parsed as `.word 123, 5, 6`
```
    <hello\n\0>
```
is parsed as `.ascii <hello\n\0>`

therefore `.word` and `.ascii` may be omitted

## timings & optimizations

- **ras** is approximately 4 times faster than **nasm**
- **ras** optimizer makes as optimized machine code as possible (unlike **fasm**, **gas**, etc.), unless code is cursed and can't be optimized in reasonable time &mdash; in that case it can produce not optimal code (though assemblers like `nasm` simply crashes in this case).

## custom opcodes

all opcodes are described in `src/opcodes.yaml`, which makes it extremely fast and easy to add new opcodes

then **ras** should be reconfigured and recompiled

## ras client

execute **ras -h** for usage
