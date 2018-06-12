.PHONY: all run clean
COMPILER = g++ -std=c++11

handler: handler.cpp
	$(COMPILER) $< -o $@

all: handler

run: all
	./handler

clean:
	rm -f handler