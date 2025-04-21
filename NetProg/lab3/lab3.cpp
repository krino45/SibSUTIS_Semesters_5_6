#include <arpa/inet.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 81

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *file;

void *
handle_client (void *fd)
{
  int clientfd = *(int *)fd;
  free (fd);
  char buf[BUFLEN];
  int n;

  while ((n = read (clientfd, buf, BUFLEN - 1)) > 0)
    {
      buf[n] = '\0';
      printf ("SERVER(%d) Получено: %s\n", gettid (), buf);
      pthread_mutex_lock (&file_mutex);
      fprintf (file, "Получено: %s\n", buf);
      fflush (file); // Принудительная запись в файл
      pthread_mutex_unlock (&file_mutex);
    }

  close (clientfd);
  return NULL;
}

int
main ()
{
  int sockMain, length, msgLength;
  struct sockaddr_in servAddr, clientAddr;
  file = fopen ("file.txt", "a");
  if ((sockMain = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("Сервер не может открыть socket для TCP.");
      exit (1);
    }
  explicit_bzero ((char *)&servAddr, sizeof (servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  servAddr.sin_port = 0;
  if (bind (sockMain, (struct sockaddr *)&servAddr, sizeof (servAddr)) < 0)
    {
      perror ("Связывание сервера неудачно.");
      exit (1);
    }
  length = sizeof (servAddr);
  if (getsockname (sockMain, (struct sockaddr *)&servAddr,
                   (socklen_t *)&length)
      < 0)
    {
      perror ("Вызов getsockname неудачен.");
      exit (1);
    }
  printf ("СЕРВЕР: номер порта - % d\n", ntohs (servAddr.sin_port));

  if (listen (sockMain, 5) < 0)
    {
      perror ("Ошибка listen.");
      exit (1);
    }

  int socketfd;
  for (;;)
    {
      length = sizeof (clientAddr);
      if ((socketfd = accept (sockMain, (struct sockaddr *)&clientAddr,
                              (socklen_t *)&length))
          < 0)
        {
          if (errno == EBADF)
            {
              break;
            }
          perror ("Ошибка accept.");
          continue;
        }
      pthread_t thread_id;
      int *client_sock = new int;
      *client_sock = socketfd;
      if (pthread_create (&thread_id, NULL, handle_client, client_sock) != 0)
        {
          perror ("Сервер: ошибка pthread_create");
          free (client_sock);
          close (socketfd);
          continue;
        }
      pthread_detach (thread_id);
    }
  fclose (file);
}
