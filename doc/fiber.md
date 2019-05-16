


### 基于ucontext库函数实现一个Fiber

fiber(纤程),轻量级的线程执行,和协程指的几乎是同一个概念,为我们提供了在一个系统线程内并发地执行多个函数的能力.
系统线程提供的是抢占式多任务,用户纤程提供的是合作式多任务,这里的合作主要体现在程序会主动让出cpu资源暂停执行.
使用ucontext库函数,我们可以手动的进行上下文切换,这样我们既可以让一个函数暂停执行,也可以让一个函数恢复执行.
在系统线程中,内核会负责线程调度,具体的机制可以是轮转的,也可以是基于优先级的,总之一个线程被调度出去后,另一个线程调度进来.
在纤程中,由于是用户级别的线程,则需要有用户显式的实现纤程调度的功能,并且这种调度可以参考内核是如何调度的.
所以说,实现一个纤程除了使用ucontext来操作上下文之外,我们还要在整体上调度程序中的多个纤程.
这种调度主要包括一个纤程yield之后接下来运行哪个纤程,以及这个让权的纤程何时会被调度回来.


在本项目中,我们对纤程使用了FIFO调度,也就是
程序会按照launch的顺序挨个执行传入的函数,当前函数yield之后则执行下一个,当前函数则被reschedule到队列.
仿照系统内核的线程控制块的概念,我们也定义一个结构体`Fiber`来代表每个线程实例.
在数据结构上,通过队列来存放所有的线程实例.


本项目是对powergraph中的fiber的简化实现,设计思路如下.


Fiber_group是一个fiber管理器
Fiber_control实现了fiber的存放和调度



### Fiber_group
```c
void launch(std::function<void()> fn);
void join();
```
### Fiber_Control

数据结构
```c
struct Fiber
{
	Fiber_Control* parent;
	std::function<void()> fn;
	ucontext_t context;
	bool terminate;
	bool inited;
};
struct Tls
{
	Fiber_Control* parent;
	Fiber* cur;
	Fiber* pre;
	Fiber* garbage;
	ucontext_t base;

};
std::queue<Fiber> workerqueue;
```
```
static Fiber_Control& get_instance();
```


### call flow
```c
Fiber_group.launch(foo)
  Fiber_Control::get_instance().launch(fnn);
    F
```
