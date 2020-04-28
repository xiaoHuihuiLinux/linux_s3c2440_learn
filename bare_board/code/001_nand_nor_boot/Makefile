objs = start.o led.o uart.o init.o nand_flash.o main.o exception.o interrupt.o timer.o nor_flash.o my_printf.o string_utils.o lib1funcs.o

objs+=LCD/font.o
objs+=LCD/framebuffer.o
objs+=LCD/geometry.o
objs+=LCD/lcd.o
objs+=LCD/lcd_4.3.o
objs+=LCD/lcd_controller.o
objs+=LCD/lcd_test.o
objs+=LCD/s3c2440_lcd_controller.o
objs+=LCD/font_8x16.o
all: $(objs)#ËùÓÐµÄÒÀÀµ
	#arm-linux-ld -Ttext 0 -Tdata 0x30000000  start.o led.o uart.o init.o main.o -o sdram.elf
	arm-linux-ld -T sdram.lds $^ -o sdram.elf
	arm-linux-objcopy -O binary -S sdram.elf sdram.bin
	arm-linux-objdump -D sdram.elf > sdram.dis
clean:
	rm *.bin *.o *.elf *.dis
	
%.o : %.c
	arm-linux-gcc -march=armv4 -c -o $@ $<

%.o : %.S
	arm-linux-gcc -march=armv4 -c -o $@ $<
	