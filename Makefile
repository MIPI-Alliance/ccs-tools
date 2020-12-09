.PHONY: clean test all

CFLAGS += -Wall -fstack-protector-all -fno-dse \
	-fno-delete-null-pointer-checks -fwrapv

all: ccs-bin-to-yaml

ccs-regs.h ccs-regs.c ccs-limits.h ccs-limits.c: ccs-regs.asc mk-ccs-regs
	./mk-ccs-regs -r ccs-regs.c -e ccs-regs.h -l ccs-limits.c \
		-L ccs-limits.h

ccs-licenses.c: licenses/* mk-licenses
	./mk-licenses -o ccs-licenses.c

ccs-bin-to-yaml: ccs-data.o ccs-regs.o ccs-licenses.o ccs-regs.h

clean:
	rm -f *.o ccs-bin-to-yaml ccs-regs.h ccs-regs.c ccs-licenses.c
	find -name '*~' | xargs rm -f

test:
	for i in t/[0-9][0-9]*.sh; do echo \#\#\#\# running test $$i; \
		$$i || exit $$?; done
	@date +'All tests successfully completed on %Y-%m-%d'
	@echo -n "git commit id "
	@git rev-list -1 HEAD
