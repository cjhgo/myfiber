#include<fiber_control.h>
#include<iostream>
void init_context(ucontext_t * uc)
{
	getcontext(uc);
	uc->uc_link=0;	  
  	uc->uc_stack.ss_sp = (char *) malloc(STACK_SIZE);
    uc->uc_stack.ss_size = STACK_SIZE;
    uc->uc_stack.ss_flags = 0;
}
struct trampoline_args {
  std::function<void(void)> fn;
};
Fiber_Control::Fiber_Control()
:stop(false)
{	
	tls.parent = this;
	tls.cur = NULL;
	tls.pre = NULL;
	tls.garbage = NULL;
	ucontext_t * cp = &tls.base;
    init_context(cp);
	auto thread_init = std::bind(&Fiber_Control::init, this);
	thread.start(thread_init);
}
void Fiber_Control::init()
{	
	mutex.lock();
	while(not stop)
	{
		Fiber* f = get_fiber();		
		if (f != NULL)
		{
			yield_to(f);
		}
		else
		{
			cond.wait(mutex);	
		}		
	}
	mutex.unlock();
}
Fiber_Control& Fiber_Control::get_instance()
{
	static Fiber_Control singleton;
	return singleton;
}
void Fiber_Control::launch(std::function<void()> fn)
{
	Fiber* f = new Fiber;
	f->parent=this;
	f->fn=fn;
	mutex.lock();
	workerqueue.push(*f);
	cond.signal();
	mutex.unlock();
}
Fiber* Fiber_Control::get_fiber()
{
	if ( workerqueue.empty() )
	{
		return NULL;
	}
	else
	{
		Fiber f = workerqueue.front();
		workerqueue.pop();		
		Fiber* fp=new Fiber;
		fp->fn=f.fn;
		return  fp;
	}
}

void Fiber_Control::join()
{
	thread.join();
}
void Fiber_Control::yield_to(Fiber* next_fib)
{
	if ( next_fib != NULL )
	{

		tls.pre = tls.cur;
		tls.cur = next_fib;
		if( tls.pre != NULL )
		{			
			std::cout<<"temp ...\n";
		}
		else
		{
			
			trampoline_args* args = new trampoline_args;	
			init_context(&(tls.cur->context));
			ucontext_t here;
			tls.cur->context.uc_link=&here;
			makecontext(&(tls.cur->context), (void (*)())trampoline, 1, args);
			swapcontext(&here, &(tls.cur->context));
		}
	}
	else
	{
		return;
	}
	if( tls.pre )
		reschedule_fiber(tls.pre);
}


// the trampoline to call the user function. This function never returns
void Fiber_Control::trampoline(void* _args) {
//void trampoline(int _args) {
  // we may have launched to here by switching in from another fiber.
  // we will need to clean up the previous fiber
  
  // if (tls.prev_fiber) t->parent->reschedule_fiber(t->workerid, t->prev_fiber);
  // t->prev_fiber = NULL;
  std::cout<<" i am trampoline\n";
  trampoline_args* args = reinterpret_cast<trampoline_args*>(_args);
  try {
    args->fn();
  } catch (...) {
  }
  delete args;
  //fiber_control::exit();
}

void Fiber_Control::reschedule_fiber(Fiber* f)
{
	mutex.lock();
	workerqueue.push(*f);
	cond.signal();
	mutex.unlock();	
}