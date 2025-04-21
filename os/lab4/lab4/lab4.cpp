#include "lib/main.h"

int main() {
    cout << "Main App ";
    cout << "PID: " << getpid() << endl;
    execl("./obj/my_app1", "./obj/my_app1", (char *)nullptr);
    pause();
}   