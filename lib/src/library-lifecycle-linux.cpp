#include "platform.h"

__attribute__((constructor))
void ctor() {
    untangle::library_constructor();
}

__attribute__((destructor))
void dtor() {
    untangle::library_destructor();
}