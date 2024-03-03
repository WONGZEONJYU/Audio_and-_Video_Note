#include <stdlib.h>
#include <string.h>

#include "vadbg.h"
#include "Videojj.h"

CVideojj::CVideojj()
{

}

CVideojj::~CVideojj()
{
	int i;
	for (i = 0; i < _vVjjSEI.size(); i++)
	{
		delete _vVjjSEI[i].szUD;
	}
}

// 用户可以根据自己的需要，对该函数进行修改或者扩展
// 下面这个函数的功能大致就是往视频中写入SEI信息
int CVideojj::Process(uint8_t *pNalu, const int nNaluLen,
	const int nTimeStamp)
{
    // 如果起始码后面的两个字节是0x05或者0x06，那么表示IDR图像或者SEI信息
	if (0x06 != pNalu[4] || 0x05 != pNalu[5]) {
		return 0;
	}

    auto p { pNalu + 4 + 2};
	while (*p++ == 0xff);
	constexpr auto szVideojjUUID { "VideojjLeonUUID"};
	auto pp {reinterpret_cast<char*>(p)};

	for (uint32_t i {}; i < strlen(szVideojjUUID); i++) {
		if (pp[i] != szVideojjUUID[i]){
			return 0;
		}
	}
	
	VjjSEI sei;
	sei.nTimeStamp = nTimeStamp;
	sei.nLen = nNaluLen - (pp - reinterpret_cast<char*>(pNalu)) - 16 - 1;
	sei.szUD = new char[sei.nLen];
	memcpy(sei.szUD, pp + 16, sei.nLen);
	_vVjjSEI.push_back(sei);

	return 1;
}
