all: hw sw

hw:
	make -C hw/xgsoc/rtl/ulx3s CONF=bvgkm VIDEO=qvga

prog: hw
	make -C hw/xgsoc/rtl/ulx3s prog
	sleep 2

flash: hw
	make -C hw/xgsoc/rtl/ulx3s flash
	sleep 2

sw:
	make -C sw/src/demo_hw

run: hw sw
	make -C sw/src/demo_hw run

clean:
	make -C hw/xgsoc/rtl/ulx3s clean
	make -C sw/src/demo_hw clean

.PHONY: all hw sw prog run	