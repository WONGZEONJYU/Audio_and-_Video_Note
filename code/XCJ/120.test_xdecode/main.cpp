extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "xdecode.hpp"
#include "xencode.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"
#include "xhelper.hpp"
#include "xvideo_view.hpp"
#include "xswscontext.hpp"

using namespace std;
#if 0
int main(const int argc,const char *argv[]) {

    if (argc < 2) {
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
    //de.InitHw();
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
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);

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

#else
//作业要求:从1920x1080 -> 800x600
int main(const int argc,const char *argv[]) {

    if (argc < 4) {
        PRINT_ERR_TIPS(GET_STR(argv miss!\n));
        return -1;
    }

    ifstream ifs(argv[1],ios::binary);
    ofstream ofs(argv[2],ios::binary);
    AVCodecParserContext *parser_ctx{};
    XVideoView* view{};

    const Destroyer d([&]{
        cerr << "Destroyer\n";
        av_parser_close(parser_ctx);
        ifs.close();
        ofs.close();
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

    if (!ofs){
        PRINT_ERR_TIPS(GET_STR(outfile open failed!));
    }

    const auto codec_id{AV_CODEC_ID_H264};

    XDecode de;
    auto c{XCodec::Create(codec_id,false)};
    de.set_codec_ctx(c);
    //de.InitHw();
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
    XAVFrame_sptr frame1;
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);
    TRY_CATCH(CHECK_EXC(frame1 = new_XAVFrame()),return -1);

    int count{};
    auto begin{XVideoView::Get_time_ms()};
    bool is_view_init{};

    auto sws{newXSwsContext()};
    constexpr auto d_w{800},d_h{600};

    frame1->width = d_w;
    frame1->height = d_h;
    frame1->format = c->pix_fmt;
    frame1->Get_Buffer(1);

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
                    if (!is_view_init) {
                        is_view_init = true;
                        view->Init(frame->width,frame->height,static_cast<XVideoView::Format>(frame->format));
                    }
                    view->DrawFrame(frame);
                    sws->reinit(frame->width,frame->height,frame->format,d_w,d_h,frame->format);
                    sws->scale(frame->data,frame->linesize,0,frame->height,frame1->data,frame1->linesize);

                    for (int i {}; i < frame1->height; ++i) {
                        const auto src {frame1->data[0] + frame1->linesize[0] * i};
                        ofs.write(reinterpret_cast<char*>(src),frame1->width);
                    }

                    for (int i {}; i < frame1->height / 2; ++i) {
                        const auto src{frame1->data[1] + frame1->linesize[1] * i};
                        ofs.write(reinterpret_cast<char*>(src),frame1->width / 2);
                    }

                    for (int i {}; i < frame1->height / 2; ++i) {
                        const auto src{frame1->data[2] + frame1->linesize[2] * i};
                        ofs.write(reinterpret_cast<char*>(src),frame1->width / 2);
                    }

                    ++count;
                    const auto curr_time{XVideoView::Get_time_ms()};
                    if (curr_time - begin >= 10LL) {
                        //std::cerr << "fps :" << count * 100<< "\n";
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

                sws->reinit(frame->width,frame->height,frame->format,d_w,d_h,frame->format);
                sws->scale(frame->data,frame->linesize,0,frame->height,frame1->data,frame1->linesize);

                for (int i {}; i < frame1->height; ++i) {
                    const auto src {frame1->data[0] + frame1->linesize[0] * i};
                    ofs.write(reinterpret_cast<char*>(src),frame1->width);
                }

                for (int i {}; i < frame1->height / 2; ++i) {
                    const auto src{frame1->data[1] + frame1->linesize[1] * i};
                    ofs.write(reinterpret_cast<char*>(src),frame1->width / 2);
                }

                for (int i {}; i < frame1->height / 2; ++i) {
                    const auto src{frame1->data[2] + frame1->linesize[2] * i};
                    ofs.write(reinterpret_cast<char*>(src),frame1->width / 2);
                }

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

            sws->reinit(item->width,item->height,item->format,d_w,d_h,item->format);
            cerr << sws->scale(item->data,item->linesize,0,item->height,frame1->data,frame1->linesize) << "\n";

            for (int i {}; i < frame1->height; ++i) {
                const auto src {frame1->data[0] + frame1->linesize[0] * i};
                ofs.write(reinterpret_cast<char*>(src),frame1->width);
            }

            for (int i {}; i < frame1->height / 2; ++i) {
                const auto src{frame1->data[1] + frame1->linesize[1] * i};
                ofs.write(reinterpret_cast<char*>(src),frame1->width / 2);
            }

            for (int i {}; i < frame1->height / 2; ++i) {
                const auto src{frame1->data[2] + frame1->linesize[2] * i};
                ofs.write(reinterpret_cast<char*>(src),frame1->width / 2);
            }
        }
        std::cerr << "\n\nparser and decode success!\n";
    }

    ifs.close();
    ofs.close();
    ifs.open(argv[2],ios::binary);
    ofs.open(argv[3],ios::binary);

    XEncode en;
    c = XEncode::Create(codec_id,true);
    en.set_codec_ctx(c);
    c->gop_size = 25;
    c->width = d_w;
    c->height = d_h;
    en.Set_Preset(veryfast_);
    en.Open();
    frame.reset();
    frame = en.CreateFrame();
    const auto one_frame_size{av_image_get_buffer_size(static_cast<AVPixelFormat>(frame->format),
                                                        frame->width,frame->height,1)};
    pmr::vector<uint8_t> read_buffer(one_frame_size * 2,0);

    auto p_rb{read_buffer.data()};
    int64_t pts{};
    packet.reset();
    while (!ifs.eof()){
        ifs.read(reinterpret_cast<char*>(p_rb),one_frame_size);
        frame->Make_Writable();
        frame->Image_Fill_Arrays(p_rb,frame->format,frame->width,frame->height,1);
        frame->pts = pts++;
        packet = en.Encode(frame);
        if (packet && packet->size){
            ofs.write(reinterpret_cast<char*>(packet->data),packet->size);
        }
    }

    auto packets{en.Flush()};
    for(auto &item: packets){
        if (item && item->size){
            ofs.write(reinterpret_cast<char*>(item->data),item->size);
        }
    }

    cerr << "encode finish\n";
    return 0;
}
#endif
