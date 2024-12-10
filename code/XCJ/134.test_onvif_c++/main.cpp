#include <iostream>
#include <algorithm>
#include "soapH.h"
#include "wsdd.nsmap"
#include "wsaapi.h"

using namespace std;

[[maybe_unused]] static constexpr char
X_ADDR[]{"soap.udp://239.255.255.250:3702"},
X_ACT[]{"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"},
X_TO[]{"urn:schemas-xmlsoap-org:ws:2005:04:discovery"},
X_ITEM[]{""},
X_TYPE[]{"tdn:NetworkVideoTransmitter tds:Device"};

int main(const int argc, const char * argv[]) {
    (void)argc, (void)argv;

    soap *soap_{};
    soap_ = soap_new();

    //设置命名空间
    soap_set_namespaces(soap_, namespaces);

    //设置超时时间
    constexpr auto timeout_sec{1};
    soap_->send_timeout = timeout_sec;
    soap_->recv_timeout= timeout_sec;
    soap_->connect_timeout = timeout_sec;

    //设置xml soap消息编码格式 统一utf格式
    soap_set_mode(soap_,SOAP_C_UTFSTRING);

    /******************************初始化消息头******************************/
    SOAP_ENV__Header *header{};
    header = static_cast<decltype(header)>(soap_malloc(soap_,sizeof(SOAP_ENV__Header)));
    fill_n(reinterpret_cast<uint8_t*>(header),sizeof(SOAP_ENV__Header),0);

    soap_default_SOAP_ENV__Header(soap_,header);

    header->wsa5__MessageID = const_cast<char*>(soap_wsa_rand_uuid(soap_)); //消息编号,唯一随机uuid
    //header->wsa5__MessageID = const_cast<char *>("urn:uuid:1419d68a-1dd2-11b2-a105-F000010A654C");
    cout << "header->wsa5__MessageID : " << header->wsa5__MessageID << "\n";
    
    header->wsa5__To = static_cast<decltype(header->wsa5__To)>(soap_malloc(soap_,size(X_TO) + 1));
    /* X_TO = urn:schemas-xmlsoap-org:ws:2005:04:discovery */
    copy_n(X_TO, size(X_TO),header->wsa5__To);
    header->wsa5__To[size(X_TO)] = 0;
    cout << "header->wsa5__To : " << header->wsa5__To << "\n";

    header->wsa5__Action = static_cast<decltype(header->wsa5__Action)>(soap_malloc(soap_,size(X_ACT) + 1));
    /*X_ACT = http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe */
    copy_n(X_ACT, size(X_ACT),header->wsa5__Action);
    header->wsa5__Action[size(X_ACT)] = 0;
    cout << "header->wsa5__Action : " << header->wsa5__Action << "\n";

    soap_->header = header;

    /********************************设定探测类型**************************************/
    //设定探测类型
    wsdd__ScopesType *scopes{};
    scopes = static_cast<decltype(scopes)>(soap_malloc(soap_,sizeof(wsdd__ScopesType)));
    fill_n(reinterpret_cast<uint8_t*>(scopes),sizeof(wsdd__ScopesType),0);
    //设定寻找设备的范围
    soap_default_wsdd__ScopesType(soap_,scopes);
    //设备范围
    scopes->__item = static_cast<decltype(scopes->__item)>(soap_malloc(soap_,size(X_ITEM) + 1));
    //strcpy(scopes->__item,X_ITEM);
    copy_n(X_ITEM, size(X_ITEM),scopes->__item);
    scopes->__item[size(X_ITEM)] = 0;
    cout << "scopes->__item : " << scopes->__item << "\n";

    /*设定寻找设备的类型*/
    /* tdn:NetworkVideoTransmitter tds:Device */
    wsdd__ProbeType probe_{};
    soap_default_wsdd__ProbeType(soap_,&probe_);
    probe_.Scopes = scopes;
    probe_.Types = static_cast<decltype(probe_.Types)>(soap_malloc(soap_,strlen(X_TYPE) + 1));
    //strcpy(probe_.Types,X_TYPE);
    copy_n(X_TYPE,size(X_TYPE),probe_.Types);
    probe_.Types[size(X_TYPE)] = 0;
    cout << "probe_.Types : " << probe_.Types << "\n";

    /*发送广播消息探测设备*/
    /* X_ADDR soap.udp://239.255.255.250:3702 */
    auto re{soap_send___wsdd__Probe(soap_,X_ADDR,nullptr,&probe_)};

    while (SOAP_OK == re) {
        __wsdd__ProbeMatches resp{};
        re = soap_recv___wsdd__ProbeMatches(soap_,&resp);
        if (SOAP_OK == re) {
            //cout << "#" << endl;
            const auto addr{resp.wsdd__ProbeMatches ? resp.wsdd__ProbeMatches->ProbeMatch->XAddrs : nullptr};
            cerr << (addr ? addr : "none") << "\n";
        }
    }

    /******************************清理******************************/
    cerr << "begin destroy\n";
    soap_destroy(soap_); //c++析构
    soap_end(soap_); //清理临时空间
    soap_done(soap_); //关闭通讯,移除回调
    soap_free(soap_); //删除soap对象

    return 0;
}
