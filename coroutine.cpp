#include "coroutine.h"
#include "coctx.h"
#include <cstring>
#include <memory>
#include <cassert>
#include <cstdlib>
#include <thread>
#include <vector>
#include <iostream>

static thread_local Coroutine::CoroutineSptr t_mainCoroutine = nullptr;
static thread_local Coroutine::CoroutineSptr t_curCoroutine = nullptr;
static thread_local char shared_stack[1024 * 512];
static thread_local std::vector<Coroutine::CoroutineSptr> t_coroutinePool;

void CoFunc(Coroutine *co) {
    Coroutine::Callback cb = co->getCallback();
    if (cb) 
        cb();
    Coroutine::Yield();
}

Coroutine::Coroutine()
    : stack_sp_(nullptr),
      cap_(0),
      size_(0),
      is_used_(false),
      status_(CO_READY)
{
    memset(&ctx_, 0, sizeof ctx_);
    /* if (t_mainCoroutine == nullptr) { */
    /*     t_mainCoroutine = this; */
    /*     t_curCoroutine = t_mainCoroutine; */
    /* } */
}

Coroutine::~Coroutine() {
    if (stack_sp_) 
        free(stack_sp_);
    status_ = CO_DEAD;
}

void Coroutine::stackCopy(char* top) {
    char dummy = 0;
    assert(top - &dummy <= sizeof shared_stack);
    if (cap_ < top - &dummy) {
        stack_sp_ = nullptr;
        cap_ = top - &dummy;
        stack_sp_ = reinterpret_cast<char*>(malloc(cap_));
    }

    size_ = top - &dummy;
    memcpy(stack_sp_, &dummy, size_);
}

void Coroutine::coroutineMake() {
    stack_sp_ = shared_stack;
    size_ = sizeof shared_stack;
    char* top = stack_sp_ + size_;
    top = reinterpret_cast<char *>((reinterpret_cast<unsigned long>(top)) & -16LL);

    memset(&ctx_, 0, sizeof ctx_);
    ctx_.regs[kRSP] = top;
    ctx_.regs[kRBP] = top;
    ctx_.regs[kRETAddr] = reinterpret_cast<char*>(CoFunc);
    ctx_.regs[kRDI] = reinterpret_cast<char*>(this);
}

bool Coroutine::isMainCoroutine() {
    if (t_mainCoroutine == nullptr || t_mainCoroutine == t_curCoroutine)
        return true;
    return false;
}

void Coroutine::setCallback(Callback cb) {
    cb_ = cb;
    getMainCoroutine();
}

Coroutine::CoroutineSptr Coroutine::getMainCoroutine() {
    if (t_mainCoroutine == nullptr) {
        t_mainCoroutine = std::make_shared<Coroutine>();
        t_curCoroutine = t_mainCoroutine;
    }
    return t_mainCoroutine;
}

Coroutine::CoroutineSptr Coroutine::getCurCoroutine() {
    if (t_curCoroutine == nullptr)
        getMainCoroutine();
    return t_curCoroutine;
}

Coroutine::CoroutineSptr Coroutine::getInstanceCoroutine() {
    int cur = 0;
    for (; cur < t_coroutinePool.size() && !t_coroutinePool.empty(); ++cur) {
        if (t_coroutinePool[cur] != nullptr && t_coroutinePool[cur]->is_used_ == false && t_coroutinePool[cur]->status_ == CO_READY) 
            break;
    }
    
    if (cur >= t_coroutinePool.size()) {
        for (int i = cur; i < cur + 16; ++i) {
            t_coroutinePool.push_back(std::make_shared<Coroutine>());
        }
    }
    std::cout << "get instance coroutine" << std::endl;
    t_coroutinePool[cur]->is_used_ = true;
    return t_coroutinePool[cur];
}

void Coroutine::Resume(CoroutineSptr co) {
    if (co == nullptr || co->status_ == CO_DEAD) {
        /* target coroutine is nullptr*/
        return;
    }

    if (!isMainCoroutine()) {
        /* current coroutine is not main coroutine without the ability to resume sub-couroutine*/
        return;
    }

    if (co->status_ == CO_READY)
        co->coroutineMake();
    else if (co->status_ == CO_SUSPEND) {
        memcpy(shared_stack + sizeof shared_stack - co->size_, co->stack_sp_, co->size_);
    }
    t_curCoroutine = co;
    coctx_swap(&t_mainCoroutine->ctx_, &co->ctx_);
}

void Coroutine::Yield() {
    if (isMainCoroutine()) {
        /* main coroutine cannot Yield*/
        return;
    }

    CoroutineSptr co = t_curCoroutine;
    t_curCoroutine = t_mainCoroutine;
    co->status_ = CO_SUSPEND;
    co->stackCopy(shared_stack + sizeof shared_stack);
    coctx_swap(&co->ctx_, &t_mainCoroutine->ctx_);
}
