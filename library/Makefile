.PHONY: all run clean
COMPILER = g++ -std=c++11 -fsanitize=address,undefined,leak -Werror -Wformat

libstatic.a: static.cpp static.h
	$(COMPILER) $< -o $*.o -c
	ar cr $@ $*.o

libdynamic1.so: dynamic1.cpp dynamic1.h
	$(COMPILER) $< -o $@ -shared -fpic

libdynamic2.so: dynamic2.cpp
	$(COMPILER) $< -o $@ -shared -fpic

main: main.cpp libstatic.a libdynamic1.so
	$(COMPILER) $< -o $@ -L. -lstatic -Wl,-rpath,. -ldynamic1 -Wl,-ldl

all: main libdynamic2.so

run: all
	./main

clean:
	rm -f *.o *.a *.so main
	