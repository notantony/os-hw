#include <stdio.h>

extern "C"
void dynamic2_hello(const char *name) {
    printf("Hello, %s. I'm dynamic2\n", name);
}