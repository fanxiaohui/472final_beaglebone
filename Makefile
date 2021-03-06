#Makefile
#EE472 Group F Lab 3

export ARCH=arm
#export CROSS_COMPILE=arm-linux-gnueabihf-
export CROSS_COMPILE=arm-linux-gnueabihf-
ccflags-y := -std=gnu99

obj-m := gpioKern.o srmod.o lcdmod.o

srmod-objs := shiftReg.o mainMod_sr.o
lcdmod-objs := lcd_sr.o mainMod_lcd.o
gpioKern-objs := interrupt.o
export-objs := shiftReg.o mainMod_sr.o

# Kernel source directory
#KDIR =/opt/PHYTEC_BSPs/yocto_ti/build/tmp-glibc/work/phyboard_wega_am335x_1-phytec-linux-gnueabi/linux-ti/3.12.30-phy2-r0/git
KDIR =/home/station02/linuxKernBeagle/
#CROSS = /home/alex/Desktop/beagle/kernelbuild/setup-scripts/build/tmp-angstrom_v2014_12-glibc/sysroots/x86_64-linux/usr/bin/arm-angstrom-linux-gnueabi/
PWD = $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

doall: default mainprog adcprog moprog i2cprog uploadrun

mainprog: main.c
	arm-linux-gnueabihf-gcc main.c -std=gnu99 -g -o control -lrt -pthread

adcprog: adc.c adc.h
	arm-linux-gnueabihf-gcc adc.c -std=gnu99 -g -o adc -lrt

moprog: motor.c
	arm-linux-gnueabihf-gcc motor.c -std=gnu99 -g -o motor -lrt

i2cprog: i2c.c
	arm-linux-gnueabihf-gcc i2c.c -std=gnu99 -g -o i2c -lrt	

#butprog: interrupt_test.c
#	arm-linux-gnueabihf-gcc interrupt_test.c -std=gnu99 -g -o interrupt_test -lrt

uploadrun:
	scp adc root@192.168.7.2:~
	scp motor root@192.168.7.2:~
	scp control root@192.168.7.2:~
	scp i2c root@192.168.7.2:~
	#sshpass -p 'temppwd' scp interrupt_test root@192.168.7.2:~
	scp *.ko root@192.168.7.2:~
	scp reloadModule.sh root@192.168.7.2:~
	ssh root@192.168.7.2 './reloadModule.sh'
	#ssh root@192.168.7.2 './control'
