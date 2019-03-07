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
using namespace std;

typedef vector< tuple<ucontext_t*, int> > threadQCond;
typedef vector<ucontext_t *> threadQ;

static threadQ readyQueue;
static threadQCond wait;
static threadQ lock;
ucontext_t* running;
static vector<int> value;


void ending_output(){
  cout << "Thread library exiting.\n" << endl;
}

static void start(thread_startfunc_t func, void *arg){
  func(arg);
}

int thread_libinit(thread_startfunc_t func, void *arg)
//We create a new thread and give access from default thread to this new thread
// Whenever we thread init with a function(usually a service thread), and a number,
// We create this service thread and set it as running and we put it in the readyQueue
// At the end of this function, we loop infinitely
{
  
  ucontext_t * ucontext_ptr = new ucontext_t;
  ucontext_t * oucp = new ucontext_t;
  getcontext(ucontext_ptr);
  char *stack = new char [STACK_SIZE];
  ucontext_ptr->uc_stack.ss_sp = stack;
  ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
  ucontext_ptr->uc_stack.ss_flags =0;
  ucontext_ptr->uc_link = NULL;
  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  //maybe swap back to original context at the end of the program (after exit)
  swapcontext(oucp, ucontext_ptr);
  wait.push_back(ucontext_ptr);
  //readyQueue.push_back(ucontext_ptr);
  running = ucontext_ptr;
  
  while (!readyQueue.empty() && running !=NULL){
    1+1;
  }
  
  ending_output();
 
  exit(0);
  return 0;
}

int thread_create(thread_startfunc_t func, void*arg)
/*Whenever a thread is create, we create the context, and push it onto the running queue*/
{
  ucontext_t* ucontext_ptr = new ucontext_t;
  getcontext(ucontext_ptr);
  char *stack = new char [STACK_SIZE];
  ucontext_ptr->uc_stack.ss_sp = stack;
  ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
  ucontext_ptr->uc_stack.ss_flags = 0;
  ucontext_ptr->uc_link = NULL;
  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  // tuple<ucontext_t *, int> thread = make_tuple (ucontext_ptr, arg);
  // TODO: making the thread wait for calls, and only when we decide to signal
  wait.push_back(ucontext_ptr);
  // readyQueue.push_back(ucontext_ptr);
  return 0;
}

int thread_yield(void){
  /*Sets the running as the next item of the queue,as running, and then pushes the current running back into the queue, returns 0 on success and -1 on failure*/
  ucontext_t *temp = running;
  ucontext_t *next = readyQueue.front();
  swapcontext(temp, next);
  readyQueue.erase(readyQueue.front());
  readyQueue.push_back(temp);
  return 0;
}

int thread_lock(unsigned int lock){
  interrupt_disable();
  if (value.size()<=lock){
      value(lock,0);
  }
  if (value[lock] == 0)
  {
      value[lock] = 1;
  }else{
    lock.push_back(running);
    swapcontext(running, wait.front());
  }
  interrupt_enable();
}

int thread_unlock(unsigned int lock){
  interrupt_disable();
  value[lock] = 0; //0 is free
  if (!lock.empty()){
    ready.push_back(lock.front());
    lock.erase(lock.front());
  }
  interrupt_enable();
}
int thread_wait(unsigned int lock, unsigned int cond)
{

}
int thread_signal(unsigned int lock, unsigned int cond)
{

}
int thread_broadcast(unsigned int lock, unsigned int cond)
{

}

