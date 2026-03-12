#include <cstdio>
#include <thread>

int main() {
    std::thread t([&]{});
    t.join();

    std::mutex m;
    std::lock_guard lock(m);
    return 0;
}