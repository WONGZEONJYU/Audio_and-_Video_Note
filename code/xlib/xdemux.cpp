extern "C"{
#include <libavformat/avformat.h>
}

#include "xdemux.hpp"
#include "xavpacket.hpp"

AVFormatContext *XDemux::Open(const std::string &url) {

    CHECK_FALSE_(!url.empty(),PRINT_ERR_TIPS(GET_STR(url is empty!));return {});

    AVFormatContext *c{};
    AVDictionary *opts{};
    bool need_free{};

    const Destroyer d([&c,&opts,&need_free]{
        av_dict_free(&opts);
        if (need_free){
            avformat_close_input(&c);
        }
    });

    //FF_ERR_OUT(av_dict_set(&opts, GET_STR(rtsp_transport), GET_STR(tcp),0),return {});
    FF_ERR_OUT(av_dict_set(&opts, GET_STR(timeout),GET_STR(1000000),0),return {});
    FF_ERR_OUT(avformat_open_input(&c,url.c_str(),nullptr,&opts),return {});
    FF_ERR_OUT(avformat_find_stream_info(c, nullptr),need_free = true;return {});
    av_dump_format(c,0,url.c_str(),0);
    return c;
}

bool XDemux::Read(XAVPacket &packet) {
    check_fmt_ctx();
    FF_ERR_OUT(av_read_frame(m_fmt_ctx_,&packet),return {});
    m_last_time_ = XHelper::Get_time_ms();
    return true;
}

bool XDemux::Seek(const int64_t &pts,const int &stream_index) {

    if (stream_index < 0 || pts < 0) {
        PRINT_ERR_TIPS(GET_STR(params error!));
        return {};
    }

    check_fmt_ctx();

    FF_ERR_OUT(avformat_flush(m_fmt_ctx_));
#if 1
    FF_ERR_OUT (avformat_seek_file(m_fmt_ctx_,stream_index,INT64_MIN,
        pts,INT64_MAX,AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD),return {});
#else
    FF_ERR_OUT(av_seek_frame(m_fmt_ctx_,stream_index,pts,
                             AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD),return {});
#endif

    return true;
}
