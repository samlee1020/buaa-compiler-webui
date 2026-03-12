if [ "$1" = "-c" ]; then
    ./build/Compiler
fi

llvm-link llvm_ir.txt lib.ll -S -o out.ll
lli out.ll