#include<fiber_control.h>
#include<iostream>

Fiber_Control::Fiber_Control()
:stop(false)
{
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
			f->fn();
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
	Fiber f;
	f.fn = fn;
	mutex.lock();
	workerqueue.push(f);
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