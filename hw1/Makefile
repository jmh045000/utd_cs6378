
CXX = g++
LD = g++
CFLAGS := -O2

all: hw1

hw1: hw1.o
	$(LD) $^ -o $@

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



