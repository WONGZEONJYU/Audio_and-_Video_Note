#include <fstream>
#include "vadbg.h"

using namespace std;

namespace vadbg
{
	void DumpString(const string& path,const string& str)
	{
		ofstream fout(path);
		fout << str.c_str();
		fout.close();
	}

    void DumpBuffer(const string& path, const char *pBuffer,const int nBufSize)
	{
		ofstream fout(path,ios_base::binary);
		fout.write(reinterpret_cast<const char *>(pBuffer), nBufSize);
		fout.close();
	}
}
