#include "static.h"
#include "dynamic1.h"
#include <stdio.h>
#include <dlfcn.h>
#include <cstdlib>


int main(int argc, char **argv) {
    const char *name = "Boris";

    static_hello(name);
    dynamic1_hello(name);

    void *handle;
    handle = dlopen("libdynamic2.so", RTLD_LAZY | RTLD_LOCAL);
    if (handle == NULL) {
        fprintf(stderr, "Cannot open libdynamic2.so library. Error:\n%s\n", dlerror());
        return EXIT_FAILURE;
    }
    
    void (*dynamic2_hello)(const char *) = (void(*)(const char *))dlsym(handle, "dynamic2_hello");

    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Cannot extract function. Error:\n%s\n", error);
        return EXIT_FAILURE;
    }

    dynamic2_hello(name);

    dlclose(handle);
    return 0;
}