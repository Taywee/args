DESTDIR		?= 	/usr/local

.PHONY: clean pages runtests uninstall install installman

uninstall:
	-rm $(DESTDIR)/include/args.hxx
	-rmdir $(DESTDIR)/include
	-rm $(DESTDIR)/share/man/man3/args_*.3.bz2
	-rmdir -p $(DESTDIR)/share/man/man3

install:
	mkdir -p $(DESTDIR)/include
	cp args.hxx $(DESTDIR)/include

installman: doc/man
	mkdir -p $(DESTDIR)/share/man/man3
	cp doc/man/man3/*.3.bz2 $(DESTDIR)/share/man/man3

clean:
	-rm -rv build doc

pages:
	-rm -r pages/*
	doxygen Doxyfile
	cp -rv doc/html/* pages/

doc/man:
	doxygen Doxyfile
	bzip2 doc/man/man3/*.3

runtests:
	cmake -B build
	cmake --build build --parallel
	cd build && ctest --output-on-failure
