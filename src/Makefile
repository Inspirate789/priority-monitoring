ifneq ($(KERNELRELEASE),)
	obj-m := taskInfoGetter.o
	taskInfoGetter-objs := md.o
else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build
	PWD = $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	make clean

logger:
	gcc starterLogger.c -Wall -Werror -o sl.exe

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order *.mod *.symvers
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions

disclean: clean
	@rm *.ko *.symvers *.mod
endif 

# sudo insmod md.ko
# lsmod | grep md
# sudo dmesg
# sudo rmmod md.ko
