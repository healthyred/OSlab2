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
#include <algorithm> 
using namespace std;

typedef vector<tuple<ucontext_t*, int>> threadQCond;
typedef vector<ucontext_t *> threadQ;

threadQ readyQueue;
map<int, threadQCond> waitQueue; //a wait with int
threadQCond lockQueue; //this has a lock number with ucontext
ucontext_t* running;
map<int, ucontext_t*> lockBool; //a map to check if we have a lock on it
ucontext_t* service;
static ucontext_t* previous;

void ending_output()
{
  cout << "Thread library exiting." << endl;
}

static void start(thread_startfunc_t func, void *arg)
{
  //cout << "enable 1\n" << endl;

  interrupt_enable();
  func(arg);

  //cout << "disable 1\n" << endl;

  interrupt_disable();
  //cout << "thread finished" << endl;
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
    //  cout << "enable2\n" << endl;
  }
  setcontext(service);
  interrupt_enable();
}

int thread_libinit(thread_startfunc_t func, void *arg)
//We create a new thread and give access from default thread to this new thread
// Whenever we thread init with a function(usually a service thread), and a number,
// We create this service thread and set it as running and we put it in the readyQueue
// At the end of this function, we loop infinitely
{
    //cout << "disable 2\n" << endl;

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
    //interrupt_enable();
    return -1;
  }

  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);

  running = ucontext_ptr;
  service = new ucontext_t;
  //interrupt_enable();

  swapcontext(service, ucontext_ptr);

  ending_output();
  delete service;
  interrupt_enable();
  exit(0);
  return 0;
}

int thread_create(thread_startfunc_t func, void*arg)
/*Whenever a thread is create, we create the context, and push it onto the running queue*/
{

  if(!service){
    //Has not called libinit
    return -1;
  }

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

    interrupt_enable();
    return -1;
  }

  makecontext(ucontext_ptr, (void (*)()) start, 2, func, arg);
  readyQueue.push_back(ucontext_ptr);

  interrupt_enable();
  return 0;
}

int thread_yield(void){
  //cout<< "thread lock.\n" << endl;
  if(!service){
    //Has not called libinit
    return -1;
  }

  interrupt_disable();
  /*Sets the running as the next item of the queue,as running, and then pushes the current running back into the queue, re  turns 0 on success and -1 on failure*/
  if (!readyQueue.empty()){
  ucontext_t *temp = running;
  ucontext_t *next = readyQueue.front();
  running = next;
  readyQueue.erase(readyQueue.begin());
  readyQueue.push_back(temp);
  //interrupt_enable();
  swapcontext(temp, next);
  }

      interrupt_enable();
  

  return 0;
}

int thread_lockhelper(unsigned int lock){
    if(!service){
    //Has not called libinit
    return -1;
  }
  
  if (lockBool.count(lock) == 0){
      //if lock is not initiated, we create this lock
      
      
      try{
        temp = new pair<int,bool>* temp<int,bool>(lock,running);
      }
  
      catch(bad_alloc){
        delete temp;
        return -1;
      }
      lockBool.push_back(&temp);


    }
    

    if (lockBool[lock] == nullptr)
    {
        //Lock the thread, and swap context sinve the thread is locked
        lockBool[lock] = running;  
    }else{
      //add thread to lockqueue, and then switch to next ready thread
      if(!readyQueue.empty()){
      lockQueue.push_back(make_tuple(running,lock));
      ucontext_t *temp = running;
      ucontext_t *next = readyQueue.front();
      running = next;
      readyQueue.erase(readyQueue.begin());
      swapcontext(temp, next);
      } 
    }
  return 0;
}

int thread_lock(unsigned int lock){
  //In this functions, we turn on a lock for a specific function and then ensure that it cannot be accessed
  //cout << "ReadyQ Size " << readyQueue.size() << "\n" << endl;
  interrupt_disable();
    if(!service){
    //Has not called libinit
    return -1;
  }
  if (thread_lockhelper(lock)!=0){
    interrupt_enable();
    return -1;
  }

  interrupt_enable();
  return 0;
}

int thread_unlockHelper(unsigned int lock){
    if(!service){
    //Has not called libinit
    return -1;
    }
    lockBool[lock] = nullptr;
    if (!lockQueue.empty()){
      for (int i = 0; i<lockQueue.size();i++){
        if (get<1>(lockQueue[i]) == lock){
            ucontext_t* temp = get<0>(lockQueue[i]);
            readyQueue.push_back(temp);
            lockQueue.erase(lockQueue.begin() + i);
            lockBool[lock]=running;
            break;
        }
      }
    }
  return 0;
}

int thread_unlock(unsigned int lock){
  //We unlock a function, and then we can access the stuff since this can be put back onto
  //a ready queue 

  interrupt_disable();  
  if(!service){
    //Has not called libinit
    return -1;
  }
  if (thread_unlockHelper(lock)!=0){
    //calling unlock without a lock
    interrupt_enable();
    return -1;
  }

  interrupt_enable();
  return 0;
}


int thread_wait(unsigned int lock, unsigned int cond)
{
  //need to swap running properly
  //1 is not properly being stored

  interrupt_disable();
  if(!service){
    //Has not called libinit
    return -1;
  }

  if(thread_unlockHelper(lock) != 0){
    return -1;
  };

  //we pushing running into wait
  if (lockBool[lock]==running){
    interrupt_enable();
    return -1;
  }
  if (!waitQueue.count(lock)){
    waitQueue.insert(pair<int,threadQCond>(lock, threadQCond {make_tuple(running,cond)}));
  }  else {
    waitQueue[lock].push_back(make_tuple(running,cond));
  }  
 // cout<<"enable wait\n"<<endl;
  if(!readyQueue.empty()){
    ucontext_t* temp = running;
    running = readyQueue.front();
    readyQueue.erase(readyQueue.begin());
    swapcontext(temp, running);
  }else{
    //error handling for just one running thread
    interrupt_enable();
    return -1;
  }
  thread_lockhelper(lock); 

  interrupt_enable();
  return 0;
}


int thread_signal(unsigned int lock, unsigned int cond)
{

  interrupt_disable();
    if(!service){
    //Has not called libinit
    return -1;
  }
  //We look inside the wait queue and search check for lock and cond, then we add 
  //onto the end of the ready queue
  //figure out when these fail

  for(int i = 0; i < waitQueue[lock].size();i++){
    
    if (get<1>(waitQueue[lock][i]) == cond){
      ucontext_t* temp = get<0>(waitQueue[lock][i]);
      waitQueue[lock].erase(waitQueue[lock].begin()+i);
      readyQueue.push_back(temp);
      interrupt_enable();
      return 0;
    }
  }

  interrupt_enable();
  return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond)
{
  //Figure out when fail

  interrupt_disable();
  if(!service){
    //Has not called libinit
    return -1;
  }
  vector<int> to_remove;

  for(int i = 0; i < waitQueue[lock].size();i++){
    if (get<1>(waitQueue[lock][i]) ==cond){
      ucontext_t* temp = get<0>(waitQueue[lock][i]);
      readyQueue.push_back(temp);
      to_remove.push_back(i);
    }
  }

  reverse(to_remove.begin(),to_remove.end());

  for(int i = 0; i < to_remove.size();i++){
    waitQueue[lock].erase(waitQueue[lock].begin()+to_remove[i]);
  }

  interrupt_enable();
  return 0;
}
