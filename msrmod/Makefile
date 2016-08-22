LIBMSR_LIB=$(HOME)/build/libmsr/lib
LIBMSR_INC=$(HOME)/git_root/libmsr_llnl/include

msrmod: msrmod.c
	gcc $< -g -o $@ -L$(LIBMSR_LIB) -lmsr -I$(LIBMSR_INC)

clean:
	rm -f msrmod
