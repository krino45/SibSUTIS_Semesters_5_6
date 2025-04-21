#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORT 80
#define BUFFER_SIZE 1024
#define MAX_MESSAGES 1000
#define HISTORY_FILE "chat_history.txt"
#define POLL_TIMEOUT 10

typedef struct
{
  char messages[MAX_MESSAGES][BUFFER_SIZE];
  int message_count;
  int oldest_message_idx;
} Chat;

int server_fd;
Chat chatroom = { .message_count = 0, .oldest_message_idx = 0 };
pthread_mutex_t chatroom_mutex = PTHREAD_MUTEX_INITIALIZER;

void
get_timestamp (char *buffer, size_t size)
{
  time_t now = time (NULL);
  struct tm *t = localtime (&now);
  strftime (buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void
load_history ()
{
  FILE *file = fopen (HISTORY_FILE, "r");
  if (!file)
    {
      printf ("No history file found.\n");
      return;
    }

  char line[BUFFER_SIZE];
  while (fgets (line, sizeof (line), file))
    {
      line[strcspn (line, "\n")] = '\0';
      pthread_mutex_lock (&chatroom_mutex);
      strncpy (chatroom.messages[chatroom.message_count], line,
               BUFFER_SIZE - 1);
      chatroom.message_count++;
      if (chatroom.message_count >= MAX_MESSAGES)
        {
          chatroom.message_count = 0;
          chatroom.oldest_message_idx
              = (chatroom.oldest_message_idx + 1) % MAX_MESSAGES;
        }
      pthread_mutex_unlock (&chatroom_mutex);
    }

  fclose (file);
  printf ("Loaded history from %s.\n", HISTORY_FILE);
}

void
save_history ()
{
  FILE *file = fopen (HISTORY_FILE, "w");
  if (!file)
    {
      perror ("Failed to save history");
      return;
    }

  pthread_mutex_lock (&chatroom_mutex);
  int index = chatroom.oldest_message_idx;
  for (int i = 0; i < chatroom.message_count; i++)
    {
      fprintf (file, "%s\n", chatroom.messages[index]);
      index = (index + 1) % MAX_MESSAGES;
    }
  pthread_mutex_unlock (&chatroom_mutex);

  fclose (file);
  printf ("Chat history saved to %s.\n", HISTORY_FILE);
}

void
sigint_handler (int signum)
{
  printf ("\nSIGINT. Saving chat history.\n");
  save_history ();
  close (server_fd);
  exit (0);
}

void
send_http_response (int client_socket, const char *status,
                    const char *content_type, const char *body)
{
  char headers[BUFFER_SIZE];
  snprintf (headers, sizeof (headers),
            "HTTP/1.1 %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n\r\n",
            status, content_type, strlen (body));

  send (client_socket, headers, strlen (headers), 0);
  send (client_socket, body, strlen (body), 0);
}

void
handle_get (int client_socket, int client_message_count)
{
  time_t start_time = time (NULL);

  while (1)
    {
      pthread_mutex_lock (&chatroom_mutex);
      int message_count = chatroom.message_count;
      pthread_mutex_unlock (&chatroom_mutex);

      if (message_count > client_message_count)
        {
          printf ("Message count > client_message_count: %d > %d\n",
                  message_count, client_message_count);
          char body[BUFFER_SIZE * MAX_MESSAGES] = "[";

          pthread_mutex_lock (&chatroom_mutex);
          int index = chatroom.oldest_message_idx;
          for (int i = 0; i < message_count; i++)
            {
              strcat (body, "\"");
              strcat (body, chatroom.messages[index]);
              strcat (body, "\"");
              if (i < message_count - 1)
                {
                  strcat (body, ",");
                }
              index = (index + 1) % MAX_MESSAGES;
            }
          pthread_mutex_unlock (&chatroom_mutex);

          strcat (body, "]");
          send_http_response (client_socket, "200 OK", "application/json",
                              body);
          break;
        }
      else if (time (NULL) - start_time >= POLL_TIMEOUT)
        {
          printf ("Timeout.\n");
          send_http_response (client_socket, "204 No Content",
                              "application/json", "[]");
          break;
        }
      else
        {
          printf ("Waiting...\n");
          sleep (1);
        }
    }

  close (client_socket);
}

void
handle_post (int client_socket, const char *request_body)
{
  char nickname[BUFFER_SIZE] = "", message[BUFFER_SIZE] = "";
  sscanf (request_body, "{\"nickname\":\"%[^\"]\",\"message\":\"%[^\"]\"}",
          nickname, message);

  char timestamp[64];
  get_timestamp (timestamp, sizeof (timestamp));

  char formatted_message[BUFFER_SIZE];
  snprintf (formatted_message, BUFFER_SIZE, "[%s] %s: %s", timestamp, nickname,
            message);

  pthread_mutex_lock (&chatroom_mutex);
  int idx
      = (chatroom.oldest_message_idx + chatroom.message_count) % MAX_MESSAGES;
  strncpy (chatroom.messages[idx], formatted_message, BUFFER_SIZE - 1);

  if (chatroom.message_count < MAX_MESSAGES)
    {
      chatroom.message_count++;
    }
  else
    {
      chatroom.oldest_message_idx
          = (chatroom.oldest_message_idx + 1) % MAX_MESSAGES;
    }
  pthread_mutex_unlock (&chatroom_mutex);

  printf ("Message #%d: %s\n", chatroom.message_count, formatted_message);

  send_http_response (client_socket, "200 OK", "text/plain",
                      "Message received");
}

void
serve_html_page (int client_socket)
{
  FILE *file = fopen ("index.html", "r");
  if (!file)
    {
      send_http_response (client_socket, "500 Internal Server Error",
                          "text/plain", "Failed to load HTML file");
      return;
    }

  fseek (file, 0, SEEK_END);
  long size = ftell (file);
  rewind (file);

  char *html = malloc (size + 1);
  fread (html, 1, size, file);
  html[size] = '\0';
  fclose (file);

  send_http_response (client_socket, "200 OK", "text/html", html);
  free (html);
}

// routing reqs
void
handle_client (int client_socket)
{
  char buffer[BUFFER_SIZE];
  int bytes_read = recv (client_socket, buffer, sizeof (buffer) - 1, 0);
  if (bytes_read <= 0)
    {
      close (client_socket);
      return;
    }
  buffer[bytes_read] = '\0';

  if (strstr (buffer, "GET / "))
    {
      serve_html_page (client_socket);
    }
  else if (strstr (buffer, "GET /messages"))
    {
      int client_message_count = -1;
      sscanf (buffer, "GET /messages?lastMessageCount=%d",
              &client_message_count);
      handle_get (client_socket, client_message_count);
    }
  else if (strstr (buffer, "POST /messages"))
    {
      char *body = strstr (buffer, "\r\n\r\n");
      if (body)
        {
          body += 4; // Skip over "\r\n\r\n"
          handle_post (client_socket, body);
        }
      else
        {
          send_http_response (client_socket, "400 Bad Request", "text/plain",
                              "Invalid POST request");
        }
    }
  else
    {
      send_http_response (client_socket, "404 Not Found", "text/plain",
                          "Resource not found");
    }

  close (client_socket);
}

int
main ()
{
  signal (SIGINT, sigint_handler);
  signal (SIGPIPE, SIG_IGN); // without this, after all sockets are closed the
                             // server crashes

  int client_socket;
  int opt = 1;
  struct sockaddr_in address;
  socklen_t addrlen = sizeof (address);

  load_history ();

  if ((server_fd = socket (AF_INET, SOCK_STREAM, 0)) == 0)
    {
      perror ("Socket failed");
      exit (EXIT_FAILURE);
    }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (PORT);

  if (bind (server_fd, (struct sockaddr *)&address, sizeof (address)) < 0)
    {
      perror ("Bind failed");
      close (server_fd);
      exit (EXIT_FAILURE);
    }

  if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) < 0)
    {
      perror ("setsockopt failed");
      close (server_fd);
      exit (EXIT_FAILURE);
    }

  if (listen (server_fd, 5) < 0)
    {
      perror ("Listen failed");
      close (server_fd);
      exit (EXIT_FAILURE);
    }

  printf ("Server running on http://localhost:%d\n", PORT);

  while (1)
    {
      if ((client_socket
           = accept (server_fd, (struct sockaddr *)&address, &addrlen))
          < 0)
        {
          perror ("Accept failed");
          continue;
        }

      char client_ip[INET_ADDRSTRLEN];
      int client_port;

      if (getpeername (client_socket, (struct sockaddr *)&address,
                       (socklen_t *)&addrlen)
          == -1)
        {
          perror ("getpeername");
          close (client_socket);
          continue;
        }

      inet_ntop (AF_INET, &address.sin_addr, client_ip, sizeof (client_ip));
      client_port = ntohs (address.sin_port);

      printf ("Connection accepted from %s:%d\n", client_ip, client_port);

      pthread_t thread;
      pthread_create (&thread, NULL, (void *)handle_client,
                      (void *)(int)client_socket);

      pthread_detach (thread);
      // handle_client (client_socket);
    }

  save_history ();
  close (server_fd);
  return 0;
}
