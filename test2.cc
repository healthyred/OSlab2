//Calling Thread Create without libinit
#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

int main()
{
  if (thread_create((thread_startfunc_t) loop, (void *) "child thread")) {
    cout << "thread_create failed\n";
    exit(1);
  }
}