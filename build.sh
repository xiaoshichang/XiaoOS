mkdir -p bin/

# build booter
nasm src/boot/booter.asm -f bin -o bin/booter.bin -l bin/booter.lst
cp env/booter_origin.img env/booter.img

# buld kernel entry
nasm src/boot/kernel_entry.asm -f elf -o bin/kernel_entry.o

# build kernel
# -m32 let gcc generate kernel.o in 32bit format
# -fno-pie let gcc
gcc -fno-pie -m32 -ffreestanding -c src/kernel/kernel.c -o bin/kernel.o

# link kernel and kernel_entry
# in 64bit environment, ld will default use 64bit mode and produce error.
# -m elf_i386 let ld work under 32bit mode.
ld -m elf_i386 -o bin/kernel.bin -Ttext 0x1000 bin/kernel.o bin/kernel_entry.o --oformat binary

# combine booter.bin and kernel.bin to generate XiaoOS.bin
cat bin/booter.bin bin/kernel.bin > bin/XiaoOS.bin

# write XiaoOS to booter.img
dd if=bin/XiaoOS.bin of=env/booter.img bs=512 count=16 conv=notrunc


