
CXX = g++
LD = g++
CFLAGS := -O2 -Iinclude/
LDFLAGS := 
HIDE := @

OBJS = hw5.o
TARGET = hw5

all: $(TARGET)

hw5: $(OBJS)
	@echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) $(OBJS) -o $@

.PHONY: debug clean dist
debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -DDEBUG" all

clean:
	@echo "Cleaning..."
	@rm -f $(TARGET) $(OBJS)
	@rm -rf .depend 

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

