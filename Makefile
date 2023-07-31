# <obj>-opt
# m: dynamic allocated kernel module
# y: compile and link with kernel
# n: do not compiile the module

#cmd: lsmod: see al modules installed in kernel
#cmd: sudo dmesh: see kernel's buffer interactions

#cat /proc/devices: See all the major numbers functioning in your OS
#ls -l /dev/ All installed dev
#ls -l /sys/class See all installed classes. (Also you can specify with yout $(class_name/) and see how many devices you had)
#udevadm info /dev/&(device_name): Info about our new device
#cat /dev/$(device_name): Tries to open your driver :. Equals to read
#echo "string" > /dev/(device_name) :. Equals to write
#sudo chmod 666 /dev/$(dev_name) give permissions to our device

obj-m := ezm-char-mod.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers