#include "xonvif.hpp"
#include <algorithm>
#include "soapH.h"
#include "wsdd.nsmap"
#include "wsaapi.h"
#include "wsseapi.h"

using namespace std;

[[maybe_unused]] static constexpr char
        X_ADDR[]{"soap.udp://239.255.255.250:3702"},
        X_ACT[]{"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"},
        X_TO[]{"urn:schemas-xmlsoap-org:ws:2005:04:discovery"},
        X_ITEM[]{""},
        X_TYPE[]{"tdn:NetworkVideoTransmitter tds:Device"};
//static constexpr auto TIMEOUT_SEC{3};

bool XOnvif::construct(const int &timeout_sec){

    if (!m_soap_){
        cerr << "soap alloc failed\n" << flush;
        return {};
    }

    //设置命名空间
    soap_set_namespaces(m_soap_, namespaces);

    //设置超时时间
    m_soap_->send_timeout = timeout_sec;
    m_soap_->recv_timeout= timeout_sec;
    m_soap_->connect_timeout = timeout_sec;

    //设置xml soap消息编码格式 统一utf格式
    soap_set_mode(m_soap_,SOAP_C_UTFSTRING);

    /*****************************************初始化消息头************************************/
    m_header_ = static_cast<decltype(m_header_)>(soap_malloc(m_soap_,sizeof(SOAP_ENV__Header)));
    if (!m_header_){
        cerr << "SOAP_ENV__Header alloc failed\n" << flush;
        return {};
    }
    *m_header_ = {};
    //fill_n(reinterpret_cast<uint8_t*>(m_header_),sizeof(SOAP_ENV__Header),0);
    soap_default_SOAP_ENV__Header(m_soap_,m_header_);

    m_header_->wsa5__MessageID = const_cast<char*>(soap_wsa_rand_uuid(m_soap_)); //消息编号,唯一随机uuid
    //header->wsa5__MessageID = const_cast<char *>("urn:uuid:1419d68a-1dd2-11b2-a105-F000010A654C");
    const auto msg_id{m_header_->wsa5__MessageID};
    cout << "header->wsa5__MessageID : " << (msg_id ? msg_id : "")  << "\n";

    m_header_->wsa5__To = static_cast<decltype(m_header_->wsa5__To)>(soap_malloc(m_soap_,size(X_TO) + 1));
    if (!m_header_->wsa5__To){
        cerr << "m_header->wsa5__To alloc failed\n" << flush;
        return {};
    }
    /* X_TO = urn:schemas-xmlsoap-org:ws:2005:04:discovery */
    copy_n(X_TO, size(X_TO),m_header_->wsa5__To);
    m_header_->wsa5__To[size(X_TO)] = 0;
    const auto wsa5_to{m_header_->wsa5__To};
    cout << "header->wsa5__To : " << (wsa5_to ? wsa5_to : "") << "\n";

    m_header_->wsa5__Action = static_cast<decltype(m_header_->wsa5__Action)>(soap_malloc(m_soap_,size(X_ACT) + 1));
    if (!m_header_->wsa5__Action){
        cerr << "m_header->wsa5__Action alloc failed\n" << flush;
        return {};
    }
    /*X_ACT = http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe */
    copy_n(X_ACT, size(X_ACT),m_header_->wsa5__Action);
    m_header_->wsa5__Action[size(X_ACT)] = 0;
    const auto wsa5_Action{m_header_->wsa5__Action};
    cout << "header->wsa5__Action : " << (wsa5_Action ? wsa5_Action : "") << "\n";
    m_soap_->header = m_header_;
    /*****************************************初始化消息头************************************/

    /***************************************设定探测类型**************************************/
    m_scopes_ = static_cast<decltype(m_scopes_)>(soap_malloc(m_soap_,sizeof(wsdd__ScopesType)));
    if (!m_scopes_){
        cerr << "wsdd__ScopesType alloc failed\n" << flush;
        return {};
    }
    *m_scopes_ = {};
    //fill_n(reinterpret_cast<uint8_t*>(m_scopes_),sizeof(wsdd__ScopesType),0);
    //设定寻找设备的范围
    soap_default_wsdd__ScopesType(m_soap_,m_scopes_);
    //设备范围
    m_scopes_->__item = static_cast<decltype(m_scopes_->__item)>(soap_malloc(m_soap_,size(X_ITEM) + 1));
    if (!m_scopes_->__item){
        cerr << "m_scopes_->__item alloc failed\n" << flush;
        return {};
    }
    //strcpy(scopes->__item,X_ITEM);
    copy_n(X_ITEM, size(X_ITEM),m_scopes_->__item);
    m_scopes_->__item[size(X_ITEM)] = 0;
    const auto item{m_scopes_->__item};
    cout << "scopes->__item : " << (item ? item : "") << "\n";
    /***************************************设定探测类型**************************************/

    /**************************************设定寻找设备的类型**********************************/
    /* tdn:NetworkVideoTransmitter tds:Device */
    //wsdd__ProbeType probe_{};

    m_probe_ = static_cast<decltype(m_probe_)>(soap_malloc(m_soap_, sizeof(wsdd__ProbeType)));
    if (!m_probe_){
        cerr << "wsdd__ProbeType alloc failed\n" << flush;
        return {};
    }

    soap_default_wsdd__ProbeType(m_soap_,m_probe_);
    m_probe_->Scopes = m_scopes_;
    m_probe_->Types = static_cast<decltype(m_probe_->Types)>(soap_malloc(m_soap_,strlen(X_TYPE) + 1));
    if (!m_probe_->Types){
        cerr << "probe_.Types alloc failed\n" << flush;
        return {};
    }

    copy_n(X_TYPE,size(X_TYPE),m_probe_->Types);
    m_probe_->Types[size(X_TYPE)] = 0;
    cout << "probe_.Types : " << (m_probe_->Types ? m_probe_->Types : "") << "\n";

    /**************************************设定寻找设备的类型**********************************/

    return true;
}

XOnvif::XOnvif():m_soap_(soap_new()){

}

XOnvif_sp XOnvif::new_XOnvif(const int &timeout_sec) {
    XOnvif_sp obj(new(nothrow) XOnvif);
    return obj && obj->construct(timeout_sec) ? obj : nullptr;
}

XOnvif::~XOnvif(){
    soap_destroy(m_soap_); //c++析构
    soap_end(m_soap_); //清理临时空间
    soap_done(m_soap_); //关闭通讯,移除回调
    soap_free(m_soap_); //删除soap对象
    //m_soap_ = nullptr;
}

[[maybe_unused]] int XOnvif::Detect_Cams(CamList &cam_list) {

    /*发送广播消息探测设备*/
    /* X_ADDR soap.udp://239.255.255.250:3702 */
    auto re{soap_send___wsdd__Probe(m_soap_,X_ADDR,nullptr,m_probe_)};
    int count{};
    while (SOAP_OK == re) {
        __wsdd__ProbeMatches resp{};
        re = soap_recv___wsdd__ProbeMatches(m_soap_,&resp);
        if (SOAP_OK == re) {
            //cout << "#" << endl;
            //const auto addr{resp.wsdd__ProbeMatches ? resp.wsdd__ProbeMatches->ProbeMatch->XAddrs : nullptr};
            //cerr << (addr ? addr : "none") << "\n";

            if (resp.wsdd__ProbeMatches){
                cam_list.emplace_back(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
            }
            ++count;
        }
    }
    return count;
}

[[maybe_unused]] bool XOnvif::MediaUrl(const string &device,
                                          const string &user,
                                          const string &passwd,
                                          string &url){
    if (device.empty()){
        return {};
    }
    /*鉴权*/
    if (!user.empty() && !passwd.empty()){
        soap_wsse_add_UsernameTokenDigest(m_soap_,{},user.c_str(),passwd.c_str());
    }

    /*
        enum tt__CapabilityCategory {
            tt__CapabilityCategory__All = 0,
            tt__CapabilityCategory__Analytics = 1,
            tt__CapabilityCategory__Device = 2,
            tt__CapabilityCategory__Events = 3,
            tt__CapabilityCategory__Imaging = 4,
            tt__CapabilityCategory__Media = 5,
            tt__CapabilityCategory__PTZ = 6
        };
    */

    _tds__GetCapabilities req{};
    _tds__GetCapabilitiesResponse resp{};

    if (SOAP_OK != soap_call___tds__GetCapabilities(m_soap_, device.c_str(),{},&req,resp)){
        cerr << *soap_faultstring(m_soap_) << "\n";
        return {};
    }

    url = resp.Capabilities->Media->XAddr;
    return true;
}

bool XOnvif::Profiles(const string &media_url,
                  string &main_token,
                  string &sub_token,
                  const string &user,
                  const string &passwd){

    if (media_url.empty()){
        return {};
    }

    /*鉴权*/
    if (!user.empty() && !passwd.empty()){
        soap_wsse_add_UsernameTokenDigest(m_soap_,{},user.c_str(),passwd.c_str());
    }

    _trt__GetProfiles req{};
    _trt__GetProfilesResponse resp{};
    const auto ret{soap_call___trt__GetProfiles(m_soap_,
            media_url.c_str(),
            nullptr, //action
            &req, resp)};

    if (SOAP_OK != ret){
        cerr << *soap_faultstring(m_soap_) << "\n";
        return {};
    }

    if (!resp.Profiles.empty()){
        main_token = resp.Profiles[0]->token;
    }
    if (resp.Profiles.size() > 1){
        sub_token = resp.Profiles[1]->token;
    }
    return true;
}

bool XOnvif::StreamUri(const string& media_url,
          const string &token,
          string &rtsp,
          const string &user,
          const string &passwd){


    if (media_url.empty()){
        return {};
    }

    /*鉴权*/
    if (!user.empty() && !passwd.empty()){
        soap_wsse_add_UsernameTokenDigest(m_soap_,{},user.c_str(),passwd.c_str());
    }

    _trt__GetStreamUri req{};
    _trt__GetStreamUriResponse resp{};

    //请求的设置
    tt__StreamSetup setup{};
    tt__Transport transport{};

    setup.Stream = tt__StreamType::RTP_Unicast,//单播
    setup.Transport = &transport;
    setup.Transport->Protocol = tt__TransportProtocol::RTSP; //协议
    req.StreamSetup = &setup;
    req.ProfileToken = token;

    const auto ret{soap_call___trt__GetStreamUri(m_soap_, media_url.c_str(),
                                                 nullptr, //action
                                                 &req,
                                                 resp)};
    if (SOAP_OK != ret){
        cerr << *soap_faultstring(m_soap_) << "\n";
        return {};
    }
    rtsp = resp.MediaUri->Uri;
    return true;
}

bool XOnvif::OnvifRtsp(const string &device,
               string &main_rtsp,
               string &sub_rtsp,
               const string &user,
               const string &passwd){

    if (device.empty()){
        return {};
    }

    string media_url,main_token,sub_token;

    if (!MediaUrl(device,user,passwd,media_url)){
        return {};
    }

    if (!Profiles(media_url,main_token,sub_token,user,passwd)){
        return {};
    }

    if (!StreamUri(media_url,main_token,main_rtsp,user,passwd)){
        return {};
    }

    if(!StreamUri(media_url,sub_token,sub_rtsp,user,passwd)){
        return {};
    }

    return true;
}
