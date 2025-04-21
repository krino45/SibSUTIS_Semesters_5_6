#include <arpa/inet.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 81

void
sigchld_handler (int s)
{
  int saved_errno = errno;
  while (waitpid (-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

int
main ()
{
  int sockMain, length, msgLength;
  struct sockaddr_in servAddr, clientAddr;
  char buf[BUFLEN];
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

  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction (SIGCHLD, &sa, NULL) == -1)
    {
      perror ("Oшибка установки обработчика SIGCHLD");
      exit (1);
    }

  int socketfd;
  for (;;)
    {
      length = sizeof (clientAddr);
      bzero (buf, BUFLEN);
      if ((socketfd = accept (sockMain, (struct sockaddr *)&clientAddr,
                              (socklen_t *)&length))
          < 0)
        {
          if (errno == EBADF)
            {
              // Основной сокет уже закрыт — выходим из цикла или завершаем
              // работу сервера
              break;
            }
          perror ("Ошибка accept.");
          continue;
        }

      pid_t pid = fork ();

      if (pid < 0)
        {
          perror ("Ошибка fork.");
          close (socketfd);
          continue;
        }
      else if (pid == 0)
        {
          close (sockMain);
          int n;
          while ((n = read (socketfd, buf, BUFLEN - 1)) > 0)
            {
              buf[n] = '\0';

              printf ("SERVER(%d): IP адрес клиента: %s\n", getpid (),
                      inet_ntoa (clientAddr.sin_addr));
              printf ("SERVER(%d): PORT клиента: %d\n", getpid (),
                      ntohs (clientAddr.sin_port));
              printf ("SERVER(%d): Длина сообщения - %d\n", getpid (),
                      msgLength);
              printf ("SERVER(%d): Сообщение: %s\n\n", getpid (), buf);
            }
          if (n < 0)
            {
              perror ("Ошибка чтения из сокета в подпроцессе");
            }
        }
      else if (pid > 0)
        {
          close (socketfd);
        }
    }
  close (sockMain);
}
