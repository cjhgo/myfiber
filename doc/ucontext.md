

### 使用ucontext来手动切换上下文
```c
#include <ucontext.h>
int getcontext(ucontext_t *ucp);
int setcontext(const ucontext_t *ucp);
void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
```

`ucontext.h`头文件定义了2个结构体和4个函数来实现用户层面的进程内多个线程之间的上下文切换能力.

#### 上下文的表示
`ucontext_t`结构体代表一个线程的上下文,定义如下:
```c
typedef struct ucontext {
    struct ucontext *uc_link;
    sigset_t         uc_sigmask;
    stack_t          uc_stack;
    mcontext_t       uc_mcontext;
    ...
} ucontext_t;
```

+ `uc_link`指向一个当前上下文结束之后继续执行的上下文
+ `sigset_t/stack_t`定义在`<signal.h>`头文件中.
`sigset_t`代表了当前上下文中的信号
`stack_t`代表了当前上下文的栈
+ `uc_context`是一个`mcontext_t`类型的字段,
`mcontext_t`是一个依赖于具体的机器类型的结构体,代表了具体保存的上下文,包括当前上下文的寄存器的状态


#### 上下文切换,get/set上下文
`getcontext`函数把当前的上下文保存到ucp中,即用当前的上下文来初始化ucp
`setcontext`函数从ucp中恢复上下文,这个函数调用之后会接着执行ucp参数指向的上下文,**因此这个函数不会返回**

一个用于setcontext的ucp有三种

1. 通过`getcontext`来初始化的
这种情况下,set ucp之后,程序会沿着原来get ucp的地方继续执行
2. 通过`makecontext`来构造的
这种情况下,set ucp之后程序会调用makecontext的时候传的`func`函数,然后接着执行ucp中的uc_link,
如果uc_link为空,则执行完func之后线程退出.
3. 作为一个参数传给了一个signal handler之后被


使用`setjmp(3)/longjmp(3)`机制也可以实现手动的上下文切换

#### 上下文

