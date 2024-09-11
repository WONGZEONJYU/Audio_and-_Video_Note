extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
}

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "xdecode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"
#include "xhelper.hpp"
#include "xvideo_view.hpp"

using namespace std;
static bool Decode(AVCodecContext *ctx,
                   const XAVPacket_sptr &pkt,
                   XAVFrame_sptr &frame,
                   XAVFrame_sptr &hw_frame,
                   int64_t &begin,
                   int &count,XVideoView *);

int main(const int argc,const char *argv[]) {

    if (argc < 2){
        PRINT_ERR_TIPS(GET_STR(argv miss!\n));
        return -1;
    }

    ifstream ifs(argv[1],ios::binary);
    AVCodecParserContext *parser_ctx{};
    XVideoView* view{};

    const Destroyer d([&]{
        cerr << "Destroyer\n";
        av_parser_close(parser_ctx);
        ifs.close();
        delete view;
    });

    /**
     * 创建显示对象
     */
    TRY_CATCH(CHECK_EXC(view = XVideoView::create()),return -1);

    if (!ifs) {
        PRINT_ERR_TIPS(GET_STR(h264_file open failed!));
        return -1;
    }
    const auto codec_id {AV_CODEC_ID_H264};

    XDecode de;
    auto c{XCodec::Create(codec_id,false)};
    de.set_codec_ctx(c);
    de.InitHw();
    de.Open();

    /**
     * 初始化解析器
     */
    parser_ctx = av_parser_init(codec_id);
    if (!parser_ctx) {
        PRINT_ERR_TIPS(GET_STR(parser_ctx is empty!));
        return -1;
    }

    static constexpr auto DATA_SIZE{4096};
    pmr::vector<uint8_t> read_data(DATA_SIZE,0);

    XAVPacket_sptr packet;
    XAVFrame_sptr frame;
    //XAVFrame_sptr hw_frame;
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);
    //TRY_CATCH(CHECK_EXC(hw_frame = new_XAVFrame()),return -1);

    int count{};
    auto begin{XVideoView::Get_time_ms()};
    bool is_view_init{};

    while (!ifs.eof()) {

        auto in_buffer{read_data.data()};
        ifs.read(reinterpret_cast<char *>(in_buffer),static_cast<long long>(read_data.capacity()));
        auto read_size {ifs.gcount()};

//        if (ifs.eof()){
//            ifs.clear();
//            ifs.seekg(0,ios::beg);
//        }

        while (read_size) {
            /**
             * 解析H264裸流
             */
            const auto parser_len{av_parser_parse2(parser_ctx, c,
                                                   &packet->data, &packet->size,
                                                   in_buffer, static_cast<int>(read_size),
                                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)};
            read_size -= parser_len;
            in_buffer += parser_len;
            if (packet->size) {
                //cerr << "packet.size = " << packet->size << "\t";
                while (de.Receive(frame)){
                    if (!is_view_init){
                        is_view_init = true;
                        view->Init(frame->width,frame->height,static_cast<XVideoView::Format>(frame->format));
                    }
                    view->DrawFrame(frame);
                    ++count;
                    const auto curr_time{XVideoView::Get_time_ms()};
                    if (curr_time - begin >= 10LL) {
                        std::cerr << "fps :" << count * 100<< "\n";
                        count = 0;
                        begin = curr_time;
                    }
                    cerr << av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)) << "\n";
                }

                if (!de.Send(packet)){
                    break;
                }

                if (view->Is_Exit_Window()){
                    return 0;
                }
            }
        }
    }

    {
        /**
         * 冲刷解析器,把缓存全部读取出来
         */
        av_parser_parse2(parser_ctx, c,
                         &packet->data, &packet->size,
                         nullptr, 0, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        if (packet->size) {
            //cerr << "packet.size = " << packet->size << "\t";
            cerr << "\n\nflush\n\n";
            /**
             * 冲刷解码器,把缓存的帧全部读取出来
             */
            if (!de.Send(packet)){
                return -1;
            }

            while (de.Receive(frame)){
                view->DrawFrame(frame);
                ++count;
                const auto curr_time{XVideoView::Get_time_ms()};
                if (curr_time - begin >= 10LL) {
                    std::cerr << "fps :" << count * 100<< "\n";
                    count = 0;
                    begin = curr_time;
                }
                cerr << av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)) << "\n";
            }
        }

        auto frames{de.Flush()};
        for (auto &item : frames) {
            view->DrawFrame(item);
        }
        std::cerr << "\n\nparser and decode success!\n";
    }

    return 0;
}

#if 0
static bool Decode(AVCodecContext *ctx,
                   const XAVPacket_sptr &pkt,
                   XAVFrame_sptr &frame,
                   XAVFrame_sptr &hw_frame,
                   int64_t &begin,
                   int &count,
                   XVideoView *view){

    static bool is_view_init{};

    /**
     * 发送未压缩的包到解码器
     */
    auto ret{avcodec_send_packet(ctx,pkt.get())};
    if (0 != ret || AVERROR(EAGAIN) != ret) {
        FF_ERR_OUT(ret, return {});
    }

    while (true) {
        /**
         * 读取解码后的帧
         */
        if (avcodec_receive_frame(ctx,frame.get()) < 0) {
            return false;
        }

        auto p_frame{frame};
        if (ctx->hw_device_ctx){
            FF_ERR_OUT(av_hwframe_transfer_data(hw_frame.get(),frame.get(),0), return {});
            p_frame = hw_frame;
        }

        ++count;
        cerr << av_get_pix_fmt_name(static_cast<AVPixelFormat>(p_frame->format)) << "\n";

        const auto curr_time{XVideoView::Get_time_ms()};
        if (curr_time - begin >= 1000LL) {
            std::cerr << "fps :" << count << "\n";
            count = 0;
            begin = curr_time;
        }

        if (!is_view_init) {
            /**
             * 用第一帧来初始化显示
             */
            is_view_init = true;
            view->Init(p_frame->width,p_frame->height,static_cast<XVideoView::Format>(p_frame->format));
        }

        /**
         * 显示一帧画面
         */
        view->DrawFrame(p_frame);
        if (view->Is_Exit_Window()){
            return true;
        }
    }
}

#endif
