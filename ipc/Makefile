.PHONY: clean all
COMPILER = g++ -std=c++11 -fsanitize=address,undefined,leak -Werror -Wformat

server: server.cpp
	$(COMPILER) $^ -o $@

client: client.cpp
	$(COMPILER) $^ -o $@

all: client server 

clean:
	rm -f client server test.sock