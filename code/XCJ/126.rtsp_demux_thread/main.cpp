#include <iostream>
#include "xtools.hpp"

class TestThread : public XThread{

public:
    void Main() override{
        LOGDEBUG(GET_STR(Main() begin));
        while (!m_is_exit){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        LOGDEBUG(GET_STR(Main() end));
    }

};


int main(const int argc,const char *argv[]){

    TestThread tt;
    tt.Start();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    tt.Stop();

    getchar();
    return 0;
}
