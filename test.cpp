#include "coroutine.h"
#include <iostream>
#include <thread>

int g = 1000;

void foo(int a)
{
    for (int i = a; i < g + 5; ++i)
    {
        std::cout << __func__ << " " << i << std::endl;
        Coroutine::Yield();
    }
}

/** @TODO 输出参数并未出现变化*/
int main()
{
    Coroutine* c1 = Coroutine::getInstanceCoroutine();
    Coroutine* c2 = Coroutine::getInstanceCoroutine();
    
    c1->setCallback(std::bind(foo, 0));
    c2->setCallback(std::bind(foo, 5));

    while(c1->is_execFunc_ && c2->is_execFunc_)
    {
        Coroutine::Resume(c1);
        Coroutine::Resume(c2);
    }

    // if (c1->is_execFunc_ && c2->is_execFunc_)
    //     std::cout << "error" << std::endl;
    return 0;
}

