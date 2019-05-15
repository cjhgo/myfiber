

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
    - `sigset_t`代表了当前上下文中的信号
    - `stack_t`代表了当前上下文的栈,指向了一段内存区域,其定义如下
        ```c
        typedef struct {
        void  *ss_sp;     /* Base address of stack */表示栈区域的起始位置
        int    ss_flags;  /* Flags */
        size_t ss_size;   /* Number of bytes in stack */表示栈区域的size
        } stack_t;
        ```
+ `uc_context`是一个`mcontext_t`类型的字段,
`mcontext_t`是一个依赖于具体的机器类型的结构体,代表了具体保存的上下文,包括当前上下文的寄存器的状态


#### 上下文切换,get/set上下文
`int getcontext(ucontext_t *ucp);`
`getcontext`函数把当前的上下文保存到ucp中,即用当前的上下文来初始化ucp
`int setcontext(const ucontext_t *ucp);`
`setcontext`函数从ucp中恢复上下文,这个函数调用之后会接着执行ucp参数指向的上下文,**因此这个函数不会返回**

一个用于setcontext的ucp有三种

1. 通过`getcontext`来初始化的
这种情况下,set ucp之后,程序会沿着原来get ucp的地方继续执行
2. 通过`makecontext`来构造的
这种情况下,set ucp之后程序会调用makecontext的时候传的`func`函数,然后接着执行ucp中的uc_link,
如果uc_link为空,则执行完func之后线程退出.
3. 作为一个参数传给了一个signal handler之后被


使用`setjmp(3)/longjmp(3)`机制也可以实现手动的上下文切换

#### 修改ucp
`void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);`
通过getcontext来初始化一个ucp之后可以用`makecontext`这个函数来修改ucp
修改之后当ucp激活的时候会执行func函数,**func代表一个返回void接收未指定数量参数的函数指针**,argc表示这个函数有多少参数,`...`是具体的整数参数
修改ucp之前要先设置ucp->uc_stack和uc_link
可以通过以下方式设置uc_stack
```c
char func1_stack[16384];
uctx_func1.uc_stack.ss_sp = func1_stack;
uctx_func1.uc_stack.ss_size = sizeof(func1_stack);
```

`int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);`
swapcontext把当前的上下文保存到oucp中,然后激活ucp指向的上下文.
和setcontext类似,swapcontext函数调用之后不返回.
但是
如果ucp->uc_link指向了oucp,那么ucp指向的上下文结束之后就会回到swapcontext调用的地方,
或者是后来通过set或swap激活了oucp,程序也会回到swapcontext调用的地方,
看起来像是swapcontext调用结束了一样.

#### 关于makecontext传函数指针的说明
c语言的最新标准已经不支持`在函数原型的参数中用`()`来表明函数可以接收任意数量的参数`这一特性.
因此ucontext这一族函数已经被指定为obsolescent
本来makecontext中的func应该是一个 指向任意类型的函数,但c语言标准不能表达这一点.
(或者像pthread_create中的(void*(void*))这样也行).
makecontext中只能用`()`这个语法特性来实现任意数量的整数参数(如果是任意数量的任意类型的参数,就无法事先计算需要分配多少栈来接收参数).
现在c语言连`()`这个语法都不支持了,因此ucontext这个功能不太被鼓励使用,但是也没有很好的替代品.
大多人是用ucontext来事先用户级别的线程调度,也就是协程.
这两个地方讨论了这个问题.
http://pubs.opengroup.org/onlinepubs/009695399/functions/makecontext.html
https://stackoverflow.com/questions/42894552/why-makecontexts-func-only-accepts-integer-arguments

在现有的条件下,makecontext只支持传入一个`void(*)()`类型的函数,然后后边跟argc个整数,
也就是`void fun(int0,int1)`这样一个函数,这个函数的表达能力是很有限的,显然不符合一个协程库的需求.
通过引入`Trampoline`这一概念,我们可以绕过这一限制,具体机制如下:
```c
void trampoline(void* _args);
struct trampoline_args {
  std::function<void(void)> fn;
};    
```
这个机制主要用到了如下原理,
`在64位机器上,void*可以转型为一个uint_64,因此可以通过向func传入uint_64来传入一个void*`
`void*`可以指向一个结构体,这个结构体封装了任意函数和函数调用需要的参数.
`void trampoline(void* _args)`这个蹦床函数就是用于我们封装实际调用的函数,唯一的作用就是调用args中实际的函数.