all: hw

hw:
	make -C hw/xgsoc/rtl/ulx3s CONF=bvgkm VIDEO=qvga

prog: hw
	make -C hw/xgsoc/rtl/ulx3s prog
	sleep 2

flash: hw
	make -C hw/xgsoc/rtl/ulx3s flash
	sleep 2

sw:
	make -C sw/src/demo

run: hw sw
	make -C sw/src/demo run

clean:
	make -C hw/xgsoc/rtl/ulx3s clean
	make -C sw/src/demo clean

clean:
	make -C hw/xgsoc/rtl/ulx3s clean

.PHONY: all hw sw prog run	