# performance test tools

start_group() {
    echo "(intr) ras: ${ras_time}s, nasm: ${nasm_time}s"
}

time_it() {
    TIMEFORMAT="%R"
    { time $@; } 2>&1
}

run_test() {
    ras_atime=$(time_it ./ras -D.=0 .bin/test/1.s -o .bin/test/1)
    nasm_atime=$(time_it nasm -f bin .bin/test/2.asm)
    ras_time=$(tools/sumtime.py $ras_time $ras_atime)
    nasm_time=$(tools/sumtime.py $nasm_time $nasm_atime)
}

ras_time=0
nasm_time=0
bits 16; run_tests
bits 32; run_tests
echo "(total) ras done in: ${ras_time}s, nasm done in: ${nasm_time}s"
