#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char LIBRARY_PATH[] = LIBRARY_DIR "/lib/libuntangle.so";

void showUsage(const char* programName) {
    printf("Usage: %s [program] {program args}\n", programName);
    printf("Finds pthreads deadlocks in a program.\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        showUsage(argv[0]);
        return 1;
    }

    const char* existingPreload = getenv("LD_PRELOAD");
    char* preloadArg;
    if (existingPreload && *existingPreload) {
        const auto existingPreloadLength = strlen(existingPreload);
        preloadArg = malloc(existingPreloadLength + sizeof(LIBRARY_PATH) + 2);
        sprintf(preloadArg, "%s:%s", existingPreload, LIBRARY_PATH);
    } else {
        preloadArg = (char*)LIBRARY_PATH;
    }
    if (0 != setenv("LD_PRELOAD", preloadArg, 1)) {
        perror("setenv");
        return -1;
    }
    execvp(argv[1], argv + 1);
    perror(argv[0]);
    return -1;
}
