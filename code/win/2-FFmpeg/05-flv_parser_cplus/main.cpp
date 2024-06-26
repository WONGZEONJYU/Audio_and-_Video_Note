﻿#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <iostream>
#include <fstream>

#include "FlvParser.h"

using namespace std;

#if 0

static constexpr uint8_t script_tag[]{
	0x12, 0x00, 0x01, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0A, 0x6F, 0x6E,
	0x4D, 0x65, 0x74, 0x61, 0x44, 0x61, 0x74, 0x61, 0x08, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x64,
	0x75, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x00, 0x40, 0x14, 0x7A, 0xe1, 0x47, 0xAE, 0x14, 0x7B,
	0x00, 0x05, 0x77, 0x69, 0x64, 0x74, 0x68, 0x00, 0x40, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x06, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x00, 0x40, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0D, 0x76, 0x69, 0x64, 0x65, 0x6F, 0x64, 0x61, 0x74, 0x61, 0x72, 0x61, 0x74, 0x65,
	0x00, 0x40, 0x69, 0xE8, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x66, 0x72, 0x61, 0x6D, 0x65,
	0x72, 0x61, 0x74, 0x65, 0x00, 0x40, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x76,
	0x69, 0x64, 0x65, 0x6F, 0x63, 0x6F, 0x64, 0x65, 0x63, 0x69, 0x64, 0x00, 0x40, 0x1C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x61, 0x75, 0x64, 0x69, 0x6F, 0x64, 0x61, 0x74, 0x61, 0x72,
	0x61, 0x74, 0x65, 0x00, 0x40, 0x3D, 0x54, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x61, 0x75,
	0x64, 0x69, 0x6F, 0x73, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x72, 0x61, 0x74, 0x65, 0x00, 0x40, 0xE5,
	0x88, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x61, 0x75, 0x64, 0x69, 0x6F, 0x73, 0x61, 0x6D,
	0x70, 0x6C, 0x65, 0x73, 0x69, 0x7A, 0x65, 0x00, 0x40, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x06, 0x73, 0x74, 0x65, 0x72, 0x65, 0x6F, 0x01, 0x01, 0x00, 0x0C, 0x61, 0x75, 0x64, 0x69,
	0x6F, 0x63, 0x6F, 0x64, 0x65, 0x63, 0x69, 0x64, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0B, 0x6D, 0x61, 0x6A, 0x6F, 0x72, 0x5F, 0x62, 0x72, 0x61, 0x6E, 0x64, 0x02, 0x00,
	0x04, 0x69, 0x73, 0x6F, 0x6D, 0x00, 0x0D, 0x6D, 0x69, 0x6E, 0x6F, 0x72, 0x5F, 0x76, 0x65, 0x72,
	0x73, 0x69, 0x6F, 0x6E, 0x02, 0x00, 0x03, 0x35, 0x31, 0x32, 0x00, 0x11, 0x63, 0x6F, 0x6D, 0x70,
	0x61, 0x74, 0x69, 0x62, 0x6C, 0x65, 0x5F, 0x62, 0x72, 0x61, 0x6E, 0x64, 0x73, 0x02, 0x00, 0x10,
	0x69, 0x73, 0x6F, 0x6D, 0x69, 0x73, 0x6F, 0x32, 0x61, 0x76, 0x63, 0x31, 0x6D, 0x70, 0x34, 0x31,
	0x00, 0x07, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x72, 0x02, 0x00, 0x0D, 0x4C, 0x61, 0x76, 0x66,
	0x35, 0x38, 0x2E, 0x32, 0x39, 0x2E, 0x31, 0x30, 0x30, 0x00, 0x08, 0x66, 0x69, 0x6C, 0x65, 0x73,
	0x69, 0x7A, 0x65, 0x00, 0x40, 0xF2, 0xBC, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09
};

int main(int argc, char *argv[])
{
    std::cout << "Start Test\n";



	std::cout << "End Test\n";
    return 0;
}
#else
static void Process(fstream &fin, const char *filename);
int main(const int argc,const char *argv[])
{
	std::cout << sizeof(*argv) << "\n";
	cout << sizeof(string) << "\n";
    cout << "Hi, this is FLV parser test program!\n";

    if (argc < 3){
        cerr << "FlvParser.exe [input flv] [output flv]\n";
        return -1;
    }

    fstream fin(argv[1],ios_base::binary | ios_base::in);

    if (!fin){
        cerr << "open error\n";
        return -1;
    }
    Process(fin, argv[2]);
    fin.close();
    return 0;
}

static void Process(fstream &fin, const char *filename)
{
    CFlvParser parser;
    constexpr auto nBufSize {2 * 1024 * 1024};
    int nFlvPos {};
    const auto pBuf { new uint8_t[nBufSize]},
    pBak {new uint8_t[nBufSize]};

    for(;;){

        fin.read(reinterpret_cast<char *>(pBuf + nFlvPos), nBufSize - nFlvPos);
        const auto nReadNum {fin.gcount()};
        if (!nReadNum) {
            break;
        }
        
        nFlvPos += static_cast<int>(nReadNum);
        int nUsedLen{};
        parser.Parse(pBuf, nFlvPos, nUsedLen);

        if (nFlvPos != nUsedLen) {
	        std::cout << "nFlvPos = " << nFlvPos << " , nUsedLen = " << nUsedLen << "\n";
            std::copy_n(pBuf + nUsedLen,nFlvPos - nUsedLen,pBak);
            std::copy_n(pBak,nFlvPos - nUsedLen,pBuf);
        }
        nFlvPos -= nUsedLen;
    }

    parser.PrintInfo();
    (void)parser.DumpH264("parser.264");
    parser.DumpAAC("parser.aac");

    //dump into flv
    (void)parser.DumpFlv(filename);

    delete []pBak;
    delete []pBuf;
}
#endif
