#include<iostream>
#include<pthread_tools.h>
#include<fiber_control.h>
#include<fiber_group.h>
void foo()
{
	std::cout<<"i am fun\n";
}
void bar()
{
	std::cout<<"i am foo\n";
}
struct hhh
{
	std::function<void()> fn;
};


int main(int argc, char const *argv[])
{
	Fiber_group g;
	auto fuck = std::bind(foo);
	hhh h;
	h.fn=fuck;
	std::queue<hhh> wq;
	wq.push(h);
	std::cout<<wq.size();	
	wq.push(h);
	std::cout<<wq.size();	
	wq.push(h);
	std::cout<<wq.size();	
	std::cout<<std::endl;
	g.launch(fuck);
	g.launch(bar);
	g.launch(fuck);
	g.launch(fuck);
	g.launch(fuck);
	g.join();
	return 0;
}