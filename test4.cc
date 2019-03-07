#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

void 

void jeannie(){

}

int main()
{
  if (thread_libinit( (thread_startfunc_t) jeannie, (void *) 100)){
    cout << "thread_libinit failed\n";
    exit(1);
  }

}
