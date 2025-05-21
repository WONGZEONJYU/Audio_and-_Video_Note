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

    {
        string main_rtsp,sub_rtsp;
        onvif->OnvifRtsp(ss.front(),main_rtsp,sub_rtsp,"admin","110120130..");
        cerr << "main: " << main_rtsp << "\nsub: " << sub_rtsp << "\n";
    }

    cerr << "\n\n";

    string url;
    onvif->MediaUrl(ss.front(),{},{},url);
    cout << "url: " << url << "\n";

    string main_token,sub_token;
    onvif->Profiles(url,main_token,sub_token,"admin","Fy225588");
    cout << "main_token: " << main_token << "\n"
        << "sub_token: " << sub_token << "\n";

    string rtsp;
    onvif->StreamUri(url,main_token,rtsp,"admin","Fy225588");
    cout << "rtsp : " << rtsp << "\n";
    return 0;
}





