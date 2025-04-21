#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


char sh[6];
void *Thread (void *pParams);
int turn = 1;

int
main (void)
{
  pthread_t thread_id;
  pthread_create (&thread_id, NULL, &Thread, NULL);
  while (1)
    {
      while (turn)
        ;
      printf ("%s\n", sh);
      turn = 1;
    }
  return 0;
}

void *
Thread (void *pParams)
{
  int counter = 0;
  while (1)
    {
      if (counter % 2)
        {
          while (!turn)
            ;
          sh[0] = 'H';
          sh[1] = 'e';
          sh[2] = 'l';
          sh[3] = 'l';
          sh[4] = 'o';
          sh[5] = '\0';
          turn = 0;
        }
      else
        {
          while (!turn)
            ;
          sh[0] = 'B';
          sh[1] = 'y';
          sh[2] = 'e';
          sh[3] = '_';
          sh[4] = 'u';
          sh[5] = '\0';
          turn = 0;
        }
      counter++;
      usleep (100000);
    }
  return NULL;
}