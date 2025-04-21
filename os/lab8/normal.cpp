#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

long long seqSum(vector<int> &data) {
    long long sum = 0;

    for (int value : data) {
        sum += value;
    }

    return sum;
}

int main() {
    const int n = 1e8;
    vector<int> data(n, 1);

    chrono::time_point<chrono::system_clock> start, end;

    start = chrono::system_clock::now();
    long long total_sum = seqSum(data);
    end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;
    time_t end_time = chrono::system_clock::to_time_t(end);
 
    cout << "\n\tnormal" << endl;

    cout << "time elapsed: " << elapsed_seconds.count() << "s" << endl;

    cout << "sum = " << total_sum << endl;

    return 0;
}