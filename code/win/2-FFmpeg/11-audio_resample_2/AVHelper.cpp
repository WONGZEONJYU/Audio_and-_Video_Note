#include "AVHelper.h"

namespace  AVHelper {

    std::string av_get_err(const int& errnum) {
        constexpr auto ERROR_STRING_SIZE {1024};
        char err_buf[ERROR_STRING_SIZE]{};
        av_strerror(errnum, err_buf, std::size(err_buf));
        return {err_buf};
    }

}
