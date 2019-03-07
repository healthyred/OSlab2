//Purposely inducing deadlock ensuring exit

#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

int g=0;

void
loop(void *a)
{
  char *id;
  int i;

  id = (char *) a;
  cout <<"loop called with id " << (char *) id << endl;

  thread_wait(0,a);
}

void
parent(void *a)
{
  int arg;
  arg = (intptr_t) a;

  cout << "parent called with arg " << arg << endl;
  if (thread_create((thread_startfunc_t) loop, (void *) "child thread")) {
    cout << "thread_create failed\n";
    exit(1);
  }

  loop( (void *) "parent thread");
}

int
main()
{
  if (thread_libinit( (thread_startfunc_t) parent, (void *) 100)) {
    cout << "thread_libinit failed\n";
    exit(1);
  }
}
