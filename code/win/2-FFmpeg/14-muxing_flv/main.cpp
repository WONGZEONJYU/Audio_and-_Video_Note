#include <iostream>

int main(const int argc,const char* argv[]) {

    if (argc < 2)
    {
        std::cerr << "usage: " << argv[0] << " output_file\n" <<
            "API example program to output a media file with libavformat.\n"
            "This program generates a synthetic audio and video stream, encodes and\n"
            "muxes them into a file named output_file.\n"
            "The output format is automatically guessed according to the file extension.\n"
            "Raw images can also be output by using '%%d' in the filename.\n\n";

        return -1;
    }



    return 0;
}