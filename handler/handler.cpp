#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <errno.h>
#include <stdint.h>
#include <vector>
#include <sys/mman.h>
#include <sys/types.h>
#include <cstring>

const int REG = 0, MEM = 1, FAIL = -1;
const signed long REGION_SIZE = 5; // shouldn't be too big
const signed long BUFFER_SIZE = 4096;

char to_hex(unsigned int t) {
    return t + ((t >= 10) ? 'a' - 10 : '0');
}

void my_write(const char *c) {
    ssize_t len = 0, msg_len = strlen(c);
    while (len < msg_len) {
        ssize_t clen;
        if ((clen = write(STDERR_FILENO, c + len, msg_len - len)) == -1) {
            return;
        }
        len += clen;
    }
}

void write_ul(unsigned long x) {
    char a[2 * sizeof(unsigned long) + 3];
    a[0] = '0';
    a[1] = 'x';
    for (size_t i = 0; i < sizeof(unsigned long); i++) {
        unsigned long t = ((x & ((unsigned long)255 << (i * 8))) >> (i * 8));
        a[2 * sizeof(unsigned long) - 2 * i + 1] = to_hex(t & 15);
        a[2 * sizeof(unsigned long) - 2 * i] = to_hex((t & 240) >> 4);
    }
    a[2 * sizeof(unsigned long) + 2] = 0;
    my_write(a);
} 

void write_ull(unsigned long long x) {
    char a[2 * sizeof(unsigned long long) + 3];
    a[0] = '0';
    a[1] = 'x';
    for (size_t i = 0; i < sizeof(unsigned long long); i++) {
        unsigned long long t = ((x & ((unsigned long long)255 << (i * 8))) >> (i * 8));
        a[2 * sizeof(unsigned long long) - 2 * i + 1] = to_hex(t & 15);
        a[2 * sizeof(unsigned long long) - 2 * i] = to_hex((t & 240) >> 4);
    }
    a[2 * sizeof(unsigned long long) + 2] = 0;
    my_write(a);
} 

void write_byte(char t) {
    char a[3];
    a[0] = to_hex((t & 240) >> 4);
    a[1] = to_hex(t & 15);
    a[2] = 0;
    my_write(a);
}

void my_handler(int signum, siginfo_t *siginfo, void *context) {
    static int status = REG;
    static char buffer[BUFFER_SIZE];

    static siginfo_t base = *siginfo;
    static unsigned long address = (unsigned long)siginfo->si_addr;

    static signed long i = -REGION_SIZE;
    static uint8_t *pos = (uint8_t *)address;
    static int len = 0;

    switch (status) {
        case REG: {
            struct ucontext *uct = (ucontext_t *) context;
            my_write("SIGSEGV received, address: ");
            write_ul(address);
            my_write("\nRegisters:\n");
            my_write("RAX: "); write_ull(uct->uc_mcontext.gregs[REG_RAX]); my_write("\n");
            my_write("RCX: "); write_ull(uct->uc_mcontext.gregs[REG_RCX]); my_write("\n");
            my_write("RDX: "); write_ull(uct->uc_mcontext.gregs[REG_RDX]); my_write("\n");
            my_write("RBX: "); write_ull(uct->uc_mcontext.gregs[REG_RBX]); my_write("\n");
            my_write("RSI: "); write_ull(uct->uc_mcontext.gregs[REG_RSI]); my_write("\n");
            my_write("RDI: "); write_ull(uct->uc_mcontext.gregs[REG_RDI]); my_write("\n");
            my_write("RSP: "); write_ull(uct->uc_mcontext.gregs[REG_RSP]); my_write("\n");
            my_write("RBP: "); write_ull(uct->uc_mcontext.gregs[REG_RBP]); my_write("\n");
            my_write("R8:  "); write_ull(uct->uc_mcontext.gregs[REG_R8]);  my_write("\n");
            my_write("R9:  "); write_ull(uct->uc_mcontext.gregs[REG_R9]);  my_write("\n");
            my_write("R10: "); write_ull(uct->uc_mcontext.gregs[REG_R10]); my_write("\n");
            my_write("R11: "); write_ull(uct->uc_mcontext.gregs[REG_R11]); my_write("\n");
            my_write("R12: "); write_ull(uct->uc_mcontext.gregs[REG_R12]); my_write("\n");
            my_write("R13: "); write_ull(uct->uc_mcontext.gregs[REG_R13]); my_write("\n");
            my_write("R14: "); write_ull(uct->uc_mcontext.gregs[REG_R14]); my_write("\n");
            my_write("R15: "); write_ull(uct->uc_mcontext.gregs[REG_R15]); my_write("\n");
            status = MEM;
            my_write("Memory dump:\n");
        }
        case MEM: {
            status = FAIL;
            while (i <= REGION_SIZE) {
                write_ul((unsigned long)(pos + i));
                my_write(": ");
                write_byte(*(pos + i));
                my_write("\n");
                i++;
            }
        }
        break;
        case FAIL: {
            my_write("??\n");
            i++;
            status = MEM;
            my_handler(signum, siginfo, context);
        }
    }
    write(STDERR_FILENO, buffer, len);
    exit(EXIT_FAILURE);
}


int main() {
    struct sigaction sigact;
    sigact.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigemptyset(&sigact.sa_mask) == -1) {
        fprintf(stderr, "Error while initializing sigaction, error code: %d\n", errno);
        return EXIT_FAILURE;
    }
    sigact.sa_sigaction = my_handler;
    if (sigaction(SIGSEGV, &sigact, NULL) == -1) {
        fprintf(stderr, "Error while setting siggaction, error code: %d\n", errno);
        return EXIT_FAILURE;
    }
    
    int test_size = 16;
    uint8_t *ptr;
    if ((ptr = (uint8_t *)mmap(NULL, test_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        fprintf(stderr, "Cannot test SIGSEGV\n");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < test_size; i++) {
        ptr[i] = (uint8_t) i;
    }
    ptr[-2] = 63;

    return 0;
}