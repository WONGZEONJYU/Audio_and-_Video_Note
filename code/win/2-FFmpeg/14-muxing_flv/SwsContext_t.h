#ifndef INC_14_MUXING_FLV_SWSCONTEXT_T_H
#define INC_14_MUXING_FLV_SWSCONTEXT_T_H

#include <memory>
extern "C"{
#include <libavutil/pixfmt.h>
};

struct SwsContext;
enum AVPixelFormat;
struct SwsFilter;

class SwsContext_t final {

    SwsContext *m_sws{};
    explicit SwsContext_t() = default;
    bool construct(const int &srcW,const int &srcH,
                   AVPixelFormat srcFormat,
                   const int &dstW,const int &dstH,
                   AVPixelFormat dstFormat,
                   const int &flags,
                   SwsFilter *srcFilter,
                   SwsFilter *dstFilter,
                   const double *param) noexcept;
public:
    using SwsContext_t_sp_type = std::shared_ptr<SwsContext_t>;
    SwsContext_t(const SwsContext_t &) = delete;
    SwsContext_t& operator=(const SwsContext_t &) = delete;
    static SwsContext_t_sp_type create(const int &srcW,const int &srcH,
                                       AVPixelFormat srcFormat,
                                       const int &dstW,const int &dstH,
                                       AVPixelFormat dstFormat,
                                       const int &flags,
                                       SwsFilter *srcFilter,
                                       SwsFilter *dstFilter,
                                       const double *param) noexcept(false);
    ~SwsContext_t();
};

#endif //INC_14_MUXING_FLV_SWSCONTEXT_T_H
