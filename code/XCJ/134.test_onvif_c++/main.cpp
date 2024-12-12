#include <iostream>
#include "xonvif.hpp"

using namespace std;

int main(const int argc, const char * argv[]) {
    (void)argc, (void)argv;

    const auto onvif{ XOnvif::new_XOnvif(10) };
    std::stringstream ss;
    onvif->dectect_Cams(ss);
    cout << ss.str() << "\n";
    return 0;
}
