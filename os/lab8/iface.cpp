#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>

#define NUM_THREADS 16

using namespace std;

struct ThreadData
{
    vector<int> *array;
    long long sum;
    int start, end;
};

void* calcPartialSum(void *arg) {
    ThreadData *data = static_cast<ThreadData*>(arg);   
    
    data->sum = 0;
    for (int i = data->start; i < data->end; i++) {
        data->sum += (*data->array)[i];
    }

    return NULL;
}

int main() {
    const int n = 1e8;
    vector<int> data(n, 1);
    chrono::time_point<chrono::system_clock> start, end;

    start = chrono::system_clock::now();

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];
    int chunk_size = data.size() / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].array = &data;
        threadData[i].sum = 0;
        threadData[i].start = i * chunk_size;
        threadData[i].end = (i == NUM_THREADS - 1) ? data.size() : threadData[i].start + chunk_size;

        pthread_create(&threads[i], NULL, calcPartialSum, &threadData[i]);
    }

    long long total_sum = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); // ожидаем завершения потока (считаем подсумму)
        total_sum += threadData[i].sum;
    }
    
    end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;
    time_t end_time = chrono::system_clock::to_time_t(end);

    cout << "\n\tpthread.h" << endl;

    cout << "Time elapsed: " << elapsed_seconds.count () << "s" << endl;

    cout << "sum = " << total_sum << endl;

    return 0;
}