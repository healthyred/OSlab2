#include <ucontext.h>
#include <queue>
#include <vector>
#include "thread.h"
#include "interrupt.h"
#include <tuple>
using namespace std;

static vector<ucontext_t *> readyQueue;
static vector<tuple<ucontext_t *, int>> wait;
//lock queue
static ucontext_t* running;

int thread_libinit(thread_startfunc_t func, void*arg)
{
  ucontext_t * ucontext_ptr = new ucontext_t *;
  getcontext(ucontext_ptr);
  char *stack = new char [STACK_SIZE];
  ucontext_ptr->uc_stack.ss_sp = stack;
  ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
  ucontext_ptr->uc_stack.ss_flags =0;
  ucontext_ptr->uc_link = NULL;
  makecontext(ucontext_ptr, (void (*)()) start, 2, arg1,arg2);
}

int thread_create(thread_startfunc_t func, void*arg)
{

}

int thread_yield(void){

}

int thread_lock(unsigned int lock){}
int thread_unlock(unsigned int lock){}
int thread_wait(unsigned int lock, unsigned int cond){}
int thread_signal(unsigned int lock, unsigned int cond){}
int thread_broadcast(unsigned int lock, unsigned int cond){}

