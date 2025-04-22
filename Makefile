obj-m := usb_kbd_driver.o

KERNEL_DIR = /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) modules
	
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order 

