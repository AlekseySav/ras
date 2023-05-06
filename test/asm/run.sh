#
# required predefines:
#   start_group
#   run_test
#

reg1="al/cl/dl/bl/ah/ch/dh/bh"
reg2="ax/cx/dx/bx/sp/bp/si/di"
reg3="eax/ecx/edx/ebx/esp/ebp/esi/edi"
cr="cr0/cr1/cr2/cr3/cr4/cr5/cr6/cr7"
dr="dr0/dr1/dr2/dr3/dr4/dr5/dr6/dr7"
tr="tr0/tr1/tr2/tr3/tr4/tr5/tr6/tr7"
sreg="es/ss/ds/fs/gs"
cseg="es/cs/ss/ds/fs/gs"
imm1="00/01/02/03/0x7f/32"
imm2="0xfff/0xff1/0xff5/0x9fff/1204/1234"
mem_nasm="0xfff/0x123/0xffff/0x10/bx+si/bx+di/bp+si/bp+di/si/di/bx/bx+0x12/bx+si+1/eax/ecx/edx/ebx/esi/edi/esp/ebp/eax+eax/100000+eax+ecx/13+eax+edx/eax+ebx/edx+esi/esp+ebp/edx+ebp*2/esi+edi*8/1000+eax/dword 0x12345678"
mem_ras="0xfff/0x123/0xffff/0x10/(bx,si)/(bx,di)/(bp,si)/(bp,di)/(si)/(di)/(bx)/0x12(bx)/1(bx,si)/(eax)/(ecx)/(edx)/(ebx)/(esi)/(edi)/(esp)/(ebp)/(eax,eax)/100000(eax,ecx)/13(eax,edx)/(eax,ebx)/(edx,esi,0)/(esp,ebp)/(edx,ebp,1)/(esi,edi,3)/1000(eax)/0x12345678"
db_nasm="\$-125/\$/\$+129"
db_ras=".-125/./.+129"
dw_nasm="\$+0xfff/\$+0xff1/\$+0xff5/\$+0x9fff/\$+1204/\$+1234"
dw_ras=".+0xfff/.+0xff1/.+0xff5/.+0x9fff/.+1204/.+1234"

group_simple="aam/aad/daa/das/aaa/aas/pusha/popa/nop/cbw/cwd/pushf/popf/sahf/lahf/leave/int3/into/iret/xlat/repne/repe/rep/hlt/cmc/clc/stc/cli/sti/cld/std/cwde/cdq/clts/invd/wbinvd"
group_simsiz="pusha/popa/pushf/popf/iret"
group_string="ins/outs/movs/cmps/stos/lods/scas"
group_setflg="seto/setno/setb/setc/setnae/setae/setnb/setnc/sete/setz/setne/setnz/setbe/setna/sets/setns/setp/setpe/setnp/setpo/setl/setnge/setge/setnl/setle/setng/setg/setnle"
group_sjumps="loopne/loope/loop/jcxz"
group_cjumps="jo/jno/jb/jnae/jc/jae/jnb/jnc/je/jz/jne/jnz/jbe/jna/ja/jnbe/js/jns/jp/jpe/jnp/jpo/jl/jge/jnl/jle/jng/jg/jnle"
group_addsub="add/or/adc/sbb/and/sub/xor/cmp"
group_shlshr="rol/ror/rcl/rcr/sal/shl/shr/sar"
group_divmul="not/neg/mul/div/idiv"
group_incdec="inc/dec"
group_loadmr="lldt/ltr/lmsw/smsw/str/verr/verw"
group_loadmm="lidt/lgdt/sgdt/sidt/sgdt"
group_movxsz="movsx/movzx"
group_memory="bound/lea/les/lds/lar/lsl/lss/lfs/lgs/bsf/bsr"
group_extreg="bswap"
group_bitset="bt/bts/btr/btc"
group_return="ret/retf"
group_in="in"
group_out="out"
group_imul="imul"
group_test="test"
group_mov="mov"
group_xchg="xchg"
group_push="push"
group_pop="pop"
group_cmpxchg="cmpxchg"

s1() { echo "$r_predefines" >.bin/test/1.s; tools/com.py "$@" >>.bin/test/1.s; }
s2() { echo "$n_predefines" >.bin/test/2.asm; tools/com.py "$@" >>.bin/test/2.asm; }

ras_wrap() {
    case $1 in
        im) echo "\$$2";;
        ib) echo "\$$2";;
        iw) echo "\$$2";;
        *)  echo "$2";;
    esac
}

nas_wrap() {
    if [[ $1 == iw ]]; then
        if [[ $size == w ]]; then
            echo "word $2"
        elif [[ $size == l ]]; then
            echo "dword $2"
        else
            echo "$2"
        fi
        return
    fi
    if [[ $1 != mm ]]; then
        echo "$2"
    elif [[ $size == b ]]; then
        echo "byte [$2]"
    elif [[ $size == w ]]; then
        echo "word [$2]"
    elif [[ $size == l ]]; then
        echo "dword [$2]"
    else
        echo "[$2]"
    fi
}

bits() {
    r_predefines=".bits=$1"
    n_predefines="[bits $1]"
}

ras_arg() {
    case $1 in
        rb)     echo "$reg1";;
        rw)     echo "$reg2";;
        rl)     echo "$reg3";;
        cr)     echo "$cr";;
        dr)     echo "$dr";;
        tr)     echo "$tr";;
        mm)     echo "$mem_ras";;
        ib)     echo "$imm1";;
        iw)     echo "$imm2";;
        db)     echo "$db_ras";;
        dw)     echo "$dw_ras";;
        sr)     echo "$sreg";;
        *)      echo "$1";;
    esac
}

nas_arg() {
    case $1 in
        rb)     echo "$reg1";;
        rw)     echo "$reg2";;
        rl)     echo "$reg3";;
        cr)     echo "$cr";;
        dr)     echo "$dr";;
        tr)     echo "$tr";;
        mm)     echo "$mem_nasm";;
        ib)     echo "$imm1";;
        iw)     echo "$imm2";;
        db)     echo "$db_nasm";;
        dw)     echo "$dw_nasm";;
        sr)     echo "$sreg";;
        *)      echo "$1";;
    esac
}

test_suite() {
    size=$1
    shift 1
    s=$size
    if [[ $s == n ]]; then
        s=""
    fi
    pa=$p
    if [[ $pa == l ]]; then
        pa=d
    fi
    v=group_$group
    case $# in
        "1")    s1 "${!v}" "$(ras_arg $1)" "{1}$p$s $(ras_wrap $1 {2});.long ."
                s2 "${!v}" "$(nas_arg $1)" "{1}$pa $(nas_wrap $1 {2}){nl}dd $";;
        "2")    s1 "${!v}" "$(ras_arg $1)" "$(ras_arg $2)" "{1}$p$s $(ras_wrap $1 {2}), $(ras_wrap $2 {3});.long ."
                s2 "${!v}" "$(nas_arg $1)" "$(nas_arg $2)" "{1}$pa $(nas_wrap $2 {3}), $(nas_wrap $1 {2}){nl}dd $";;
        "3")    s1 "${!v}" "$(ras_arg $1)" "$(ras_arg $2)" "$(ras_arg $3)" "{1}$p$s $(ras_wrap $1 {2}), $(ras_wrap $2 {3}), $(ras_wrap $3 {4});.long ."
                s2 "${!v}" "$(nas_arg $1)" "$(nas_arg $2)" "$(nas_arg $3)" "{1}$pa $(nas_wrap $3 {4}), $(nas_wrap $2 {3}), $(nas_wrap $1 {2}){nl}dd $";;
        *)      s1 "${!v}" "{1}$p;.long ."
                s2 "${!v}" "{1}$pa{nl}dd $";;
    esac
    run_test "$group $@"
    p=""
}

test_group() {
    start_group $1
    group=$1
}

run_tests() {
    test_group simple
    test_suite

    test_group simsiz
    test_suite
    p=l; test_suite

    test_group string
    p=b; test_suite
    p=w; test_suite
    p=l; test_suite

    test_group setflg
    test_suite n rb
    test_suite n mm

    test_group sjumps
    test_suite n db

    test_group cjumps
    test_suite n db
    test_suite n dw

    test_group addsub
    test_suite b rb mm
    test_suite b mm rb
    test_suite w rw rw
    test_suite w rw mm
    test_suite w mm rw
    test_suite l rl rl
    test_suite l rl mm
    test_suite l mm rl
    test_suite b ib rb
    test_suite b ib mm
    test_suite w ib rw
    test_suite w ib mm
    test_suite l ib rl
    test_suite l ib mm
    test_suite w iw rw
    test_suite w iw mm
    test_suite l iw rl
    test_suite l iw mm

    test_group shlshr
    test_suite b cl rb
    test_suite b cl mm
    test_suite w cl rw
    test_suite w cl mm
    test_suite l cl rl
    test_suite l cl mm
    test_suite b ib rb
    test_suite b ib mm
    test_suite w ib rw
    test_suite w ib mm
    test_suite l ib rl
    test_suite l ib mm

    test_group divmul
    test_suite b rb
    test_suite b mm
    test_suite w rw
    test_suite w mm
    test_suite l rl
    test_suite l mm

    test_group incdec
    test_suite b rb
    test_suite b mm
    test_suite w rw
    test_suite w mm
    test_suite l rl
    test_suite l mm

    test_group loadmr
    test_suite n mm
    test_suite n rw

    test_group loadmm
    test_suite n mm

    test_group movxsz
    test_suite b rb rw
    test_suite b mm rw
    test_suite b rb rl
    test_suite b mm rl
    test_suite w rw rl
    test_suite w mm rl

    test_group memory
    test_suite n mm rw
    test_suite n mm rl

    test_group extreg
    test_suite n rl

    test_group bitset
    test_suite w rw rw
    test_suite w rw mm
    test_suite l rl rl
    test_suite l rl mm
    test_suite w ib rw
    test_suite w ib mm
    test_suite l ib rl
    test_suite l ib mm

    test_group return
    test_suite
    test_suite n ib
    test_suite n iw

    test_group in
    test_suite b dx al
    test_suite w dx ax
    test_suite b ib al
    test_suite w ib ax

    test_group out
    test_suite b al dx
    test_suite w ax dx
    test_suite b al ib
    test_suite w ax ib

    test_group imul
    test_suite b rb
    test_suite b mm
    test_suite w rw
    test_suite w mm
    test_suite l rl
    test_suite l mm
    test_suite w rw rw
    test_suite l rl rl
    test_suite w ib mm rw
    test_suite w iw mm rw
    test_suite w ib rw
    test_suite w iw rw
    test_suite l ib mm rl
    test_suite l iw mm rl
    test_suite l ib rl
    test_suite l iw rl

    test_group test
    test_suite b rb rb
    test_suite b rb mm
    test_suite b mm rb
    test_suite w rw rw
    test_suite w rw mm
    test_suite w mm rw
    test_suite l rl rl
    test_suite l rl mm
    test_suite l mm rl
    test_suite b ib rb
    test_suite b ib mm
    test_suite w ib rw
    test_suite w ib mm
    test_suite w iw rw
    test_suite w iw mm
    test_suite l ib rl
    test_suite l ib mm
    test_suite l iw rl
    test_suite l iw mm

    test_group mov
    test_suite b rb rb
    test_suite b rb mm
    test_suite b mm rb
    test_suite w rw rw
    test_suite w rw mm
    test_suite w mm rw
    test_suite l rl rl
    test_suite l rl mm
    test_suite l mm rl
    test_suite b ib rb
    test_suite b ib mm
    test_suite w ib rw
    test_suite w ib mm
    test_suite w iw rw
    test_suite w iw mm
    test_suite l ib rl
    test_suite l ib mm
    test_suite l iw rl
    test_suite l iw mm
    test_suite w rw sr
    test_suite w mm sr
    test_suite w sr rw
    test_suite w sr mm
    test_suite n cr rl
    test_suite n rl cr
    test_suite n dr rl
    test_suite n rl dr
    test_suite n tr rl
    test_suite n rl tr

    test_group xchg
    test_suite b rb mm
    test_suite b mm rb
    test_suite w rw rw
    test_suite w rw mm
    test_suite l rl rl
    test_suite l rl mm

    test_group push
    test_suite w rw
    test_suite w mm
    test_suite l rl
    test_suite l mm
    test_suite w sr
    test_suite w ib
    test_suite w iw

    test_group pop
    test_suite w rw
    test_suite w mm
    test_suite l rl
    test_suite l mm
    test_suite w sr

    test_group cmpxchg
    test_suite b rb mm
    test_suite w rw mm
    test_suite l rl mm

    test_group seg
    s1 $cseg "seg {1}"
    s2 $cseg "{1}"
    run_test seg
}
