#include "dirent.h"
#include "fcntl.h"
#include "stdio.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "unistd.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sys/wait.h>
using namespace std;

int
main ()
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
        cout << "child process, ppid == " << getpid () << "\n";
        pid_t pid2 = fork ();
        if (pid2 < 0)
          {
            cout << "error";
          }
        else if (pid2 == 0)
          {
            cout << "im a second child pog " << pid2 << endl;
            while (1)
              ;
          }
        else
          {
            cout << "i have a child pog time to have another one \n";
            pid_t pid3 = fork ();
            if (pid3 < 0)
              {
                cout << "error";
              }
            else if (pid3 == 0)
              {
                cout << "im a third child of the first one pog " << pid3 << endl;
                while (1)
                  ;
              }
            else 
            {
              wait (&pid3);
            }
            wait (&pid2);
          }
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