# performance test tools

start_group() {
    return
}

time_it() {
    TIMEFORMAT="%R"
    { time $@; } 2>&1
}

run_test() {
    cat .bin/test/1.s >>.bin/test/perf_1.s
    cat .bin/test/2.asm >>.bin/test/perf_2.asm
}

sizeof() {
    stat $1 | grep Size | awk '{print $2}'
}

echo running performance test...
echo >.bin/test/perf_1.s
echo >.bin/test/perf_2.asm

bits 16; run_tests
bits 32; run_tests

ras_time=$(time_it ./ras -D.=0 .bin/test/perf_1.s -o .bin/test/1)
nasm_time=$(time_it nasm .bin/test/perf_2.asm -o .bin/test/2)

cmp .bin/test/1 .bin/test/2
echo "input size: $(wc -l .bin/test/perf_1.s | awk '{print $1}') lines"
echo "output binary size: $(sizeof .bin/test/1) bytes"
echo "ras done in: ${ras_time}s, nasm done in: ${nasm_time}s"
