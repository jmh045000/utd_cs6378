
CXX = g++
LD = g++
CFLAGS := -O2 -Iinclude/
LDFLAGS := -Llib/ -Wl,-rpath,'$(PWD)/lib/' 
HIDE := @

OBJS = hw2.o Weight.o Events.o

all: hw2

hw2: $(OBJS)
	@echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -lgmpxx -lgmp $(OBJS) -o $@

$(OBJS): lib/libgmpxx.a

lib/libgmpxx.a: gmp-5.0.5.tar.bz2
	rm -rf gmp-5.0.5
	tar xf $^
	cd gmp-5.0.5 ; ./configure --prefix $(PWD) --enable-cxx ; $(MAKE) ; $(MAKE) install

gmp-5.0.5.tar.bz2:
	wget ftp://ftp.gmplib.org/pub/gmp-5.0.5/gmp-5.0.5.tar.bz2

.PHONY: debug clean dist
debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -DDEBUG" all

clean:
	rm -f hw2 *.o gmp-5.0.5.tar.bz2
	rm -rf .depend lib include share gmp-5.0.5

dist:
	$(MAKE) clean
	mkdir jmh045000
	cp *.cpp Makefile README jmh045000
	zip -r jmh045000.zip jmh045000/
	rm -rf jmh045000/

-include .depend/*.d

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)



