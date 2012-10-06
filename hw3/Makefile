
CXX = g++
LD = g++
CFLAGS := -O2 -Iinclude/
LDFLAGS := -lpthread
HIDE := @

OBJS = hw3.o Message.o Mutex.o Socket.o

all: dmutex

dmutex: $(OBJS)
	@echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) $(OBJS) -o $@
	cp $@ ~

fakedmutex: clean
	@echo BUILDING WITH A FAKE MUTEX
	$(MAKE) CFLAGS="$(CFLAGS) -DFAKEMUTEX" all

.PHONY: debug clean dist
debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -DDEBUG" all

clean:
	rm -f dmutex *.o
	rm -rf .depend 

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

