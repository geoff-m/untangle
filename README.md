# untangle
A deadlock-detecting debugging tool for applications that use pthreads.

## Usage
We illustrate by example. Consider the following buggy program:
```c++
// buggy.cpp
#include <thread>
#include <mutex>
#include <pthread.h> // for setname

int main() {
    pthread_setname_np(pthread_self(), "outer");
    std::mutex m;
    std::lock_guard outerLock(m);
    std::thread t([&] {
        pthread_setname_np(pthread_self(), "inner");
        std::lock_guard innerLock(m);
    });
    t.join();
    return 0;
}
```
Compile it normally:
```shell
$> g++ -g -o buggy buggy.cpp
``` 
Running `./buggy` will simply hang indefinitely.
Using untangle will identify the problem.
```shell
$> untangle ./buggy
```
Untangle prints an explanation of the deadlock (one line per involved thread) and raises SIGTRAP:
```text
untangle: Thread "inner" (0x7f8eb3c0c6c0) created a deadlock involving 2 threads by waiting for mutex 0x7ffe9a9c9420, which is held by thread "outer" (0x7f8eb4121780):
untangle:  Thread "outer" (0x7f8eb4121780) is joining thread "inner" (0x7f8eb3c0c6c0).
Trace/breakpoint trap      (core dumped) untangle ./buggy
```

Untangle pairs well with a debugger.
(As usual, compiling your application with debug symbols is recommended,
but untangle does not use them.)
```shell
$> gdb --args untangle ./buggy
```
Since the signal is raised during the offending attempt to lock a mutex or join a thread,
inspecting the stack trace is often enough to pinpoint the problem.
In this example, output from gdb's `bt` command indicates the source line `std::lock_guard innerLock(m);`,
which corresponds as expected to the message we got from untangle.

## How it works
Shared object interposition via LD_PRELOAD.

## Limitations
Untangle detects only deadlocks formed by calls to `pthreads_mutex_lock`, `pthreads_join`,
and mixtures thereof.
Many C++ applications rely on these APIs indirectly via typical standard library implementations,
and many C applications may use them directly.
However, untangle does not detect deadlocks caused in other ways.

## Extras
Normal usage of untangled is like above--simply running `untangled [your-application]`,
likely inside a debugger.
However, if you're able to rebuild the debuggee,
you can take steps to get even more out of untangle.
Header `<untangle/untangle.h>` declares some utility functions that can enhance untangle's output.
After including this header, you'll need to make your linker happy,
such as by including `-luntangle` in your compile command.

### Giving a name to a mutex
Call `void untangle_set_mutex_name(pthread_mutex_t* mutex, const char* name)`
to give a name to a mutex.
This name will show up in untangle messages if the mutex is involved in a deadlock.

### Giving a name to a thread
Untangle doesn't provide a function for this purpose, but I mention this idea anyway as a tip.
Use `pthread_setname_np` (defined in `<pthread.h>`) for this. 

### Controlling output
Normally, untangle writes its messages to stderr.
You can override this by calling
`void untangle_set_writer(void (*writer)(const char*, size_t, void*), void* state)`.
When untangle wants to write something, it will make a call like
`writer(text, textLength, state)` where `writer` and `state` have the values you provided when you called
`untangle_set_writer`.
