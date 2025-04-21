#include "../lib/main.h"

void handler(int s)
{
    cout << "woo app 6\n";
}

int main() {
  signal (SIGUSR1, handler);
    cout << "App 6" << endl;
    pause ();
    return 0;
}