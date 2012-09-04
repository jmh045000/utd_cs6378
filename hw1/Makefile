
CXX = g++
LD = g++
CFLAGS := -O2

all: main

.PHONY: debug
debug:
	$(MAKE) CFLAGS="$(CFLAGS) -DDEBUG" all

main: main.o
	$(LD) $^ -o $@

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)

