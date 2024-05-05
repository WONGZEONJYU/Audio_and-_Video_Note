#include <iostream>
#include "AVFilter_Demo.hpp"

int main(const int argc,const char *argv[]) {

    if (argc < 3){
        std::cerr << "usage: in.yuv out.yuv";
        return -1;
    }

    try {
        auto filter{new_AVFilter_Demo(argv[1],argv[2])};
        filter->exec();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}

