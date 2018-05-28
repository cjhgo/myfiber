#include<iostream>
#include<vector>
#include <time.h>
#include<unistd.h>
#include<ucontext.h>
#define STACK_SIZE 16384  
struct Fiber
{
	ucontext_t ucon;
};
void foo()
{
	std::cout<<"i am foo\n";
}
struct TLS
{
	Fiber* cur;
	Fiber* pre;
	ucontext_t base;
};


void init_context(ucontext_t * uc)
{
	getcontext(uc);
	uc->uc_link=0;	  
  	uc->uc_stack.ss_sp = (char *) malloc(STACK_SIZE);
    uc->uc_stack.ss_size = STACK_SIZE;
    uc->uc_stack.ss_flags = 0;
}

std::vector<Fiber> fibv={Fiber(),Fiber(),Fiber(),Fiber(),Fiber()};

struct trampoline_args {
  void(*fn)(void) =foo;
};
void trampoline(void* arg)
{
  trampoline_args* args = reinterpret_cast<trampoline_args*>(arg);
  try {
    args->fn();
  } catch (...) {
  }
}

Fiber* get_next_fiber(int i)
{
	Fiber* f = new Fiber;
	if (i < fibv.size())
	{
		f->ucon = fibv[i].ucon;
		return f;
	}
	else
	{
		return NULL;
	}
}


void yield_to(Fiber* f)
{
	if (f != NULL)
	{
		trampoline_args* args = new trampoline_args;	
		init_context(&(f->ucon));
		ucontext_t here;
		f->ucon.uc_link=&here;
		makecontext(&(f->ucon), (void (*)())trampoline, 1, args);
		swapcontext(&here, &(f->ucon));
	}
	else
	{
		return;
	}	
}
void init()
{
	for(int i = 0; i <=5; i++)
	{
		Fiber* f = get_next_fiber(i);		
		yield_to(f);
		usleep(1500);
	}

}
TLS tls;
int main(int argc, char const *argv[])
{
	init();
	return 0;
}