#include <fstream>
#include "vadbg.h"

using namespace std;

namespace vadbg
{
	void DumpString(const string& path,const string& str)
	{
		ofstream fout(path.c_str());
		fout << str.c_str();
		fout.close();
	}

    void DumpBuffer(const string& path, uint8_t *pBuffer,const int nBufSize)
	{
		ofstream fin;
		fin.open(path.c_str(), ios_base::out | ios_base::binary);
		fin.write(reinterpret_cast<char *>(pBuffer), nBufSize);
		fin.close();
	}
}
