#include "coroutine.h"
#include <iostream>
#include <thread>


int g = 10000;

void foo1(int a) {
    for (int i = a; i < g + a; ++i) {
        std::cout << __func__ << " int thread " << std::this_thread::get_id() << " value is " << i << std::endl;
        Coroutine::Yield();
    }
}

void foo2() {
    for (int i = 0; i < 500; ++i) {
        std::cout << __func__ << " in thread " << std::this_thread::get_id() << "value is " << i << std::endl;
        Coroutine::Yield();
    }
}

void foo() {
     Coroutine* co1 = Coroutine::getInstanceCoroutine();
    //  Coroutine* co2 = Coroutine::getInstanceCoroutine();

    co1->setCallback(std::bind(foo1, 0));
    // co2->setCallback(foo2);
    
    for (int i = 0; i < g; ++i) {
        Coroutine::Resume(co1);
        // Coroutine::Resume(co2);
    }

}

/** @TODO 输出参数并未出现变化*/
int main()
{
    std::thread t1(foo);
    // std::thread t2(foo);

    t1.join();
    // t2.join();

    return 0;
}

