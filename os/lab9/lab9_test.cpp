#include <chrono>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;
using namespace chrono;

#define COUNT 1000

int sh = 0;
int turn = 0;
pthread_spinlock_t lock;

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
my_thread0_safe_spin (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      while (turn)
        ;
      sh += 1;
      turn = 1;
      usleep (1);
    }
  return NULL;
}
void *
my_thread1_safe_spin (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      while (!turn)
        ;
      sh += 2;
      turn = 0;
      usleep (1);
    }
  return NULL;
}

void *
my_thread0_safe_lock (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      pthread_spin_lock (&lock);
      sh += 1;
      pthread_spin_unlock (&lock);
      usleep (1);
    }
  return NULL;
}
void *
my_thread1_safe_lock (void *arg)
{
  for (int i = 0; i < COUNT; i++)
    {
      pthread_spin_lock (&lock);
      sh += 2;
      pthread_spin_unlock (&lock);
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

  pthread_create (&th_id[0], NULL, my_thread0_safe_spin, NULL);
  pthread_create (&th_id[1], NULL, my_thread1_safe_spin, NULL);
  pthread_join (th_id[0], NULL);
  pthread_join (th_id[1], NULL);

  end = system_clock::now ();

  cout << "spin blocker count: " << sh << endl;
  elapsed_seconds = end - start;
  cout << "time elapsed: " << elapsed_seconds.count () << endl;

  sh = 0;

  start = system_clock::now ();

  pthread_spin_init (&lock, PTHREAD_PROCESS_PRIVATE);
  pthread_create (&th_id[0], NULL, my_thread0_safe_lock, NULL);
  pthread_create (&th_id[1], NULL, my_thread1_safe_lock, NULL);
  pthread_join (th_id[0], NULL);
  pthread_join (th_id[1], NULL);
  pthread_spin_destroy (&lock);
  end = system_clock::now ();

  cout << "spin blocker lock count: " << sh << endl;
  elapsed_seconds = end - start;
  cout << "time elapsed: " << elapsed_seconds.count () << endl;
}