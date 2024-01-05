#include <iostream>
#include <string>
#include <thread>
#include <SDL2/SDL.h>

#undef main
using namespace std;
using namespace chrono;
using namespace this_thread;

SDL_mutex *s_lock {};
SDL_cond *s_cond {};

int thread_work(void *arg)
{
    (void)arg;
    SDL_LockMutex(s_lock);
    cout << "                <============thread_work sleep\n";
    sleep_for(10s); // 用来测试获取锁
    cout << "                <============thread_work wait\n";

    // 释放s_lock资源，并等待signal。之所以释放s_lock是让别的线程能够获取到s_lock
    SDL_CondWait(s_cond, s_lock); //另一个线程(1)发送signal和(2)释放lock后，这个函数退出

    cout << "                <===========thread_work receive signal, continue to do ~_~!!!\n";
    cout << "                <===========thread_work end\n";
    SDL_UnlockMutex(s_lock);
    return 0;
}

int main()
{
    s_lock = SDL_CreateMutex();
    s_cond = SDL_CreateCond();

    auto t {SDL_CreateThread(thread_work,"thread_work",nullptr)};

    if(!t) {
        throw string(SDL_GetError());
    }

    for(int i {};i< 2;i++){
        sleep_for(2s);
        cout << "main execute =====>\n";
    }

    cout << "main SDL_LockMutex(s_lock) before ====================>\n";

    SDL_LockMutex(s_lock);  // 获取锁，但是子线程还拿着锁
    cout << "main ready send signal====================>\n";
    cout << "main SDL_CondSignal(s_cond) before ====================>\n";

    SDL_CondSignal(s_cond); // 发送信号，唤醒等待的线程
    cout << "main SDL_CondSignal(s_cond) after ====================>\n";

    sleep_for(10s);
    SDL_UnlockMutex(s_lock);// 释放锁，让其他线程可以拿到锁
    cout << "main SDL_UnlockMutex(s_lock) after ====================>\n";

    SDL_WaitThread(t, nullptr);
    SDL_DestroyMutex(s_lock);
    SDL_DestroyCond(s_cond);

    return 0;
}
