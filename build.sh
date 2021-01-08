mkdir -p bin/

nasm src/booter.asm -f bin -o bin/booter.bin -l bin/booter.lst

cp env/booter_origin.img env/booter.img

dd if=bin/booter.bin of=env/booter.img bs=512 count=1 conv=notrunc
