
CXX=g++

all: main

main: main.o
	$(CXX) $^ -o $@

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)

