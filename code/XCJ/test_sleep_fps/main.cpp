#include <iostream>
#include <thread>
#include <mutex>

using namespace std;
using namespace chrono;
using namespace this_thread;

static int64_t Get_time_ms() {
    const auto now_{std::chrono::high_resolution_clock::now()};
    const auto now_ms{std::chrono::time_point_cast<std::chrono::milliseconds>(now_)};
    return now_ms.time_since_epoch().count();
}

static inline void MSleep(const uint64_t &ms){
    const auto begin{Get_time_ms()};
    auto ms_{ms};
    while (ms_--) {
        sleep_for(1ms);
        const auto now{Get_time_ms()};
        if (now - begin >= ms){
            return;
        }
    }
}

int main(const int argc,const char *argv[]) {

    auto begin{Get_time_ms()};
    int64_t fps{};

    while (true) {
        ++fps;
        MSleep(10);
        if (Get_time_ms() - begin > 1000) {
            cerr << "fps: " << fps << "\n";
            break;
        }
    }

    cerr << "wait exit\n";
    return 0;
}
