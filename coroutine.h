#ifndef __COROUTINE_H__
#define __COROUTINE_H__
#include "coctx.h"
#include <cstddef>
#include <functional>
#include <memory>

class Coroutine;

void CoFund(Coroutine *);

class Coroutine
{
public:
    using Callback = std::function<void()>;
    using CoroutineSptr = std::shared_ptr<Coroutine>;
    
    enum Status
    {
        CO_READY = 0,
        CO_SUSPEND,
        CO_RUNNING,
        CO_DEAD
    };

public:
	Coroutine();
	~Coroutine();
    
    void setCallback(Callback);
    Callback getCallback() const { return cb_; }
    static void Resume(CoroutineSptr);
    static void Yield();

    static bool isMainCoroutine();

    static CoroutineSptr getMainCoroutine();
    static CoroutineSptr getCurCoroutine();
    static CoroutineSptr getInstanceCoroutine();
private:
    void coroutineMake();
    void stackCopy(char* top);
public:
    coctx_t ctx_;
    char* stack_sp_;
    ptrdiff_t cap_;
    ptrdiff_t size_;
	bool is_used_;
    Status status_;
private:    
    Callback cb_;
};

#endif

