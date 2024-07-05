//
// Created by Administrator on 2024/7/4.
//

#include "VideoDecoder.hpp"
#include "FFPlay.hpp"
#include "ShareAVFrame.hpp"
#include "AVHelper.h"

VideoDecoder::VideoDecoder(DecoderAbstract::Cv_Any_Type &cv, PacketQueue &q, AVCodecContext &av_codec_ctx) :
                            DecoderAbstract(cv,q,av_codec_ctx){

}

int VideoDecoder::get_video_frame(AVFrame *frame) {

    int got_picture{};

    if ((got_picture = decode_frame(frame)) < 0){
        return -1;
    }

    if (got_picture){
//        double dpts = NAN;
//
//        if (frame->pts != AV_NOPTS_VALUE){
//            //pts不为无效值,用视频流的时间基准计算pts的时间
//            dpts = av_q2d(is->video_st->time_base) * frame->pts;
//            //计算出秒为单位的pts
//        }
//
//        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
//        //猜测缩放比例
//
//        if (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) { // 允许drop帧
//            if (frame->pts != AV_NOPTS_VALUE) { // pts值有效
//                double diff = dpts - get_master_clock(is);
//                if (!isnan(diff) && // 差值有效
//                    fabs(diff) < AV_NOSYNC_THRESHOLD && //差值在可同步范围呢
//                    diff - is->frame_last_filter_delay < 0 && // 和过滤器有关系
//                    is->viddec.pkt_serial == is->vidclk.serial && // 同一序列的包
//                    is->videoq.nb_packets) { // packet队列至少有1帧数据
//                    is->frame_drops_early++;
//                    av_frame_unref(frame);
//                    got_picture = 0;
//                }
//            }
//        }
    }

    return got_picture;
}

int VideoDecoder::queue_picture(FrameQueue *fq, AVFrame *src_frame,const double &pts,const double &_duration,const int64_t &pos,const int &serial) {

    Frame *vp{};

    if (!(vp = frame_queue_peek_writable(fq))) { /*检查队列是否有空间可写,有空间则返回一个可写的自定义的Frame*/
        return -1;
    }

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = _duration;
//    vp->pos = pos;
//    vp->serial = serial;

    av_frame_move_ref(vp->frame, src_frame); //src_frame转移到vp->frame
    frame_queue_push(fq); //更新写索引

    return 0;
}

void VideoDecoder::av_decoder_thread(void *o) {

    std::cerr << __FUNCTION__ << "begin\n";
    auto obj{static_cast<FFPlay*>(o)};
    
    ShareAVFrame_sp_type frame;

    try {
        frame = new_ShareAVFrame();
        const auto tb {obj->f_video_st()->time_base};

        int64_t pos{};
        int serial{};

        while (true){

            auto ret {get_video_frame(*frame)};
            if (ret < 0){
                throw std::runtime_error(std::string(__FUNCTION__ ) + " " + std::to_string(__LINE__) + " " + AVHelper::av_get_err(ret) + "\n");
            } else if (!ret){
                continue;
            }

            const auto frame_rate (av_guess_frame_rate(obj->f_format_ctx(), obj->f_video_st(), *frame));
            //frame_rate是视频帧率,分子比分母大,比如 (num = 24)/(den = 1)
            const auto duration_{frame_rate.den != 0 && frame_rate.num != 0 ? av_q2d({frame_rate.den,frame_rate.num}):0};

            const auto pts_{static_cast<double >((*frame)->pts)};

            const auto pts{AV_NOPTS_VALUE == pts_? NAN : pts_ * av_q2d(tb)};

            ret = queue_picture(obj->f_pic_frame_q(),*frame,pts,duration_,pos,serial);
            if (ret < 0){
                throw std::runtime_error(std::string(__FUNCTION__ ) + " " + std::to_string(__LINE__) + " " + AVHelper::av_get_err(ret) + "\n");
            }
        }

    } catch (const std::exception &e) {
        frame.reset();
        std::cerr << e.what() << "\n";
    }

    std::cerr << __FUNCTION__ << "end\n";
}

VideoDecoder_sptr new_VideoDecoder(std::condition_variable_any &cv,PacketQueue &q,AVCodecContext &av_codec_ctx) noexcept(false)
{
    VideoDecoder_sptr obj;
    try {
        obj.reset(new VideoDecoder(cv,q,av_codec_ctx));
        return obj;
    } catch (const std::runtime_error &e) {
        obj.reset();
        throw e;
    } catch (const std::bad_alloc &e) {
        obj.reset();
        throw std::runtime_error("new VideoDecoder");
    }
}
