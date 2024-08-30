#include <iostream>
#include <ctime>
#include <thread>
#include <Windows.h>

using namespace std;
using namespace chrono;
using namespace this_thread;

static inline constexpr auto TIME_BASE{CLOCKS_PER_SEC / 1000};

static inline void MSleep(const uint64_t &ms){

    const auto begin{clock()};
    for (uint64_t i {}; i < ms; ++i) {
        sleep_for(1ms);
        if ((clock() - begin) / (CLOCKS_PER_SEC / 1000) >= ms){
            break;
        }
    }
}

int main(const int argc,const char *argv[]) {

    int fps{};

    auto begin{clock()};
    for (;;){

        fps++;
        auto curr = clock();
        sleep_for(10ms);
        cerr << clock() - curr << " " << flush;
        if (((clock() - begin) / (CLOCKS_PER_SEC / 1000)) > 1000){
            cerr << "\nsleep fps: " << fps << flush;
            break;
        }
    }

    auto handle{CreateEvent({},{},{},{})};
    fps = 0;
    begin = clock();
    while (true){
        fps++;
        WaitForSingleObject(handle,10);
        if (((clock() - begin) / (CLOCKS_PER_SEC / 1000)) > 1000){
            cerr << "\nwait fps: " << fps << flush;
            break;
        }
    }

    fps = 0;
    begin = clock();
    while (true){
        fps++;
        MSleep(10);
        if (((clock() - begin) / (CLOCKS_PER_SEC / 1000)) > 1000){
            cerr << "\nMSleep fps: " << fps << flush;
            break;
        }
    }

    (void )getchar();
    return 0;
}
