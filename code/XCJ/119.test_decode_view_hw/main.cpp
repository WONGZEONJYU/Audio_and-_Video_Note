extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
}

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "XAVPacket.hpp"
#include "XAVFrame.hpp"
#include "XHelper.hpp"
#include "xvideo_view.hpp"

using namespace std;
static void Decode(AVCodecContext *ctx,const XAVPacket_sptr &pkt,XAVFrame_sptr &frame,int64_t &begin,int &count,XVideoView *);

int main(const int argc,const char *argv[]) {

    if (argc < 2){
        PRINT_ERR_TIPS(GET_STR(argv miss!\n));
        return -1;
    }

    ifstream ifs(argv[1],ios::binary);
    AVCodecContext *codec_ctx{};
    AVCodecParserContext *parser_ctx{};
    XVideoView* view{};

    const Destroyer d([&]{
        avcodec_free_context(&codec_ctx);
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

    const auto codec_id{AV_CODEC_ID_H264};

    /**
     * 查找解码器
     */
    auto codec{avcodec_find_decoder(codec_id)};
    if (!codec){
        PRINT_ERR_TIPS(GET_STR(codec not found!));
        return -1;
    }

    /**
     * 分配解码器上下文
     */
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        PRINT_ERR_TIPS(GET_STR(avcodec_alloc_context3 failed!));
        return -1;
    }
#if MACOS
    const auto hw_type{AV_HWDEVICE_TYPE_VIDEOTOOLBOX};
#else
    const auto hw_type{AV_HWDEVICE_TYPE_DXVA2};
#endif
    /**
     * 列出所有支持的硬件加速方式
     */
    for (int i {};;++i) {
        auto config{avcodec_get_hw_config(codec,i)};
        if (!config) {
            break;
        }
        if (config->device_type){
            cerr << av_hwdevice_get_type_name(config->device_type) << "\n";
        }
    }
    AVBufferRef *hw_ctx{};
    FF_ERR_OUT(av_hwdevice_ctx_create(&hw_ctx,hw_type,{},{},{}),return -1);
    //codec_ctx->hw_device_ctx = av_buffer_ref(hw_ctx);

    /**
     * 设置解码器线程数
     */
    codec_ctx->thread_count = static_cast<int>(thread::hardware_concurrency());

    /**
     * 打开解码器
     */
    FF_CHECK_ERR(avcodec_open2(codec_ctx,codec,nullptr),return -1);

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
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);

    int count{};
    auto begin{XVideoView::Get_time_ms()};
    while (!ifs.eof()) {

        auto in_buffer{read_data.data()};
        ifs.read(reinterpret_cast<char *>(in_buffer),static_cast<long long>(read_data.capacity()));
        auto read_size {ifs.gcount()};

        if (ifs.eof()){
            ifs.clear();
            ifs.seekg(0,ios::beg);
        }

        while (read_size) {
            /**
             * 解析H264裸流
             */
            const auto parser_len{av_parser_parse2(parser_ctx, codec_ctx,
                                                   &packet->data, &packet->size,
                                                   in_buffer, static_cast<int>(read_size),
                                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)};
            read_size -= parser_len;
            in_buffer += parser_len;
            if (packet->size) {
                //cerr << "packet.size = " << packet->size << "\t";
                /**
                 * 解码
                 */
                Decode(codec_ctx,packet,frame,begin,count,view);
            }
        }
    }

    {
        /**
         * 冲刷解析器,把缓存全部读取出来
         */
        av_parser_parse2(parser_ctx, codec_ctx,
                         &packet->data, &packet->size,
                         nullptr, 0, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        if (packet->size) {
            //cerr << "packet.size = " << packet->size << "\t";
            cerr << "\n\nflush\n\n";
            /**
             * 冲刷解码器,把缓存的帧全部读取出来
             */
            Decode(codec_ctx, {}, frame, begin, count, view);
        }
    }

    std::cerr << "\n\nparser and decode success!\n";

    return 0;
}

static void Decode(AVCodecContext *ctx,
                   const XAVPacket_sptr &pkt,
                   XAVFrame_sptr &frame,
                   int64_t &begin,
                   int &count,
                   XVideoView *view){
    static bool is_view_init{};

    /**
     * 发送未压缩的包到解码器
     */
    auto ret{avcodec_send_packet(ctx,pkt.get())};
    if (0 != ret || AVERROR(EAGAIN) != ret) {
        FF_ERR_OUT(ret, return);
    }

    while (true) {
        /**
         * 读取解码后的帧
         */
        if (avcodec_receive_frame(ctx,frame.get()) < 0) {
            return;
        }

        ++count;
//        cerr << av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)) <<
//                " width: " << frame->width << " height: " << frame->height << "\n";

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
            view->Init(frame->width,frame->height,static_cast<XVideoView::Format>(frame->format));
        }

        /**
         * 显示一帧画面
         */
        view->DrawFrame(frame);
#if MACOS
        if (view->Is_Exit_Window()){
            return;
        }
#endif
    }
}
