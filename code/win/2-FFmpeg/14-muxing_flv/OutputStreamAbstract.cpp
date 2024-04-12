
#include "OutputStreamAbstract.h"

#include "AVHelper.h"

int OutputStreamAbstract::write_media_file(AVFormatContext &fmt_ctx, const AVRational &time_base, AVStream &st, AVPacket &pkt) {

    /* rescale output packet timestamp values from codec to stream timebase */
    // 将packet的timestamp由codec to stream timebase pts_before = -1024
    av_packet_rescale_ts(&pkt, time_base, st.time_base);
    pkt.stream_index = st.index;
    AVHelper::log_packet(fmt_ctx,pkt);
    return av_interleaved_write_frame(&fmt_ctx, &pkt);
}
