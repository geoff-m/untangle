#include "platform.h"
#include "write.h"

namespace untangle {
    void library_constructor() {
        writer::constructWrite();
        originalFunctions.initialize();
        originalFunctions.mutex_init(mutexInfosMutex, nullptr);
        originalFunctions.mutex_init(deadlockCheckMutex, nullptr);
    }

    void library_destructor() {
        writer::destroyWrite();
    }
}