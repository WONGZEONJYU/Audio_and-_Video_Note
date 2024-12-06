#include "soapH.h"
#include "wsdd.nsmap"

int main(const int argc,const char * const argv[]) {
    (void)argc;
    (void)argv;

    struct soap * soap = soap_new();
    soap_delete(soap,NULL);
    return 0;
}
