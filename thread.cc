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
map<int, threadQCond> waitQueue; //a wait with int
threadQCond lockQueue; //this has a lock number with ucontext
static ucontext_t* running;
map<int, bool> lockBool; //a map to check if we have a lock on it
static ucontext_t* service;
static ucontext_t* previous;

void ending_output()
{
  cout << "Thread library exiting.\n" << endl;
}

static void start(thread_startfunc_t func, void *arg)
{
  interrupt_enable();
  func(arg);
  interrupt_disable();
  if(previous){
    //deleting past ucontext
    delete[] (char*)previous->uc_stack.ss_sp;
    delete previous;
  }

  //store thread as temp, set a new thread as running and then deallocate the temp variable
  if(!readyQueue.empty()){

    previous = running;
    ucontext_t* next = readyQueue.front();
    readyQueue.erase(readyQueue.begin());
    running = next;
    setcontext(next);
  }

  setcontext(service);

}

int thread_libinit(thread_startfunc_t func, void *arg)
//We create a new thread and give access from default thread to this new thread
// Whenever we thread init with a function(usually a service thread), and a number,
// We create this service thread and set it as running and we put it in the readyQueue
// At the end of this function, we loop infinitely
{
  interrupt_disable();
  ucontext_t* ucontext_ptr; 
  char *stack; 

  if(service){
    //Second libinit call
    return -1;
  }

  try{
    ucontext_ptr = new ucontext_t;
    getcontext(ucontext_ptr);
    stack = new char [STACK_SIZE];
    ucontext_ptr->uc_stack.ss_sp = stack;
    ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
    ucontext_ptr->uc_stack.ss_flags =0;
    ucontext_ptr->uc_link = NULL;
  }
  catch(bad_alloc){
    delete[] stack;
    delete ucontext_ptr;
    return -1;
  }

  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);

  running = ucontext_ptr;
  service = new ucontext_t;
  
  swapcontext(service, ucontext_ptr);

  ending_output();
  interrupt_enable();
  exit(0);
  return 0;
}

int thread_create(thread_startfunc_t func, void*arg)
/*Whenever a thread is create, we create the context, and push it onto the running queue*/
{
  interrupt_disable();
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
  readyQueue.push_back(ucontext_ptr);
  interrupt_enable();
  return 0;
}

int thread_yield(void){
  //cout << "thread lock.\n" << endl;

  interrupt_disable();
  /*Sets the running as the next item of the queue,as running, and then pushes the current running back into the queue, returns 0 on success and -1 on failure*/
  if (!readyQueue.empty()){
  ucontext_t *temp = running;
  ucontext_t *next = readyQueue.front();
  running = next;
  readyQueue.erase(readyQueue.begin());
  readyQueue.push_back(temp);
  swapcontext(temp, next);
  }
  interrupt_enable();
  return 0;
}

int thread_lock(unsigned int lock){
  //In this functions, we turn on a lock for a specific function and then ensure that it cannot be accessed
  
  interrupt_disable();
  cout << "Trying to lock: " <<lock <<".\n" << endl;
  if (lockBool.count(lock) == 0){
    //if lock is not initiated, we create this lock
    //cout << "adding lock " << lock <<".\n" << endl;
    lockBool.insert(pair<int,bool>(lock, false));
  }
  
  if (lockBool[lock] == false)
  {
      //Lock the thread, and swap context sinve the thread is locked
      lockBool[lock] = true;  
      
  }else{
    //pushing this thread back, onto the ready queue if lock is called
    lockQueue.push_back(make_tuple(running,lock));
    ucontext_t *temp = running;
    ucontext_t *next = readyQueue.front();
    swapcontext(temp, next);
  }
  
  interrupt_enable();
}

int thread_unlock(unsigned int lock){
  //We unlock a function, and then we can access the stuff since this can be put back onto
  //a ready queue
  
  interrupt_disable();
    
  lockBool[lock] = false;
  if (!lockQueue.empty()){
    //cout << "Trying to unlock: " << lock << ".\n" << endl;
    for (int i = 0; i<lockQueue.size();i++){
      if (get<1>(lockQueue[i]) == lock){
      	  cout << "unlocked: " << lock << ".\n" << endl;	
          readyQueue.push_back(get<0>(lockQueue[i]));
	        //remove the lock from the queue of threads waiting for lock
	        lockQueue.erase(lockQueue.begin()+i);
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
  //need to swap running properly
  //1 is not properly being stored
  
  if (!waitQueue.count(lock)){
    
    waitQueue.insert(pair<int,threadQCond>(lock, threadQCond {make_tuple(running,cond)}));
    
  } else {
    
    waitQueue[lock].push_back(make_tuple(running,cond));
    
  }

  ucontext_t* temp = running;
  running = readyQueue.front();
  readyQueue.erase(readyQueue.begin());
  swapcontext(temp,readyQueue.front());
  interrupt_enable();
}


int thread_signal(unsigned int lock, unsigned int cond)
{
  interrupt_disable();
  //We look inside the wait queue and search for the thread_signal
  
  for(int i = 0; i<waitQueue[lock].size();i++){
    
    if (get<1>(waitQueue[lock][i]) == cond){
      
      cout << "Found lock in cond.\n" << endl;
      
      readyQueue.push_back(get<0>(waitQueue[lock][i]));
      //ucontext_t* temp = get<0>(waitQueue[lock][i]);
      //ucontext_t* current = running;
      //running = temp;
      //readyQueue.push_back(temp);
      waitQueue[lock].erase(waitQueue[lock].begin()+i);
      //cout << "The Readyq: " << readyQueue << endl;
      //swapcontext(running,temp);
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
