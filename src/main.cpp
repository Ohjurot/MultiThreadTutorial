#define _USE_MATH_DEFINES
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include <cassert>
#include <atomic>

std::atomic<unsigned int> doneWork;

double work(unsigned int idx, double input) {
    doneWork.fetch_add(1, std::memory_order_seq_cst);
    return abs(sin(input * idx)) * input;
}

void doWork(double* data, unsigned int count, unsigned int offset = 0) {
    for (unsigned int i = offset; i < (offset + count); i++) {
        data[i] = work(i, data[i]);
    }
}

void doWorkMT(double* data, unsigned int count, unsigned int threadCount) {
    // Work per thread
    unsigned int wpt = count / threadCount;

    // Schedule word
    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < threadCount; i++) {
        // Create thread
        threads.push_back(std::thread(&doWork, data, wpt, i * wpt));
    }

    // Wait for threads to finish
    for (auto it = threads.begin(); it != threads.end(); it++) {
        it->join();
    }
}

int main() {
    // Heavy work count
    const unsigned int workCount = 1024 * 1024 * 32;
    // Genneratew work
    std::cout << "=== GENNERATE WORK ===" << std::endl;
    double* workData = (double*) malloc(sizeof(double) * workCount);
    std::srand(time(0));
    for (unsigned int i = 0; i < workCount; i++) {
        workData[i] = std::rand() / (double)RAND_MAX;
    }

    // Work and time
    std::cout << "=== START WORK ===" << std::endl;
    auto tStart = std::chrono::steady_clock::now();
    doWorkMT(workData, workCount, 4);
    auto tStop = std::chrono::steady_clock::now();

    // Print timing result
    std::cout << std::endl << "Timings: " << std::endl
        << "NS: " << std::chrono::duration_cast<std::chrono::nanoseconds>(tStop - tStart).count() << std::endl
        << "US: " << std::chrono::duration_cast<std::chrono::microseconds>(tStop - tStart).count() << std::endl
        << "MS: " << std::chrono::duration_cast<std::chrono::milliseconds>(tStop - tStart).count() << std::endl
        << " S: " << std::chrono::duration_cast<std::chrono::seconds>(tStop - tStart).count() << std::endl;

    // Assert my work
    assert(doneWork.load(std::memory_order_relaxed) == workCount && "Work success");

    // Free
    free(workData);
    return 0;
}
