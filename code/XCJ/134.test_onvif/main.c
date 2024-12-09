#include "soapH.h"
#include "wsdd.nsmap"
#include "wsaapi.h"

#define X_TO "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define X_ACT "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
#define X_IEAM ""
#define X_TYPE "tdn:NetworkVideoTransmitter tds:Device"  //设备类型
#define X_ADDR "soap.udp://239.255.255.250:3702"
int main(const int argc,const char *argv[])
{
    (void )argc,(void )argv;
    //for(;;){
    /////////////////////////////////////////////////////////
    /// 初始化soap
    struct soap* soap;
    int timeout_sec = 1;
    soap = soap_new();

    //设置命名空间
    soap_set_namespaces(soap, namespaces);

    //设置超时时间
    soap->recv_timeout = timeout_sec;
    soap->send_timeout = timeout_sec;
    soap->connect_timeout = timeout_sec;

    //设置xml soap消息编码格式 utf
    soap_set_mode(soap, SOAP_C_UTFSTRING);


    ///////////////////////////////////////////////////
    /// 初始化消息头
    struct SOAP_ENV__Header* header;
    header = (struct SOAP_ENV__Header*)soap_malloc(soap,
                                                   sizeof( struct SOAP_ENV__Header));
    memset(header, 0, sizeof(struct SOAP_ENV__Header));
    soap_default_SOAP_ENV__Header(soap, header);
    header->wsa5__MessageID = (char*)soap_wsa_rand_uuid(soap); // 消息编号，唯一随机uuid
    //"urn:schemas-xmlsoap-org:ws:2005:04:discovery"
    header->wsa5__To = soap_malloc(soap, strlen(X_TO) + 1);

    //C4996	_CRT_SECURE_NO_WARNINGS
    strcpy(header->wsa5__To, X_TO);
    //"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
    header->wsa5__Action = soap_malloc(soap, strlen(X_ACT) + 1);
    strcpy(header->wsa5__Action, X_ACT);
    soap->header = header;


    //设定探测类型
    struct wsdd__ScopesType* scope;
    scope = soap_malloc(soap, sizeof(wsdd__ScopesType));
    memset(scope,0,sizeof (struct wsdd__ScopesType));
    //设定寻找设备的范围
    soap_default_wsdd__ScopesType(soap, scope);
    //设备范围
    scope->__item = soap_malloc(soap, strlen(X_IEAM) + 1);
    strcpy(scope->__item, X_IEAM);

    //设定寻找设备的类型
    //"tdn:NetworkVideoTransmitter tds:Device"
    struct wsdd__ProbeType probe;
    memset(&probe, 0, sizeof(probe));
    soap_default_wsdd__ProbeType(soap, &probe);
    probe.Scopes = scope;
    probe.Types = soap_malloc(soap, strlen(X_TYPE)+1);
    strcpy(probe.Types, X_TYPE);

    //发送广播消息探测设备
    // "soap.udp://239.255.255.250:3702"
    int re = soap_send___wsdd__Probe(soap, X_ADDR, nullptr, &probe);


    while (SOAP_OK == re){
        struct __wsdd__ProbeMatches resp; //接收探测结果
        re = soap_recv___wsdd__ProbeMatches(soap, &resp);
        if (SOAP_OK == re){
            printf("#\n");
            //char *device_service = resp.wsdd__ProbeMatches->ProbeMatch->XAddrs;
            //printf("device_service:[%s]\n", device_service);
        }
    }
    ////////////////////////////////////////////////////
    ///清理soap
    printf("begin Clear soap!\n");
    soap_destroy(soap);     // c++析构
    soap_end(soap);         //清理临时空间
    soap_done(soap);        //关闭通信，移除回调
    soap_free(soap);        //删除soap对象空间
    // }
    return 0;
}