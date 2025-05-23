extern "C"{
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/opt.h>
}

#include <iostream>
#include <thread>
#include <fstream>
#include "XAVFrame.hpp"
#include "XAVPacket.hpp"
#include "XHelper.hpp"
#include "Encode_Params.hpp"

static void write_file(std::ofstream &ofs,const XAVPacket_sptr &packet);

int main(const int argc,const char *argv[]) {

    AVCodecID codecId {AV_CODEC_ID_HEVC};
    std::string file_name("800x600_25_preset.");
    //std::string file_name("800x600_25.");
    if (argc >= 2) {
        const std::string argv_1(argv[1]);
        if (argv_1 == "h265" || argv_1 == "hevc") {
            codecId = AV_CODEC_ID_HEVC;
            file_name += argv_1;
        } else if (argv_1 == "h264"){
            codecId = AV_CODEC_ID_H264;
            file_name += argv_1;
        }else{
            file_name += "h265";
            PRINT_ERR_TIPS(GET_STR(parameters error Use default parameters h265));
        }
    }else{
        file_name += "h265";
        PRINT_ERR_TIPS(GET_STR(no parameters Use default parameters h265));
    }

    XAVFrame_sptr frame;
    XAVPacket_sptr packet;
    AVCodecContext *codec_ctx{};
    std::ofstream ofs(file_name,std::ios::binary | std::ios::trunc);

    const Destroyer d([&]{
        avcodec_free_context(&codec_ctx);
        frame.reset();
        packet.reset();
        ofs.close();
    });

    if (!ofs) {
        std::cerr << GET_STR(FILE_ open failed!\n);
        return -1;
    }

    //1.查找编码器
    auto codec{avcodec_find_encoder(codecId)};

    if (!codec) {
        std::cerr << "codec not found!\n";
        return -1;
    }

    //2.分配解码器上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        std::cerr << "codec_ctx is empty\n";
        return -1;
    }

    //3.设置参数基本参数,必须设置
    codec_ctx->width = 800;
    codec_ctx->height = 600;
    codec_ctx->time_base = {1,25};
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->thread_count = static_cast<int>(std::thread::hardware_concurrency());

#if 0
    //设置B帧数量
    codec_ctx->max_b_frames = 0;

    //预设编码器参数
    FF_ERR_OUT(av_opt_set(codec_ctx->priv_data,preset_ultrafast.first,preset_ultrafast.second,0)); //最快速度编码
    FF_ERR_OUT(av_opt_set(codec_ctx->priv_data,tune_zerolatency.first,tune_zerolatency.second,0)); //H265零延时不支持B-Frame
#endif

#if 0
    //ABR平均比特率
    {
        auto br{400000};//400kb
        codec_ctx->bit_rate = br;
    }
#endif

#if 0
    //CQP恒定质量值(0~51)
    //H265 默认是28 效果好是23
    //H264 默认是23 效果好是18
    FF_ERR_OUT(av_opt_set_int(codec_ctx->priv_data, QP::m_name,QP(18),0));
#endif

#if 0
    //恒定比特率(CBR) 由于MP4不支持NAL填充,因此输出文件必须为(MPEG-2 TS)
    {
        auto br{400000};//400kb
        codec_ctx->rc_max_rate = br;
        codec_ctx->rc_min_rate = br;
        codec_ctx->rc_buffer_size = br;
        codec_ctx->bit_rate = br;
        FF_ERR_OUT(av_opt_set(codec_ctx->priv_data,cbr.first,cbr.second,0));
    }
#endif

#if 0
    //恒定速率因子(CRF)
    FF_ERR_OUT(av_opt_set_int(codec_ctx->priv_data,CRF::m_name,CRF(23),0));

    //约束编码和恒定速率因子是配合使用的
    //约束编码(VBV) Constrained Encoding(VBV)
    {
        FF_ERR_OUT(av_opt_set_int(codec_ctx->priv_data,CRF::m_name,CRF(23),0));
        auto br{400000};//400kb
        codec_ctx->rc_max_rate = br;
        codec_ctx->rc_buffer_size = br * 2;
    }
#endif

#if 1
    //图像组,一组包含一个IDR SPS PPS 其它是P帧和B帧(B帧需要设置才会有)
    codec_ctx->gop_size = 6;

#endif

    //4.打开编码器
    FF_ERR_OUT(avcodec_open2(codec_ctx,codec,{}),return -1);

    //5.创建AVFrame对象并分配AVFrame存储空间
    TRY_CATCH(CHECK_EXC(frame = new_XAVFrame()),return -1);
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    frame->format = codec_ctx->pix_fmt;
    FF_ERR_OUT(frame->Get_Buffer(),return -1);

    //6.创建AVPacket对象
    TRY_CATCH(CHECK_EXC(packet = new_XAVPacket()),return -1);
    //生成YUV数据
    for(int i{};i < 250;++i) {
        while (true) {
            //从编码器读取压缩后的包
            const auto ret{avcodec_receive_packet(codec_ctx,packet.get())};
            if (AVERROR_EOF == ret){
                FF_ERR_OUT(ret);
                break;
            } else if (AVERROR(EAGAIN) == ret) {
                break;
            } else if (ret < 0) {
                FF_ERR_OUT(ret);
                return -1;
            } else {
                write_file(ofs,packet);
            }
        }

        //Y
        for (int y{}; y < codec_ctx->height; ++y) {
            for (int x{}; x < codec_ctx->width; ++x) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }
        //UV
        for (int y {}; y < codec_ctx->height / 2; ++y) {
            for (int x {}; x < codec_ctx->width / 2; ++x) {
                frame->data[1][ y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][ y * frame->linesize[2] + x] = 64 + y + i * 5;
            }
        }

        frame->pts = i;
        //发送未压缩的帧进入编码器
        const auto ret{avcodec_send_frame(codec_ctx,frame.get())};
        if (AVERROR_EOF == ret) {
            FF_ERR_OUT(ret);
            break;
        }else if (AVERROR(EAGAIN) == ret) {
            continue;
        } else if(ret < 0) {
            FF_ERR_OUT(ret);
            return -1;
        } else{}
    }

    //发送空包,冲刷编码器
    FF_ERR_OUT(avcodec_send_frame(codec_ctx,{}));

    while (true) {
        //同上
        const auto ret{avcodec_receive_packet(codec_ctx, packet.get())};
        if (AVERROR_EOF == ret) {
            FF_ERR_OUT(ret);
            break;
        } else if (AVERROR(EAGAIN) == ret) {
            break;
        } else if (ret < 0){
            FF_ERR_OUT(ret);
            return -1;
        } else{
            write_file(ofs,packet);
        }
    }

    std::cerr << "\nencode success!\n\n";
    return 0;
}

static void write_file(std::ofstream &ofs,const XAVPacket_sptr &packet) {

    //std::cerr << "flush: " << packet.get()->size << "\t";
    //分析NALU
    //一个AVPacket中包含多个NALU 以0x00000001间隔,多个是以0x000001间隔
    // 0x00000001[NAL_HEAD],0x00000001是start code
    //[NAL_HEAD]
    //一个字节 orbidden_bit(1bit),nal_reference_bit(2bit)(优先级),nal_unit_type(5bit)
    auto nal_head{*(packet->data + 4)}; //去掉 00 00 00 01
    auto nal_unit_type{nal_head & 0x1f}; //取后5bit 000 11111
    std::cerr << GET_STR(nal_unit_type :) << nal_unit_type << "\n";

    for (int i {4}; i < packet->size - 4; ++i) { //一个data中有多条nalu
        const auto b_{!packet->data[i] && !packet->data[i + 1] && 0x01 == packet->data[i + 2]};
        if (b_) {
            nal_unit_type = packet->data[i + 3] & 0x1f;
            std::cerr << "(" << nal_unit_type << ")" << " ";
        }
    }
    ofs.write(reinterpret_cast<const char *>(packet->data),packet->size);
}
