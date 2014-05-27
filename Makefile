export

all:
	make -C src

debug:
	make -C src debug

full-debug:
	make -C src full-debug

clean:
	make -C src clean

mrproper:
	make -C src mrproper