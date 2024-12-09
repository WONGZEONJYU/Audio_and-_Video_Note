#include <iostream>
#include "soapH.h"
#include "wsdd.nsmap"

int main(const int argc, const char * argv[]) {
    (void)argc, (void)argv;

    (void)argc;
    (void)argv;

    soap *soap {};
    soap = soap_new();
    soap_delete(soap,nullptr);
    return 0;
}

