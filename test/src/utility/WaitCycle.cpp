#include "WaitCycle.h"
#include "untangle/untangle.h"
#include <pthread.h>
#include <stdexcept>

std::shared_ptr<std::thread> WaitCycle::makeWaiter(int index, int waitForIndex) {
    return std::make_shared<std::thread>([&, index, waitForIndex] {
        char name[16] = {};
        snprintf(name, sizeof(name), "cycle_%d", index);
        pthread_setname_np(pthread_self(), name);
        std::lock_guard myLock(taskMutexes[index]);
        std::unique_lock lock(taskStateMutex);
        if (++tasksReady == tasks.size())
            taskStateCv.notify_all();
        else
            taskStateCv.wait(lock, [&] { return tasksReady == tasks.size(); });
        lock.unlock();
        // Just for variety,
        // we'll wait by joining if the index is even,
        // and we'll wait by locking if the index is odd.
        if (index % 2 == 0)
            tasks[waitForIndex]->join();
        else
            std::lock_guard otherLock(taskMutexes[waitForIndex]);
    });
}

WaitCycle::WaitCycle(int length) {
    if (length <= 0)
        throw std::invalid_argument("Length must be positive");

    taskMutexes = new std::mutex[length];
    tasks.resize(length);
    for (int i = 0; i < length; i++) {
        new(&taskMutexes[i]) std::mutex();
        untangle_set_mutex_name(taskMutexes[i].native_handle(),
                                ("m" + std::to_string(i)).c_str());
        tasks[i] = makeWaiter(i, (i + 1) % length);
    }
}

WaitCycle::~WaitCycle() {
    delete[] taskMutexes;
}

std::shared_ptr<std::thread> WaitCycle::getFirst() {
    return tasks[0];
}