#include<iostream>
#include<pthread_tools.h>
#include<fiber_control.h>
#include<fiber_group.h>
void foo()
{
	std::cout<<"i am foo\n";
}
void bar()
{
	std::cout<<"i am bar1\n";
	Fiber_Control::yield();
	std::cout<<"i am bar2!!!!!\n";
}
struct hhh
{
	std::function<void()> fn;
};


int main(int argc, char const *argv[])
{
	Fiber_group g;
	g.launch(foo);
	g.launch(bar);
	g.launch(foo);
	g.launch(bar);
	g.join();
	return 0;
}