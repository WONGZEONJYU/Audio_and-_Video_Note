#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include "FlvParser.h"

using namespace std;

#if 0
int main(int argc, char *argv[])
{
    std::cout << "fuck\n";
    return 0;
}
#else
static void Process(fstream &fin, const char *filename);
int main(const int argc,const char *argv[])
{
    cout << "Hi, this is FLV parser test program!\n";

    if (argc < 3){
        cerr << "FlvParser.exe [input flv] [output flv]\n";
        return -1;
    }

    fstream fin(argv[1],ios_base::binary);

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
        nFlvPos += nReadNum;
        int nUsedLen{};
        parser.Parse(pBuf, nFlvPos, nUsedLen);

        if (nFlvPos != nUsedLen) {
            //memcpy(pBak, pBuf + nUsedLen, nFlvPos - nUsedLen);
            std::copy_n(pBuf + nUsedLen,nFlvPos - nUsedLen,pBak);
            //memcpy(pBuf, pBak, nFlvPos - nUsedLen);
            std::copy_n(pBak,nFlvPos - nUsedLen,pBuf);
        }
        nFlvPos -= nUsedLen;
    }
    parser.PrintInfo();
    parser.DumpH264("parser.264");
    parser.DumpAAC("parser.aac");

    //dump into flv
    parser.DumpFlv(filename);

    delete []pBak;
    delete []pBuf;
}
#endif
