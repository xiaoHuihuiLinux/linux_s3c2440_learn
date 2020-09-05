KERN_DIR = /home/book/s3c2440/kernel_2.6.22/linux-2.6.22.6
#KERN_DIR =  /home/book/s3c2440/linux-2.6.22-patch/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= Usbmouce_as_key.o
