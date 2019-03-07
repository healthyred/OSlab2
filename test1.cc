#include <iostream>
#include "thread.h"
#include <assert.h>

//Test to call 2 libinit
using namespace std;

int g=0;



void
parent(void *a)
{
  int arg;
  arg = (intptr_t) a;

  cout << "parent called with arg " << arg << endl;

  loop( (void *) "parent thread");
}

int
main()
{
  if (thread_libinit( (thread_startfunc_t) parent, (void *) 100)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }

  if (thread_libinit( (thread_startfunc_t) parent, (void *) 200)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }
}
