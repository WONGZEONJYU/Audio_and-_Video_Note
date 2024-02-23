#ifndef VADBG_H
#define VADBG_H

#include <iostream>

namespace vadbg
{
	void DumpString(const std::string& path, std::string str);
    void DumpBuffer(const std::string& path,const char *pBuffer, int nBufSize);
}

#endif // VADBG_H
