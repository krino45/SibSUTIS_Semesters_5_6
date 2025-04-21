#include <chrono>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <atomic>

using namespace std;
using namespace chrono;

#define COUNT 100000

atomic<int> sh (0);
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

void *
my_thread0 (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      sh += 1;
    }
  return NULL;
}
void *
my_thread1 (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      sh += 2;
    }
  return NULL;
}

void *
my_thread0_safe_peterson (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      sh.fetch_add (1, memory_order_relaxed);
    }
  return NULL;
}
void *
my_thread1_safe_peterson (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      sh.fetch_add (2, memory_order_relaxed);
    }
  return NULL;
}


int
main ()
{
  time_point<system_clock> start, end;

  pthread_t th_id[2];

  start = system_clock::now ();

  pthread_create (&th_id[0], NULL, my_thread0, NULL);
  pthread_create (&th_id[1], NULL, my_thread1, NULL);
  pthread_join (th_id[0], NULL);
  pthread_join (th_id[1], NULL);

  end = system_clock::now ();

  cout << "atomic count: " << sh << endl;
  duration<double> elapsed_seconds = end - start;
  cout << "time elapsed: " << elapsed_seconds.count () << endl;

  sh = 0;

  start = system_clock::now ();

  pthread_create (&th_id[0], NULL, my_thread0_safe_peterson, NULL);
  pthread_create (&th_id[1], NULL, my_thread1_safe_peterson, NULL);
  pthread_join (th_id[0], NULL);
  pthread_join (th_id[1], NULL);

  end = system_clock::now ();

  cout << "atomic fetch count: " << sh << endl;
  elapsed_seconds = end - start;
  cout << "time elapsed: " << elapsed_seconds.count () << endl;

  return 0;
}