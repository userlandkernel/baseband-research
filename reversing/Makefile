all:
	mkdir -p build
	gcc src/qcomm/bbunitool.c -o build/bbunitool
	gcc src/elf/elfparse_new.c src/disasm/disasm.c -o build/elftool-supreme
	gcc src/elf/elfparse.c -o build/elftool

install:
	make all
	cp -r build/* /usr/local/bin/

clean:
	rm -rf /usr/local/bin/bbunitool
	rm -rf /usr/local/bin/elftool-supreme
	rm -rf /usr/local/bin/elftool
	rm -rf build
