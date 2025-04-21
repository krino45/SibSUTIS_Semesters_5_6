#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;


int main ()
{
  pid_t pid = fork ();
  switch (pid)
    {
    case -1:
    {
      cout << "error";
      break;
    }
    case 0:
    {
      cout << "child process, ppid == " << getpid() << "\n";
      while(true)
        ;
      break;
    }
    default:
    {
      cout << "parent process woo (pid == " << getpid () << ")\n";
      wait (&pid);
      break;
    }
    }
}