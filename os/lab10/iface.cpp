#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>

#define NUM_THREADS 2

using namespace std;

long long total_sum = 0;
pthread_mutex_t mut;
pthread_spinlock_t spinlock;

struct ThreadData
{
    vector<int> *array;
    int start, end;
    int id;
};

void* calcPartialSum_Mutex(void *arg) {
    ThreadData *data = static_cast<ThreadData*>(arg);   
    
    for (int i = data->start; i < data->end; i++) {
        pthread_mutex_lock(&mut);
        total_sum += (*data->array)[i];
        pthread_mutex_unlock(&mut);
    }

    return NULL;
}

void* calcPartialSum_Spin(void *arg) {
    ThreadData *data = static_cast<ThreadData*>(arg);   
    
    for (int i = data->start; i < data->end; i++) {
        pthread_spin_lock(&spinlock);
        total_sum += (*data->array)[i];
        pthread_spin_unlock(&spinlock);
    }

    return NULL;
}

int turn = 0;
int flagReady[2] = {1, 1};

void EnterCriticalRegion (int id)
{
  flagReady[id] = 1;
  turn = 1 - id;
  while (turn == 1 - id && flagReady[1 - id] == 1)
    ;
}

void LeaveCriticalRegion (int id)
{
  flagReady[id] = 0;
}

void* calcPartialSum_Peterson(void *arg) {
    ThreadData *data = static_cast<ThreadData*>(arg);

    for (int i = data->start; i < data->end; i++) {
        EnterCriticalRegion(data->id);
        total_sum += (*data->array)[i];
        LeaveCriticalRegion(data->id);
    }

    return NULL;
}

int main() {
  const int n = 1e8;
  vector<int> data (n, 1);
  chrono::time_point<chrono::system_clock> start, end;
  pthread_t threads[NUM_THREADS];
  ThreadData threadData[NUM_THREADS];
  int chunk_size = data.size () / NUM_THREADS;

  for (int i = 0; i < NUM_THREADS; i++)
    {
      threadData[i].array = &data;
      threadData[i].start = i * chunk_size;
      threadData[i].end = (i == NUM_THREADS - 1)
                              ? data.size ()
                              : threadData[i].start + chunk_size;
      threadData[i].id = i;
    }

    pthread_mutex_init(&mut, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_create (&threads[i], NULL, calcPartialSum_Mutex, &threadData[i]);
    }

    start = chrono::system_clock::now ();


    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); 
    }
    
    end = chrono::system_clock::now();

    chrono::duration<double> elapsed_seconds = end - start;

    cout << "\n\tpthread.h MUTEX" << endl;

    cout << "Time elapsed: " << elapsed_seconds.count () << "s" << endl;

    cout << "sum = " << total_sum << endl;

    total_sum = 0;
    pthread_mutex_destroy(&mut);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    start = chrono::system_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, calcPartialSum_Spin, &threadData[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); 
    }
    
    end = chrono::system_clock::now();

    elapsed_seconds = end - start;

    cout << "\n\tpthread.h SPINLOCK" << endl;

    cout << "Time elapsed: " << elapsed_seconds.count () << "s" << endl;

    cout << "sum = " << total_sum << endl;

    pthread_spin_destroy(&spinlock);
    if (NUM_THREADS != 2)
        return 0;
    
    total_sum = 0;

    start = chrono::system_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, calcPartialSum_Peterson, &threadData[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL); 
    }
    
    end = chrono::system_clock::now();

    elapsed_seconds = end - start;

    cout << "\n\tpthread.h PETERSON" << endl;

    cout << "Time elapsed: " << elapsed_seconds.count () << "s" << endl;

    cout << "sum = " << total_sum << endl;

    return 0;
}