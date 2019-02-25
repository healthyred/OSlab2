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

/*Initiate some number of requester threads, and one service thread*/
int max_requests;
int requesters;
int service = 1;

int main(){
  
  return 0;
}
