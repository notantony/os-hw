#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstring>
#include <vector>


// function to implement
int f(int x, int y, int z) {
    return x * y + z;
}

const int MAX_LEN = 16;
std::vector<uint8_t> text = {
    0x0f, 0xaf, 0xfe, 0x8d
};

/**
 * Implements function f. If z is passed as an argument, value will patched into the implemention. 
 */
int main(int argc, char const *argv[]) {
    if (argc == 1) {
        text.push_back(0x04);
        text.push_back(0x17);
    } else if (argc == 2) {
        size_t n = strlen(argv[1]);
        if (n > 12) {
            printf("Z_VALUE is too big/small\n");
            return 0;
        }
        int64_t z = 0;
        for (size_t i = 0; i < n; i++) {
            z *= 10;
            z += argv[1][i] - '0';
            if (!(argv[1][i] >= '0' && argv[1][i] <= '9')) {
                printf("Unexpected symbol: \'%c\'\n", argv[1][i]);
                return 0;
            }
        }
        if (z > INT32_MAX || z < INT32_MIN) {
            printf("Z_VALUE is too big/small\n");
            return 0;
        }
        int zz = (int) z;
        text.push_back(0x87);
        text.push_back((uint8_t)(zz & 0x000000ff));
        text.push_back((uint8_t)(zz & 0x0000ff00) >> 8);
        text.push_back((uint8_t)(zz & 0x00ff0000) >> 16);
        text.push_back((uint8_t)(zz & 0xff000000) >> 24);
    } else {
        printf("Usage: jit [<Z_VALUE>]\n");
        return 0;
    }
    text.push_back(0xc3);
    
   
    void *ptr;
    if ((ptr = mmap(NULL, MAX_LEN, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error occured while allocating memory\n");
        return EXIT_FAILURE;
    }

    memcpy(ptr, &text[0], text.size());

    mprotect(ptr, MAX_LEN, PROT_EXEC);

    if (argc == 1) {
        int x, y, z;
        int (*myf)(int, int, int) = (int(*) (int, int, int)) ptr;
        if (scanf("%d%d%d", &x, &y, &z) != 3) {
            printf("Bad input, three integers exprected\n");
            return 0;
        }
        printf("%d\n", myf(x, y, z));
    } else {
        int x, y;
        int (*myf)(int, int) = (int(*) (int, int)) ptr;
        if (scanf("%d%d", &x, &y) != 2) {
            printf("Bad input, two integers exprected\n");
            return 0;
        }
        printf("%d\n", myf(x, y));
    }

    munmap(ptr, MAX_LEN);

    return 0;
}
