
#ifndef AVHELPER_H
#define AVHELPER_H

extern "C"{
#include <libavutil/error.h>
}

#include <string>

namespace AVHelper {
    std::string av_get_err(const int& errnum);

}





#endif //AVHELPER_H
