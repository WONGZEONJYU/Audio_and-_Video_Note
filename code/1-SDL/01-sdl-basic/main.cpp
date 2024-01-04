#include <iostream>
#include <SDL.h>

using namespace std;

#undef main
int main(int argc,const char* argv[])
{
    SDL_version ver{};
    SDL_GetVersion(&ver);

    cout << dec << static_cast<int>(ver.major) << "." <<
            static_cast<int>(ver.minor) << "." <<
            static_cast<int>(ver.patch) << "\n";

    return 0;
}
