
CXX = g++
LD = g++
CFLAGS := -O2 -Iinclude/
LDFLAGS :=  
HIDE := @

OBJS = hw3.o Socket.o

all: hw3

hw3: $(OBJS)
	@echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) $(OBJS) -o $@

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
	cp *.h *.cpp Makefile README jmh045000
	zip -r jmh045000.zip jmh045000/
	rm -rf jmh045000/

-include .depend/*.d

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)

