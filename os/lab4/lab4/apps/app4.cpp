#include "../lib/main.h"

void handler(int s)
{
    cout << "woo app 4\n";
}

int main() {
  signal (SIGUSR1, handler);
  cout << "App 4" << endl;
  pause ();
  return 0;
}