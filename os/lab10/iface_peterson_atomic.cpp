#include <iostream>
#include <pthread.h>
#include <vector>
#include <atomic>
#include <chrono>

#define NUM_THREADS 2

using namespace std;

atomic<long long> total_sum(0);
atomic<int> turn;
atomic<bool> flag[NUM_THREADS]; 

struct ThreadData {
    vector<int>* array;
    int id;
    int start;
    int end;
};

void EnterCriticalRegion(int id) {
    flag[id] = true;
    turn = 1 - id;
    while (flag[1 - id] && turn == 1 - id)
        ;
}

void LeaveCriticalRegion(int id) {
    flag[id] = false;
}

void* calcPartialSum(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    for (int i = data->start; i < data->end; i++) {
        EnterCriticalRegion(data->id);
        total_sum += (*data->array)[i];
        LeaveCriticalRegion(data->id);
    }
    return nullptr;
}

int main() {
    const int n = 1e8;
    vector<int> data(n, 1); 

    cout << "<====== ATOMIC PETERSON WITH PTHREADS =======>\n";

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];
    int chunk_size = n / NUM_THREADS;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].array = &data; 
        threadData[i].id = i;
        threadData[i].start = i * chunk_size;
        threadData[i].end = (i == NUM_THREADS - 1) ? n : (i + 1) * chunk_size;
        pthread_create(&threads[i], nullptr, calcPartialSum, &threadData[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;

    cout << "Time elapsed: " << elapsed_seconds.count() << "s" << endl;
    cout << "Total sum = " << total_sum.load() << endl;

    return 0;
}
