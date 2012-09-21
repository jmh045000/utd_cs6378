
CXX = g++
LD = g++
CFLAGS := -O2 -Iinclude/
LDFLAGS := -Llib/

all: hw2 gmp

gmp: lib/libgmp.a

lib/libgmp.a: gmp-5.0.5.tar.bz2
	rm -rf gmp-5.0.5
	tar xf $^
	cd gmp-5.0.5 ; ./configure --prefix $(PWD) --enable-cxx ; $(MAKE) ; $(MAKE) install

hw2: hw2.o
	$(LD) $(LDFLAGS) -lgmpxx -lgmp $^ -o $@

.PHONY: debug clean dist
debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -DDEBUG" all

clean:
	rm -f main *.o
	rm -rf .depend

dist:
	$(MAKE) clean
	mkdir jmh045000
	cp *.cpp Makefile README jmh045000
	zip -r jmh045000.zip jmh045000/
	rm -rf jmh045000/

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)



