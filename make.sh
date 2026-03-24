success=1

for file in src/*.c; do
    basen=$(basename $file)
    clang -c -g $file -o "obj/${basen%.*}.o"

    if [ $? -ne 0 ]; then
        success=0
    fi
done

if [ $success -eq 0 ]; then
    echo "Compilation failed"
    exit
fi

clang -g obj/*.o -lSDL3 -lSDL3_ttf -o build/cpm

if [ $? -eq 0 ]; then
    dsymutil build/cpm
fi

if [ $? -eq 0 ]; then
    ./build/cpm
fi