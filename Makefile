CFLAGS = -g -Wall -Wstrict-prototypes \
	 -Wmissing-prototypes \
	 -Wmissing-declarations \
	 -Wshadow -Wpointer-arith \
	 -Wcast-qual -Wsign-compare

all:
	@echo "CC	portreader"
	@gcc portreader.c -o portreader $(CFLAGS)

clean:
	@echo "Cleaning"
	@rm -f portreader
