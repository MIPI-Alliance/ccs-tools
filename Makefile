.PHONY: clean test all

CFLAGS += -Wall -fstack-protector-all -fno-dse \
	-fno-delete-null-pointer-checks -fwrapv

all: ccs-bin-to-yaml

ccs-regs.h ccs-regs.c: ccs-regs.txt mk-regs
	./mk-regs -r ccs-regs.c -e ccs-regs.h

ccs-licenses.c: licenses/* mk-licenses
	./mk-licenses -o ccs-licenses.c

ccs-bin-to-yaml: ccs-data.o ccs-regs.o ccs-licenses.o ccs-regs.h

clean:
	rm -f *.o ccs-bin-to-yaml ccs-regs.h ccs-regs.c ccs-licenses.c
	find -name '*~' | xargs rm -f
