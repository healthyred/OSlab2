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

static vector<ucontext_t *> readyQueue;
static vector<tuple<ucontext_t *, int>> wait;
static vector<tuple<ucontext_t *, int>> lock;
ucontext_t* running;

void ending_output(){
  cout << "Thread library exiting." << endl;
}

static void start(thread_startfunc_t func, void *arg){
  func(arg);
}

int thread_libinit(thread_startfunc_t func, void * arg)
//We create a new thread and give access from default thread to this new thread
{
  
  ucontext_t * ucontext_ptr;
  getcontext(ucontext_ptr);
  char *stack = new char [STACK_SIZE];
  ucontext_ptr->uc_stack.ss_sp = stack;
  ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
  ucontext_ptr->uc_stack.ss_flags =0;
  ucontext_ptr->uc_link = NULL;
  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  swapcontext(NULL, ucontext_ptr);
  
  //running = ucontext_prt;

}

int thread_create(thread_startfunc_t func, void*arg)
  
{
  ucontext_t *ucontext_ptr;
  getcontext(ucontext_ptr);
  char *stack = new char [STACK_SIZE];
  ucontext_ptr->uc_stack.ss_sp = stack;
  ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
  ucontext_ptr->uc_stack.ss_flags = 0;
  ucontext_ptr->uc_link = NULL;
  
  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  
  //TODO: making the thread wait for calls, and only when we decide to signal
  readyQueue.push_back(ucontext_ptr);
}

int thread_yield(void){

}

int thread_lock(unsigned int lock){}
int thread_unlock(unsigned int lock){}
int thread_wait(unsigned int lock, unsigned int cond){}
int thread_signal(unsigned int lock, unsigned int cond){}
int thread_broadcast(unsigned int lock, unsigned int cond){}

