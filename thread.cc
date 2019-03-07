#include <ucontext.h>
#include <queue>
#include <vector>
#include "thread.h"
#include "interrupt.h"
#include <tuple>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <map>
#include <new>
using namespace std;

typedef vector<tuple<ucontext_t*, int>> threadQCond;
typedef vector<ucontext_t *> threadQ;

 threadQ readyQueue;
 map<int, threadQCond> waitQueue;
 threadQCond lockQueue;
ucontext_t* running;
 map<int, bool> lockBool;

void ending_output()
{
  cout << "Thread library exiting.\n" << endl;
}

static void start(thread_startfunc_t func, void *arg)
{
  func(arg);
  //store thread as temp, set a new thread as running and then deallocate the temp variable
  //Deallocate memory of the currently running thread
  if(!readyQueue.empty()){
    ucontext_t* temp = running;
    ucontext_t* next = readyQueue.front();
    swapcontext(temp, next);
    readyQueue.erase(readyQueue.begin());
    delete[] (char*)temp->uc_stack.ss_sp;
  }

}

int thread_libinit(thread_startfunc_t func, void *arg)
//We create a new thread and give access from default thread to this new thread
// Whenever we thread init with a function(usually a service thread), and a number,
// We create this service thread and set it as running and we put it in the readyQueue
// At the end of this function, we loop infinitely
{
  ucontext_t* ucontext_ptr; 
  char *stack; 
  ucontext_t* oucp;
  
  try{
  ucontext_ptr = new ucontext_t;
  oucp = new ucontext_t;
  getcontext(ucontext_ptr);
  stack = new char [STACK_SIZE];
  ucontext_ptr->uc_stack.ss_sp = stack;
  ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
  ucontext_ptr->uc_stack.ss_flags =0;
  ucontext_ptr->uc_link = NULL;
  }
  catch(bad_alloc){
    delete[] stack;
    delete oucp;
    delete ucontext_ptr;
    return -1;
  }

  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  swapcontext(oucp, ucontext_ptr);
  running = ucontext_ptr;
  
  while (!readyQueue.empty() &&running !=NULL)
  {
    1+1;
  }

  //Try catch to deallocate if we are in deadlock
  while(!waitQueue.empty()){
    waitQueue.begin() -> first;
  } 

  ending_output(); 
  exit(0);
  return 0;
}

int thread_create(thread_startfunc_t func, void*arg)
/*Whenever a thread is create, we create the context, and push it onto the running queue*/
{
  ucontext_t* ucontext_ptr; 
  char *stack; 
  try 
  {
    ucontext_ptr = new ucontext_t;
    getcontext(ucontext_ptr);
    stack = new char [STACK_SIZE];
    ucontext_ptr->uc_stack.ss_sp = stack;
    ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
    ucontext_ptr->uc_stack.ss_flags = 0;
    ucontext_ptr->uc_link = NULL;
  }
  catch(bad_alloc){
    delete[] stack;
    delete ucontext_ptr;
    return -1;
  }

  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  // tuple<ucontext_t *, int> thread = make_tuple (ucontext_ptr, arg);
  // TODO: making the thread wait for calls, and only when we decide to signal
  // readyQueue.push_back(ucontext_ptr);
  return 0;
}

int thread_yield(void){
  /*Sets the running as the next item of the queue,as running, and then pushes the current running back into the queue, returns 0 on success and -1 on failure*/
  if (!readyQueue.empty()){
  ucontext_t *temp = running;
  ucontext_t *next = readyQueue.front();
  swapcontext(temp, next);
  readyQueue.erase(readyQueue.begin());
  readyQueue.push_back(temp);
  }
  return 0;
}

int thread_lock(unsigned int lock){
  interrupt_disable();
  if (!lockBool.count(lock)){
      lockBool.insert(pair<int,bool>(lock, false));
}
  if (lockBool[lock] ==false)
  {
      lockBool[lock] = true;
  }else{
    lockQueue.push_back(make_tuple(running,lock));
    swapcontext(running, readyQueue.front());
  }
  interrupt_enable();
}

int thread_unlock(unsigned int lock){
  interrupt_disable();
  lockBool[lock] = false;
  if (!lockQueue.empty()){
    for (int i = 0; i<lockQueue.size();i++){
      if (get<1>(lockQueue[i])== lock){
          readyQueue.push_back(get<0>(lockQueue[i]));
          break;
          }
    }
    lockBool[lock]=true;
  }
  interrupt_enable();
}
int thread_wait(unsigned int lock, unsigned int cond)
{
interrupt_disable();
 if (!waitQueue.count(lock)){
   waitQueue.insert(pair<int,threadQCond>(lock,   threadQCond {make_tuple(running,cond)}));
 } else {
   waitQueue[lock].push_back(make_tuple(running,cond));
 }
 swapcontext(running,readyQueue.front());
 running = readyQueue.front();
 readyQueue.erase(readyQueue.begin());
interrupt_enable();
}


int thread_signal(unsigned int lock, unsigned int cond)
{
interrupt_disable();
 for(int i = 0; i<waitQueue[lock].size();i++){
  if (get<1>(waitQueue[lock][i]) ==cond){
    readyQueue.push_back(get<0>(waitQueue[lock][i]));
    waitQueue[lock].erase(waitQueue[lock].begin()+i);
    break;
  }
}
 interrupt_enable();
}

int thread_broadcast(unsigned int lock, unsigned int cond)
{
interrupt_disable();
for(int i = 0; i<waitQueue[lock].size();i++){
  if (get<1>(waitQueue[lock][i]) ==cond){
    readyQueue.push_back(get<0>(waitQueue[lock][i]));
    waitQueue[lock].erase(waitQueue[lock].begin()+i);
  }
}
interrupt_enable();
}
