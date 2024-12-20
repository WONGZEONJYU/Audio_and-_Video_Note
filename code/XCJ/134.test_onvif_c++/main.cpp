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

    if (ss.empty()){
        cerr << "empty\n" << endl;
        return 0;
    }
    string url;
    onvif->MediaUrl(ss.front(),{},{},url);
    cout << "url: " << url << "\n";
    string main_token,sub_token;
    onvif->Profiles(url,main_token,sub_token,"admin","Fy225588");
    cout << "main_token: " << main_token << "\n"
        << "sub_token: " << sub_token << "\n";

    return 0;
}
