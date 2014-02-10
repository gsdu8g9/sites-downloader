
all: sd

sd:
	make -C src $(MFLAGS)

debug:
	make -C src debug $(MFLAGS)

clean:
	make -C src clean