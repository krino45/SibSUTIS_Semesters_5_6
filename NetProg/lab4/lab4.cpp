#include <arpa/inet.h>
#include <bits/types/struct_timeval.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFLEN 81
#define MAX_CLIENTS 32

bool
handle_client (int clientfd)
{
  char buf[BUFLEN];
  int n = read (clientfd, buf, BUFLEN - 1);
  if (!n)
    return false;
  buf[n] = '\0';
  printf ("SERVER(fd:%d): Получено: %s\n", clientfd, buf);
  return true;
}

int
main (int argc, char **argv)
{
  int supress_timeout_output = 0;
  if (argc > 1 && strcmp (argv[1], "-q") == 0)
    supress_timeout_output = 1;
  int sockMain, length, msgLength;

  fd_set fd_in;

  FD_ZERO (&fd_in);

  struct sockaddr_in servAddr, clientAddr;
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
  FD_SET (sockMain, &fd_in);
  int largest_socket_fd = sockMain;

  for (;;)
    {
      fd_set fd_in_copy = fd_in;
      switch (select (largest_socket_fd + 1, &fd_in_copy, NULL, NULL, NULL))
        {
        case -1:
          perror ("select");
          exit (1);
        case 0:
          if (!supress_timeout_output)
            printf ("Select timeout\n");
        default:
          if (FD_ISSET (sockMain, &fd_in_copy))
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
              FD_SET (socketfd, &fd_in);
              if (socketfd > largest_socket_fd)
                largest_socket_fd = socketfd;
            }
          for (int fd = sockMain + 1; fd <= largest_socket_fd; fd++)
            {
              if (FD_ISSET (fd, &fd_in_copy))
                if (!handle_client (fd))
                  {
                    close (fd);
                    FD_CLR (fd, &fd_in);
                  }
            }

          break;
        }
    }
}
