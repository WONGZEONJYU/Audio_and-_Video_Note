extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
}

#include <iostream>
#include <fstream>
#include <vector>
#include "XAVPacket.hpp"
#include "XAVFrame.hpp"
#include "XHelper.hpp"

using namespace std;
static void Decode(AVCodecContext *ctx,const XAVPacket_sptr &pkt,XAVFrame_sptr &frame);

int main(const int argc,const char *argv[]) {

    ifstream ifs("test.h264",ios::binary);
    AVCodecContext *codec_ctx{};
    AVCodecParserContext *parser_ctx{};

    const Destroyer d([&]{
        avcodec_free_context(&codec_ctx);
        av_parser_close(parser_ctx);
        ifs.close();
    });

    if (!ifs){
        PRINT_ERR_TIPS(GET_STR(test.h264 open failed!));
        return -1;
    }

    const auto codec_id{AV_CODEC_ID_H264};

    auto codec{avcodec_find_decoder(codec_id)};
    if (!codec){
        PRINT_ERR_TIPS(GET_STR(codec not found!));
        return -1;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        PRINT_ERR_TIPS(GET_STR(avcodec_alloc_context3 failed!));
        return -1;
    }

    FF_CHECK_ERR(avcodec_open2(codec_ctx,codec,nullptr),return -1);

    parser_ctx = av_parser_init(codec_id);
    if (!parser_ctx) {
        PRINT_ERR_TIPS(GET_STR(parser_ctx is empty!));
        return -1;
    }

    static constexpr auto DATA_SIZE{4096};
    pmr::vector<uint8_t> read_data(DATA_SIZE,0);

    XAVPacket_sptr packet;
    XAVFrame_sptr frame;
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);

    while (!ifs.eof()) {

        auto in_buffer{read_data.data()};
        ifs.read(reinterpret_cast<char *>(in_buffer),static_cast<long long>(read_data.capacity()));
        auto read_size {ifs.gcount()};

        while (read_size) {
            const auto parser_len{av_parser_parse2(parser_ctx, codec_ctx,
                                                   &packet->data, &packet->size,
                                                   in_buffer, static_cast<int>(read_size),
                                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)};
            read_size -= parser_len;
            in_buffer += parser_len;
            if (packet->size){
                //cerr << "packet.size = " << packet->size << "\t";
                Decode(codec_ctx,packet,frame);
            }
        }
    }

    av_parser_parse2(parser_ctx, codec_ctx,
                     &packet->data, &packet->size,
                     nullptr, 0,AV_NOPTS_VALUE, AV_NOPTS_VALUE,0);

    if (packet->size){
        //cerr << "packet.size = " << packet->size << "\t";
        cerr << "\n\nflush\n\n";
        Decode(codec_ctx,{},frame);
    }

    std::cerr << "\n\nparser and decode success!\n";
    return 0;
}

static void Decode(AVCodecContext *ctx,const XAVPacket_sptr &pkt,XAVFrame_sptr &frame){

    auto ret{avcodec_send_packet(ctx,pkt.get())};
    if (0 != ret || AVERROR(EAGAIN) != ret){
        FF_ERR_OUT(ret, return);
    }

    while (true) {
        ret = avcodec_receive_frame(ctx,frame.get());
        if (ret < 0){
            return;
        }
        cerr << av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)) <<
                " width: " << frame->width << " height: " << frame->height << "\n";
    }
}
