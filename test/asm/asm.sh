# asm test tools

start_group() {
    echo test group $1...
}

run_test() {
    ./ras -D.=0 .bin/test/1.s -o .bin/test/1
    nasm -f bin .bin/test/2.asm
    if ! cmp .bin/test/1 .bin/test/2; then
        echo failed "$@" >&2
        echo expected:
        hexdump .bin/test/2 -C >&2
        echo got:
        hexdump .bin/test/1 -C >&2
        exit 1
    fi
}

echo .bits=16 tests...
bits 16; run_tests
echo .bits=32 bit tests...
bits 32; run_tests
echo asm tests passed
