#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#define BUFLEN 81

int
main (int argc, char **argv)
{
  int sock;
  struct sockaddr_in servAddr, clientAddr;
  struct hostent *hp;
  if (argc < 4)
    {
      printf ("ВВЕСТИ tcpclient имя_хоста порт сообщение\n");
      exit (1);
    }
  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("He могу получить socket\n");
      exit (1);
    }
  bzero ((char *)&servAddr, sizeof (servAddr));
  servAddr.sin_family = AF_INET;
  hp = ::gethostbyname (argv[1]);
  bcopy (hp->h_addr, &servAddr.sin_addr, hp->h_length);
  servAddr.sin_port = htons (atoi (argv[2]));
  bzero ((char *)&clientAddr, sizeof (clientAddr));
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  clientAddr.sin_port = 0;
  int i = atoi (argv[3]);
  if (i <= 0)
    {
      printf ("Нужно ввести положительное число.\n");
      exit (1);
    }
  char sendBuf[BUFLEN];

  if (connect (sock, (struct sockaddr *)&servAddr, sizeof (servAddr)) < 0)
    {
      perror ("Не получилось подключиться к серверу.");
      exit (1);
    }

  for (int ii = 0; ii < i; ii++)
    {
      int n = snprintf (sendBuf, BUFLEN, "%d", i);

      if (write (sock, sendBuf, n) < 0)
        {
          perror ("Проблемы с write.");
          exit (1);
        }
      printf ("CLIENT: Отправлено сообщение: %s\n", sendBuf);
      std::this_thread::sleep_for (std::chrono::seconds (i));
    }

  close (sock);
  return 0;
}
