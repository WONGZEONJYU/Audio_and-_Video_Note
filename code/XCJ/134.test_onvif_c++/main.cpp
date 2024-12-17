#include <iostream>
#include "xonvif.hpp"

using namespace std;

int main(const int argc, const char * argv[]) {
    (void)argc, (void)argv;

    const auto onvif{ XOnvif::new_XOnvif(1) };
    XOnvif::CamList ss;
    onvif->Detect_Cams(ss);
    for (const auto &item:ss) {
        cerr << item << "\n";
    }
    string url;
    if (ss.empty()){
        cerr << "empty\n" << endl;
        return 0;
    }
    onvif->MediaUrl(ss.front(),{},{},url);
    cout << url << "\n";
    return 0;
}
