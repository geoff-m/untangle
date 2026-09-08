#include "platform.h"
#include "write.h"

using namespace untangle;

__attribute__((constructor))
void ctor() {
    writer::constructWrite();
    originalFunctions.initialize();
    originalFunctions.mutex_init(mutexInfosMutex, nullptr);
    originalFunctions.mutex_init(deadlockCheckMutex, nullptr);
}

__attribute__((destructor))
void dtor() {
    writer::destroyWrite();
}
