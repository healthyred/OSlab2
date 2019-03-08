#include "thread.h"
#include <assert.h>
#include <iostream>

using namespace std;

void child(void *a) {
  int i = (intptr_t) a;
  cout <<"**child " << i << " created" << endl;

  //signal the last thread created
  if (i>0) {
    cout <<"signal child " << i-1 << endl;
    thread_signal(0, i-1);
  }

  if (i<4) {
    //acquire lock 0
    thread_lock(0);

    //wait for CV i (note that we aren't looping here, but we often should)
    cout <<"child " << i << " is waiting" << endl;
    thread_wait(0, i);
    cout <<"child " << i << " received signaled" << endl;

    //release 0
    thread_unlock(0);
  }

}

void parent(void *a) {
  int arg, i;
  arg = (intptr_t) a;
  
  cout << "parent created with arg " << arg << endl;

  for (i=0; i<arg; i++) {
    //create thread that will run "child" function, pass i as parameter to child
    if (thread_create((thread_startfunc_t) child, (void *) i)) {
      cout << "thread_create failed" << endl;
      exit(1);
    }
    
    //usually we don't want threads to yield voluntarily, but it allows us to experiment a bit
    thread_yield();
  }

}

int main() {
  //initialize thread library, run "parent" function, pass 5 as parameter to parent
  if (thread_libinit( (thread_startfunc_t) parent, (void *) 5)) {
    cout << "thread_libinit failed" << endl;
    exit(1);
  }
}
