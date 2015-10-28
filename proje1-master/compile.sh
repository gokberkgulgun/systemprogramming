clear
echo "Removing old files..."
rm obj/source.o
rm obj/main.o
rm obj/program.o
clear
echo "Compiling..."
nasm -f elf32 src/source.asm -o obj/source.o
if [ "$?" -ne "0" ]; then
  exit 1
fi
gcc -c src/source.c -o obj/main.o
if [ "$?" -ne "0" ]; then
  exit 1
fi
gcc obj/main.o obj/source.o -o obj/program.o
if [ "$?" -ne "0" ]; then
  exit 1
fi
echo "Running..."
sleep 0.45
clear
cd obj
./program.o
