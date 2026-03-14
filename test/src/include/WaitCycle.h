#pragma once
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <thread>

class WaitCycle {
    std::mutex taskStateMutex;
    std::condition_variable taskStateCv;
    std::vector<std::shared_ptr<std::thread>> tasks;
    std::mutex* taskMutexes;
    int tasksReady = 0;

    std::shared_ptr<std::thread> makeWaiter(int index, int waitForIndex);

public:
    explicit WaitCycle(int length);
    ~WaitCycle();

    std::shared_ptr<std::thread> getFirst();
};
