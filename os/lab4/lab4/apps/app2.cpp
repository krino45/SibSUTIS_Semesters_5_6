#include "../lib/main.h"

int main() {
    cout << "App 2 ";
    cout << "PID: " << getpid() << endl; // вывод id процесса
    execl("./obj/my_app3", "./obj/my_app3", (char *)nullptr);

    pause();
}   