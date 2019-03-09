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
#include <climits>
using namespace std;

typedef tuple<int, int> SSTF;

int max_requests;
int live_requests;
int cond;
vector<SSTF> queue(0);
int start = 10000;
int previous =0;
void request_output(int requester, int track){
  cout << "requester " << track << " track " << requester << endl;
}

void service_output(int requester, int track){
  cout << "service requester " << track << " track " << requester << endl;
}

void request(char *a) {		
  string file(a);
   int fileNum = file.back()-'0';
   // cout<< "fileNum"<< fileNum<<endl;
   ifstream stream;
   stream.open(file);
   if (stream.is_open()){
     string line;
     int i = 0;
   // cout << "request\n" << endl;

     thread_lock(0);
  //   cout << "1\n" << endl;
     while(getline(stream, line)){
       int intLine = atoi(line.c_str());
       auto x = SSTF(intLine,fileNum);
       //cout<<fileNum<<endl;
       // cout<<"queue size"<< queue.size()<<endl;
       
       if  ((queue.size() == live_requests && live_requests <= max_requests)||(queue.size()==max_requests&&live_requests>max_requests)){
	 //cout<<"signal"<<endl;
	 thread_signal(0,cond);
 //  cout << "2\n" << endl;

	 thread_wait(0,start);
  // cout << "3\n" << endl;

       }
    //  cout << "4\n" << endl;

       if((queue.size() <max_requests && max_requests <live_requests)|| (queue.size() <live_requests && max_requests >=live_requests))
	 {
     	//   cout << "5\n" << endl;

	   queue.push_back(x);
	   request_output(intLine, fileNum);	   
	   thread_signal(0,cond);

	 }
       thread_wait(0,fileNum);
     }
   }
   //cout<<"decrement live requests"<<endl;
   --live_requests;
   thread_signal(0,cond);
   thread_unlock(0);
   
}

int sortSSFT(vector<tuple<int,int>> a, int previous){
  int dif = INT_MAX;
  int index = -1;
  for (int i = 0; i < a.size(); i++){
    if(abs(get<0>(a[i])-previous)<dif){
      dif = abs(get<0>(a[i])-previous);
      index = i;
    }
  }

  return index;
}

void service(char **a)
{
  start_preemptions(false, true,0);
  int i = 2;
  char ** files = (char**) a;
  while (files[i] != '\0'){
    if (thread_create((thread_startfunc_t) request, (char *) files[i])) {
      cout << "thread_create failed" << endl;
      exit(1);
    }
    ++i;
  }
  thread_lock(0);
  while(live_requests != 0){
   // cout << "before wait in service\n" << endl;

    thread_wait(0,cond);
  //  cout << "after wait in service\n" << endl;

    if(live_requests == 0){break;}
    //cout<<"live_request"<<live_requests<<"max_request"<<max_requests<< "queue size" << queue.size() <<endl;
    if ((queue.size() == live_requests && live_requests <= max_requests)||(queue.size()==max_requests && live_requests>max_requests)){
      int i = sortSSFT(queue, previous);
      previous = get<0>(queue[i]);
      int x = get<1>(queue[i]);
      service_output(get<0>(queue[i]),get<1>(queue[i]));
      queue.erase(queue.begin()+i);
       //     cout<<"error1"<<endl;

      thread_signal(0, x);
    }else{
     // cout<<"error2"<<endl;
      thread_signal(0,start);
    }
  }
  thread_unlock(0);
}

/*Initiate some number of requester threads, and one service thread*/

int main(int argc, char* argv[]){
   max_requests = atoi(argv[1]);
	queue;
	live_requests = argc -2;
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
