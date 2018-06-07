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

const int REG = 0, MEM = 1, FAIL = -1;
const signed long REGION_SIZE = 5; // shouldn't be too big


void my_handler(int signum, siginfo_t *siginfo, void *context) {
    static int status = REG;

    static siginfo_t base = *siginfo;
    static unsigned long address = (unsigned long)siginfo->si_addr;

    static std::vector<std::pair<unsigned long, uint8_t> > dump;
    static signed long i = -REGION_SIZE;
    static uint8_t *pos = (uint8_t *)address;
    
    switch (status) {
        case REG: {
            fprintf(stderr, "SIGSEGV received, address: 0x%.16lx\n", address);
            fprintf(stderr, "Registers:\n");
            struct ucontext *uct = (ucontext_t *) context;
            fprintf(stderr, "RAX: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RAX]);
            fprintf(stderr, "RCX: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RCX]);
            fprintf(stderr, "RDX: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RDX]);
            fprintf(stderr, "RBX: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RBX]);
            fprintf(stderr, "RSI: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RSI]);
            fprintf(stderr, "RDI: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RDI]);
            fprintf(stderr, "RSP: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RSP]);
            fprintf(stderr, "RBP: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_RBP]);
            fprintf(stderr, "R8:  0x%.16llx\n", uct->uc_mcontext.gregs[REG_R8]);
            fprintf(stderr, "R9:  0x%.16llx\n", uct->uc_mcontext.gregs[REG_R9]);
            fprintf(stderr, "R10: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_R10]);
            fprintf(stderr, "R11: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_R11]);
            fprintf(stderr, "R12: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_R12]);
            fprintf(stderr, "R13: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_R13]);
            fprintf(stderr, "R14: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_R14]);
            fprintf(stderr, "R15: 0x%.16llx\n", uct->uc_mcontext.gregs[REG_R15]);
            status = MEM;
        }
        case MEM: {
            status = FAIL;
            while (i <= REGION_SIZE) {
                dump.push_back(std::make_pair((unsigned long)(pos + i), *(pos + i)));
                i++;
            }
            if (dump.size() == 0) {
                fprintf(stderr, "Memory dump is unavailable\n");
            } else {
                fprintf(stderr, "Memory dump:\n");
                for (i = 0; i < dump.size(); i++) {
                    fprintf(stderr, "0x%.16lx: %.2x\n", dump[i].first, dump[i].second);
                }
            }
        }
        break;
        case FAIL: {
            i++;
            status = MEM;
            my_handler(signum, siginfo, context);
        }
    }
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