/*This is the Disk Scheduler portion
By- Jian Lu and Vincent Lin
*/

#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "thread.h"
#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
using namespace std;

typedef tuple<int, int> SSTF;

int max_requests;
int live_requests;
int cond;
vector<SSTF> queue(0);


void request_output(int requester, int track){
  cout << "requester " << requester << " track " << track << endl;
}

void service_output(int requester, int track){
  cout << "service requester " << requester << " track " << track << endl;
}

void request(char *a) {		
  string file(a);
   int fileNum = file.back()-'0';
   ifstream stream;
   stream.open(file);
   if (stream.is_open()){
     string line;
     int i = 0;
     while(getline(stream, line)){
       int intLine = atoi(line.c_str());
       auto x = SSTF(intLine,fileNum);
       thread_wait(0, get<1>(x));
       queue.push_back(x);
       request_output(get<0>(x), get<1>(x));
       thread_signal(0, cond);	 
       thread_wait(0,get<1>(x));
     }
    }
    --live_requests;
}

void service(char **a)
{	
  int i = 2;
  char ** files = (char**) a;
  while (files[i] != '\0'){
    if (thread_create((thread_startfunc_t) request, (char *) files[i])) {
      cout << "thread_create failed" << endl;
      exit(1);
    }
    ++i;
  }
  cout<<"service request lock"<<endl;  
  cout<<"queue size"<<queue.size()<<endl;
  cout<<"live_request"<<live_requests<<endl;
  thread_lock(0);
  while(live_requests != 0){
    for(int i =live_requests;i>=0;i--){
      if(queue.size() < max_requests ||queue.size() < live_requests){
	cout<<"inti:"<<i<<endl;	
	thread_signal(0, i);
	thread_wait(0,cond);
      }
      else{
      sort(queue.begin(),queue.end());
      int x = get<1>(queue.front());
      service_output(get<0>(queue.front()),get<1>(queue.front()));
      queue.erase(queue.begin());
      thread_signal(0, x);
      thread_wait(0,cond);
      }
     }
  }
  thread_unlock(0);
}

/*Initiate some number of requester threads, and one service thread*/

int main(int argc, char* argv[]){
	max_requests = atoi(argv[1]);
	queue;
	live_requests = argc -3;
	cond = 2*live_requests;
	if (thread_libinit( (thread_startfunc_t) service, (char **) argv)) 
	  {
	    cout << "thread_libinit failed" << endl;
	    exit(1);
	  }
	
	cout<<"here"<<endl;
	for(int i=0; i<queue.size();i++)
	  cout<<get<0>(queue[i])<<" "<<get<1>(queue[i]) <<endl;
	queue.pop_back();
	cout<<queue.size()<<"size"<<endl;
	
	return 0;
}
