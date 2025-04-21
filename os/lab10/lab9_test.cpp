#include <chrono>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;
using namespace chrono;

#define COUNT 100000

int sh = 0;
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
      usleep (1);
    }
  return NULL;
}
void *
my_thread1 (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      sh += 2;
      usleep (1);
    }
  return NULL;
}

void *
my_thread0_safe_peterson (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      EnterCriticalRegion (0);
      sh += 1;
      LeaveCriticalRegion (0);
      usleep (1);
    }
  return NULL;
}
void *
my_thread1_safe_peterson (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      EnterCriticalRegion (1);
      sh += 2;
      LeaveCriticalRegion (1);
      usleep (1);
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

  cout << "unsafe count: " << sh << endl;
  duration<double> elapsed_seconds = end - start;
  cout << "time elapsed: " << elapsed_seconds.count () << endl;

  sh = 0;

  start = system_clock::now ();

  pthread_create (&th_id[0], NULL, my_thread0_safe_peterson, NULL);
  pthread_create (&th_id[1], NULL, my_thread1_safe_peterson, NULL);
  pthread_join (th_id[0], NULL);
  pthread_join (th_id[1], NULL);

  end = system_clock::now ();

  cout << "peterson block count: " << sh << endl;
  elapsed_seconds = end - start;
  cout << "time elapsed: " << elapsed_seconds.count () << endl;

  return 0;
}