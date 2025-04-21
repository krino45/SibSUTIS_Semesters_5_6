#include "../lib/main.h"

int main() {
    cout << "App 1 ";
    cout << "PID: " << getpid() << endl; // вывод id процесса
    execl("./obj/my_app2", "./obj/my_app2", (char *)nullptr);
    pause ();
}   