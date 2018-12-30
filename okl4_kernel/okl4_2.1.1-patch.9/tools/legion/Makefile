CFLAGS = -O2 -Wall -DDEBUG -arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -gdwarf-2
DESTDIR = $HOME

all: legion

install: all
	install -m 755 legion $(DESTDIR)/bin

test: all
	./legion -k l4kernel -m ig_sigma0.reloc -m iguana.reloc -o test.legion

legion: legion.o mach-o.h
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c mach-o.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f legion *.o

