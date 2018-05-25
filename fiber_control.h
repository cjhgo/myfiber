#ifndef MYFIBER_FIBER_CONTROL
#define MYFIBER_FIBER_CONTROL
#include<functional>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<queue>
#include<pthread_tools.h>

struct Fiber
{
	std::function<void()> fn;
	ucontext_t context;
};
struct Tls
{
	Fiber* cur;
	Fiber* pre;
	Fiber* next;
	ucontext_t base;

};
class Fiber_Control
{
public:
	//static Fiber_Control singleton;
	static Fiber_Control& get_instance();
	void init();
	void launch(std::function<void()> fn);
	Fiber_Control();
	void join();
private:	
	Thread thread;
	Mutex mutex;
	Conditional cond;
	bool stop;
	std::queue<Fiber> workerqueue;
	Fiber* get_fiber();
	Tls tls;
};

#endif