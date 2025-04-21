#include "lib/main.h"

void startProgram(const char *program, char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        cerr << "err" << endl; 
        exit(1);
    } else if (pid == 0) {        
        execvp(program, argv);
        cerr << "err launch: " << program << endl; 
        exit(1);
    } else {
        cout << "Made process " << program << " PID: " << pid << endl;
    }
}

int main() {
  cout << "parent (main) pid: " << getpid () << endl;
  vector<pair<const char *, char *const *> > programs
      = { { "./obj/my_app4", nullptr },
          { "./obj/my_app5", nullptr },
          { "./obj/my_app6", nullptr } };

  for (const auto &program : programs)
    {
      startProgram (program.first, program.second);
    }

    int status; 
    while (wait(&status) > 0); // do kill -USR1 "pid" to send signals, pstree -p pid for other fun stuff
    cout << "Every child died pog" << endl;
    
       return 0;
}
