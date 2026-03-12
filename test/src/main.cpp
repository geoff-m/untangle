#include <condition_variable>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

class JoinCycle {
    std::mutex mutex;
    std::condition_variable taskStarted;
    std::vector<std::shared_ptr<std::thread>> tasks;
    bool allTasksCreated = false;

    std::shared_ptr<std::thread> makeWaiter(int index, int waitForIndex) {
        return std::make_shared<std::thread>([&, index, waitForIndex] {
            char name[16] = {};
            snprintf(name, sizeof(name), "cycle_%d", index);
            pthread_setname_np(pthread_self(), name);
            std::unique_lock lock(mutex);
            taskStarted.wait(lock, [&] { return allTasksCreated; });
            lock.unlock();
            const auto toAwait = tasks[waitForIndex];
            toAwait->join();
        });
    }

public:
    explicit JoinCycle(int length) {
        if (length <= 0)
            throw std::invalid_argument("Length must be positive");

        // Create tasks 0..length-1 where task i waits for task i+1.
        // However, we'll create them (add them to the pool) in a random order.
        // Otherwise, these tests are boring.
        auto indices = std::unique_ptr<int[]>(new int[length]);
        auto inverseIndices = std::unique_ptr<int[]>(new int[length]);
        for (int i = 0; i < length; i++) {
            indices[i] = i;
            inverseIndices[i] = i;
        }
        // Randomize order that we create tasks in.
        // NOLINTNEXTLINE(cert-msc51-cpp)
        std::mt19937 engine(1337);
        std::uniform_int_distribution<int> dist(0, length - 1);
        for (int i = 0; i < length; i++) {
            const auto r = dist(engine);
            std::swap(indices[i], indices[r]);
        }
        for (int i = 0; i < length; i++) {
            inverseIndices[indices[i]] = i;
        }

        tasks.resize(length);
        for (int i = 0; i < length; i++) {
            tasks[i] = makeWaiter(indices[i],
                inverseIndices[(indices[i] + 1) % length]);
        }

        {
            std::lock_guard lock(mutex);
            allTasksCreated = true;
        }
        taskStarted.notify_all();
    }

    std::shared_ptr<std::thread> getFirst() {
        return tasks[0];
    }
};

void selfDeadlock() {
    std::mutex m;
    std::lock_guard l1(m);
    std::lock_guard l2(m);
}

void mutexAndJoin() {
    pthread_setname_np(pthread_self(), "outer");
    std::mutex m;
    std::lock_guard outerLock(m);
    std::thread t([&] {
        pthread_setname_np(pthread_self(), "inner");
        std::lock_guard innerLock(m);
    });
    t.join();
}

int main() {
    mutexAndJoin();
    //selfDeadlock();
    //JoinCycle c(5);
    //c.getFirst()->join();
    return 0;
}
