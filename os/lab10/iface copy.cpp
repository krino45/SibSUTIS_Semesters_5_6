#include <chrono>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <atomic>

#define NUM_THREADS 2

using namespace std;

long long total_sum = 0;
atomic<long long> total_sum2 (0);
pthread_spinlock_t spinlock;

struct ThreadData
{
  vector<int> *array;
  int start, end;
  int id;
  int order;
};

void *
calcPartialSum_Spin (void *arg)
{
  ThreadData *data = static_cast<ThreadData *> (arg);

  for (int i = data->start; i < data->end; i++)
    {
      pthread_spin_lock (&spinlock);
      total_sum += (*data->array)[i];
      pthread_spin_unlock (&spinlock);
    }

  return NULL;
}

void *
calcPartialSum_Atomic (void *arg)
{
  ThreadData *data = static_cast<ThreadData *> (arg);

  for (int i = data->start; i < data->end; i++)
    {
      switch (data->order) {
        case 0:
          total_sum2.fetch_add((*data->array)[i], memory_order_relaxed);
          break;
        case 1:
          total_sum2.fetch_add((*data->array)[i], memory_order_consume);
          break;
        case 2:
          total_sum2.fetch_add((*data->array)[i], memory_order_acquire);
          break;
        case 3:
          total_sum2.fetch_add((*data->array)[i], memory_order_release);
          break;
        case 4:
          total_sum2.fetch_add((*data->array)[i], memory_order_acq_rel);
          break;
        case 5:
          total_sum2.fetch_add((*data->array)[i], memory_order_seq_cst);
          break;
      }
    }

  return NULL;
}

int
main ()
{
  int n = 5e7;
  vector<int> data (n, 1);
  chrono::time_point<chrono::system_clock> start, end;

  pthread_t threads[NUM_THREADS];
  ThreadData threadData[NUM_THREADS];
  int chunk_size = data.size () / NUM_THREADS;

  pthread_spin_init (&spinlock, PTHREAD_PROCESS_PRIVATE);
  start = chrono::system_clock::now ();

  for (int i = 0; i < NUM_THREADS; i++)
    {
      threadData[i].array = &data;
      threadData[i].start = i * chunk_size;
      threadData[i].end = (i == NUM_THREADS - 1)
                              ? data.size ()
                              : threadData[i].start + chunk_size;
      threadData[i].id = i;
      threadData[i].order = 0;

      pthread_create (&threads[i], NULL, calcPartialSum_Spin, &threadData[i]);
    }

  for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_join (threads[i], NULL);
    }

  end = chrono::system_clock::now ();

  chrono::duration<double> elapsed_seconds = end - start;

  cout << "\n<===== pthread.h SPINLOCK =====>" << endl;

  cout << "Time elapsed: " << elapsed_seconds.count () << "s" << endl;

  cout << "sum = " << total_sum << endl;

  pthread_spin_destroy (&spinlock);

  n = 5e8;

  vector<int> data2 (n, 1);

  for (int i = 0; i < NUM_THREADS; i++)
    {
      threadData[i].array = &data2;
      threadData[i].start = i * chunk_size;
      threadData[i].end = (i == NUM_THREADS - 1)
                              ? data2.size ()
                              : threadData[i].start + chunk_size;
    }

  for (int order = 0; order < 6; order++) {
    total_sum2 = 0;
    start = chrono::system_clock::now ();

    for (int i = 0; i < NUM_THREADS; i++)
      {
        threadData[i].order = order;
        pthread_create (&threads[i], NULL, calcPartialSum_Atomic, &threadData[i]);
      }

    for (int i = 0; i < NUM_THREADS; i++)
      {
        pthread_join (threads[i], NULL);
      }

    end = chrono::system_clock::now ();

    elapsed_seconds = end - start;

    cout << "\n<===== pthread.h ATOMIC (Order " << order << ") =====>" << endl;

    cout << "Time elapsed: " << elapsed_seconds.count () << "s" << endl;

    cout << "sum = " << total_sum2 << endl;
  }

  return 0;
}
