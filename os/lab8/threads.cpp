#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;

mutex mtx;
void calcSum(vector<int> &data, long long &sum, int start, int end) {
    long long partial_sum = 0;
    for (int i = start; i < end; i++) {
        partial_sum += data[i];
    }

    lock_guard<mutex> lock(mtx);
    sum += partial_sum;
}


int main() {
    const int n = 1e8;
    int numThreads = 4;
    vector<int> data(n, 1);

    chrono::time_point<chrono::system_clock> start, end;

    start = chrono::system_clock::now();

    vector<thread> threads;
    int chunk_size = data.size() / numThreads;
    long long total_sum = 0;

    for (int i = 0; i < numThreads; i++) {
        int start = i * chunk_size;
        int end = (i == numThreads - 1) ? data.size() : start + chunk_size;
        threads.emplace_back(calcSum, ref(data), ref(total_sum), start, end);
    }

    for (int i = 0; i < numThreads; i++) {
        threads[i].join();
    }

    end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;
    time_t end_time = chrono::system_clock::to_time_t(end);
 
    cout << "\n\tpthread C++11" << endl;

    cout << "TIme elapsed: " << elapsed_seconds.count() << "s" << endl;

    cout << "sum = " << total_sum << endl;

    return 0;
}