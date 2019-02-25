/*This is the Disk Scheduler portion
By- Jian Lu and Vincent Lin
*/

#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "thread.h"
#include <iostream>

using namespace std;

void request_output(int requester, int track){
  cout << "requester " << requester << " track " << track << endl;
}

void service_output(int requester, int track){
  cout << "service requester " << requester << " track " << track << endl;
}

void request(void *a) {
		


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

void service(void *a)
{	
	int i = 2;
	while (a[i] != " "){
    //create thread that will run "child" function, pass i as parameter to child
	    if (thread_create((thread_startfunc_t) request, (void *) &a[i])) {
	      cout << "thread_create failed" << endl;
	      exit(1);
	    }
	    ++i;
    //usually we don't want threads to yield voluntarily, but it allows us to experiment a bit
    //thread_yield();
  }
}

/*Initiate some number of requester threads, and one service thread*/
int max_requests;
int requesters;


int main(int argc, char* argv[]){
	max_requests = atoi(argv[1]);
	cout<< max_requests <<endl;	
  	if (thread_libinit( (thread_startfunc_t) service, (void *) argv[])) 
	  {
	    cout << "thread_libinit failed" << endl;
	    exit(1);
	  }
	  return 0;
}
