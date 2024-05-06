#include <iostream>
#include "Watermark_Demo.hpp"


int main(const int argc,const char **argv) {

    if (argc < 4){
        std::cerr << "usage main.jpg logo.jpeg out.jpg\n";
        return -1;
    }

    try {
        auto w{new_Watermark_Demo(argv)};
        w->exec();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}


