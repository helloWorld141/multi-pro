#include <iostream>
#include <thread>
#include <atomic>
#include <random> // uniform_int_distribution, mt19937
#include <unistd.h>

#define THREAD_NUM 2
#define N_PRODUCER 1
#define BUFFER_SIZE 10
#define SEED 25

std::mt19937 randomEngine;
std::uniform_int_distribution<int32_t> uint_dist(0, 100);

int buffer[BUFFER_SIZE];
std::atomic<int> count(0);
std::atomic_flag full = ATOMIC_FLAG_INIT;
std::atomic_flag empty;

void producer() {
    while (1) {
        // create
        int x = uint_dist(randomEngine);
        //std::cout << "Producing " << x << std::endl;
        while (full.test_and_set()) {}
        // critical region - BEGIN
        buffer[count] = x;
        count++;
        if (count < BUFFER_SIZE) full.clear();
        if (count > 0) empty.clear();
        // critical region - END
        //// if thread is locked/busy waiting for full, then it will not reach end of this block if not entering critical region
        //sleep(1);
    }
}

void consumer() {
    while (1) {
        int x = -1;
        while (empty.test_and_set()) {}
        // critical region - BEGIN
        x = buffer[count-1];
        count--;
        if (count > 0) empty.clear();
        if (count < BUFFER_SIZE) full.clear();
        // critical region - END
        // if it does not print -1, meaning the thread is not blocked but just skipped the critical region
        std::cout << "Consuming " << x << std::endl;
        //sleep(1);
    }
}

int main(int arg, char* argv[]) {
    std::cout << "Start main thread. Creating child threads..." << std::endl;    
    randomEngine.seed(SEED);
    // lock the consumer in the first place
    empty.test_and_set();
    std::thread threads[THREAD_NUM];
    for (int i=0; i<N_PRODUCER; i++) {
        threads[i] = std::thread(producer);
    }
    for (int i=N_PRODUCER; i<THREAD_NUM; i++) {
        threads[i] = std::thread(consumer);
    }
    std::cout << "Synchronizing all threads..." << std::endl;
    for (auto& th: threads) th.join();
    
    return 0;
}
