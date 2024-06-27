/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * simple media player based on the FFmpeg libraries
 */

#include "config.h"
#include "config_components.h"
#include <cmath>
#include <climits>
#include <csignal>
#include <cstdint>
extern "C" {
#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#include "libavutil/pixdesc.h"
#include "libavutil/dict.h"
#include "libavutil/fifo.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/bprint.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/tx.h"
#include "libswresample/swresample.h"

#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include <SDL.h>
#include <SDL_thread.h>
}

#include "cmdutils.h"
#include "ffplay_renderer.h"
#include "opt_common.h"

const char program_name[] = "ffplay";
const int program_birth_year = 2003;

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Step size for volume control in dB */
#define SDL_VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

typedef struct MyAVPacketList { /*PacketQueue队列存放的元素模型*/
    AVPacket *pkt;  /*解封装后的packet*/
    int serial;     /*播放序列,用于快进快退*/
} MyAVPacketList;

typedef struct PacketQueue { /*包队列*/
    AVFifo *pkt_list; /*FFmpeg 的avfifo */
    int nb_packets; /*包数量*/
    int size;   //队列所有元素的大小(队列每一个元素size计算方式sizeAVPacket.size + sizeof(MyAVPacketList)) , 不是包数量
    // size = (sizeAVPacket.size + sizeof(MyAVPacketList)) * nb_packets
    int64_t duration; /*队列所有元素的数据播放持续时间(队列每一个元素duration的AVPacket.duration) */
    // duration = AVPacket.duration * nb_packets
    int abort_request; // 用户退出请求标志
    int serial; // 播放序列号，和MyAVPacketList的serial作用相同,但改变的时序稍微有点不同
    SDL_mutex *mutex; // 用于维持PacketQueue的多线程安全(SDL_mutex可以按pthread_mutex_t理解）
    SDL_cond *cond; // 用于读、写线程相互通知(SDL_cond可以按pthread_cond_t理解)
} PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 3 // 图像帧缓存数量
#define SUBPICTURE_QUEUE_SIZE 16 // 字幕帧缓存数量
#define SAMPLE_QUEUE_SIZE 9 // 采样帧缓存数量
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct AudioParams {
    int freq;                   //采样率
    AVChannelLayout ch_layout; //通道布局
    enum AVSampleFormat fmt;    //采样格式
    int frame_size;             //一个采样单元占用的字节数(比如2通道时,则左右通道各采样一次合成一个采样单元)
    int bytes_per_sec;          //一秒时间的字节数,比如采样率48Khz,2 channel,16bit,则一秒48000*2*16/8=192000
} AudioParams;//音频硬件参数

// 这里讲的系统时钟 是通过av_gettime_relative()获取到的时钟,单位为微妙
typedef struct Clock {
    // 时钟基础, 当前帧(待播放)显示时间戳,播放后,当前帧变成上一帧
    double pts;           /* clock base */
    // 当前pts与当前系统时钟的差值,audio、video对于该值是独立的
    double pts_drift;     /* clock base minus time at which we updated the clock */
    // 当前时钟(如视频时钟)最后一次更新时间,也可称当前时钟时间
    double last_updated; // 最后一次更新的系统时钟
    double speed;   // 时钟速度控制,用于控制播放速度
    //播放序列,所谓播放序列就是一段连续的播放动作,一个seek操作会启动一段新的播放序列
    int serial;          /* clock is based on a packet with this serial */
    int paused;         // = 1 说明是暂停状态
    // 指向packet_serial
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

typedef struct FrameData {
    int64_t pkt_pos;
} FrameData;

/* Common struct for handling all types of decoded data and allocated render buffers. */
// 用于缓存解码后的数据
typedef struct Frame {
    AVFrame *frame;     // 指向数据帧
    AVSubtitle sub;     // 用于字幕
    int serial;         // 帧序列，在seek的操作时serial会变化
    double pts;           /* presentation timestamp for the frame */ // 时间戳,单位为秒
    double duration;      /* estimated duration of the frame */     // 该帧持续时间,单位为秒
    int64_t pos;          /* byte position of the frame in the input file */    // 该帧在输入文件中的字节位置
    int width;      // 图像宽度
    int height;     // 图像高读
    int format;     // 对于图像为(enum AVPixelFormat)
    // 对于声音则为(enum AVSampleFormat)
    AVRational sar;     // 图像的宽高比（16:9，4:3...）,如果未知或未指定则为0/1
    int uploaded;       // 用来记录该帧是否已经显示过？
    int flip_v;         // =1则垂直翻转, 0则正常播放
} Frame;

/* 这是一个循环队列,windex是指其中的首元素,rindex是指其中的尾部元素. */
typedef struct FrameQueue {
    Frame queue[FRAME_QUEUE_SIZE];// FRAME_QUEUE_SIZE  最大size, 数字太大时会占用大量的内存，需要注意该值的设置
    int rindex;                 // 读索引。待播放时读取此帧进行播放，播放后此帧成为上一帧
    int windex;                 //写索引
    int size;                   // 当前总帧数
    int max_size;               // 可存储最大帧数
    int keep_last;              // = 1说明要在队列里面保持最后一帧的数据不释放,只在销毁队列的时候才将其真正释放
    int rindex_shown;           // 初始化为0,配合keep_last=1使用
    SDL_mutex *mutex;           // 互斥量
    SDL_cond *cond;             // 条件变量
    PacketQueue *pktq;          // 数据包缓冲队列
} FrameQueue;

/**
 *音视频同步方式，缺省以音频为基准
 */
enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */ // 以音频为基准
    AV_SYNC_VIDEO_MASTER,   // 以视频为基准
    AV_SYNC_EXTERNAL_CLOCK, // 以外部时钟为基准,synchronize to an external clock
};

/**
 * 解码器封装
 */
typedef struct Decoder {
    AVPacket *pkt;
    PacketQueue *queue;          // 数据包队列
    AVCodecContext *avctx;       // 解码器上下文
    int pkt_serial;             // 包序列
    int finished;               // =0,解码器处于工作状态; =!0,解码器处于空闲状态
    int packet_pending;         // =0,解码器处于异常状态,需要考虑重置解码器; =1,解码器处于正常状态
    SDL_cond *empty_queue_cond; // 检查到packet队列空时发送 signal缓存read_thread读取数据
    int64_t start_pts;          // 初始化时是stream的start time
    AVRational start_pts_tb;    // 初始化时是stream的time_base
    int64_t next_pts;           // 记录最近一次解码后的frame的pts,当解出来的部分帧没有有效的pts时则使用next_pts进行推算
    AVRational next_pts_tb;     // next_pts的单位
    SDL_Thread *decoder_tid;    // 线程句柄
} Decoder;

typedef struct VideoState {
    SDL_Thread *read_tid;           // 读线程句柄
    const AVInputFormat *iformat;   // 指向demuxer
    int abort_request;              // =1时请求退出播放
    int force_refresh;              // =1时需要刷新画面,请求立即刷新画面的意思
    int paused;                     // =1时暂停,=0时播放
    int last_paused;                // 暂存 "暂停" / "播放" 状态
    int queue_attachments_req;      //请求封面
    int seek_req;                   // 标识一次seek请求
    int seek_flags;                 // seek标志,诸如AVSEEK_FLAG_BYTE等
    int64_t seek_pos;               // 请求seek的目标位置(当前位置+增量)
    int64_t seek_rel;               // 本次seek的位置增量
    int read_pause_return;          // 网络流暂停标志
    AVFormatContext *ic;            // iformat的上下文
    int realtime;                   // =1为实时流

    Clock audclk;                   // 音频时钟
    Clock vidclk;                   // 视频时钟
    Clock extclk;                   // 外部时钟

    FrameQueue pictq;               // 视频Frame队列
    FrameQueue subpq;               // 字幕Frame队列
    FrameQueue sampq;               // 采样Frame队列

    Decoder auddec;                 // 音频解码器
    Decoder viddec;                 // 视频解码器
    Decoder subdec;                 // 字幕解码器

    int audio_stream;               // 音频流索引

    int av_sync_type;               // 音视频同步类型,默认audio master

    double audio_clock;             // 当前音频帧的PTS+当前帧Duration
    int audio_clock_serial;         // 播放序列,seek可改变此值
    // 以下4个参数 非audio master同步方式使用
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    // 以上4个参数 非audio master同步方式使用
    AVStream *audio_st;             // 音频流
    PacketQueue audioq;             // 音频packet队列
    int audio_hw_buf_size;          // SDL音频缓冲区的大小(字节为单位)
    // 指向待播放的一帧音频数据,指向的数据区将被拷入SDL音频缓冲区,若经过重采样则指向audio_buf1,否则指向frame中的音频
    uint8_t *audio_buf;             // 指向需要重采样的数据
    uint8_t *audio_buf1;            // 指向重采样后的数据
    unsigned int audio_buf_size; /* in bytes */ // 待播放的一帧音频数据(audio_buf指向)的大小
    unsigned int audio_buf1_size;   // 申请到的音频缓冲区audio_buf1的实际尺寸
    int audio_buf_index; /* in bytes */
    // 更新拷贝位置 当前音频帧中已拷入SDL音频缓冲区的位置索引(指向第一个待拷贝字节)
    // 当前音频帧中尚未拷入SDL音频缓冲区的数据量:
    // audio_buf_size = audio_buf_index + audio_write_buf_size
    int audio_write_buf_size;       //剩余还没写入SDL_buf的数据大小
    int audio_volume;               // 音量
    int muted;                      // =1静音，=0则正常
    struct AudioParams audio_src;   // 音频frame的参数
    struct AudioParams audio_filter_src;
    struct AudioParams audio_tgt;   // SDL支持的音频参数，重采样转换：audio_src->audio_tgt
    struct SwrContext *swr_ctx;     // 音频重采样context
    int frame_drops_early;          // 丢弃视频packet计数
    int frame_drops_late;           // 丢弃视频frame计数

    enum ShowMode {
        SHOW_MODE_NONE = -1,    // 无显示
        SHOW_MODE_VIDEO = 0,    // 显示视频
        SHOW_MODE_WAVES,        // 显示波浪,音频
        SHOW_MODE_RDFT,         // 自适应滤波器
        SHOW_MODE_NB
    }show_mode;

    // 音频波形显示使用
    int16_t sample_array[SAMPLE_ARRAY_SIZE];    // 采样数组
    int sample_array_index;                     // 采样索引
    int last_i_start;                           // 上一开始
    AVTXContext *rdft;                          // 自适应滤波器上下文
    av_tx_fn rdft_fn;
    int rdft_bits;                              // 自使用比特率
    float *real_data;                           // 实时数据
    AVComplexFloat *rdft_data;                  // 快速傅里叶采样
    int xpos;
    double last_vis_time;
    SDL_Texture *vis_texture;                   // 音频Texture
    SDL_Texture *sub_texture;                   // 字幕显示
    SDL_Texture *vid_texture;                   // 视频显示

    int subtitle_stream;                        // 字幕流索引
    AVStream *subtitle_st;                      // 字幕流
    PacketQueue subtitleq;                      // 字幕packet队列

    double frame_timer;                         // 记录最后一帧播放的时刻
    double frame_last_returned_time;            // 上一次返回时间
    double frame_last_filter_delay;             // 上一个过滤器延时
    int video_stream;                           // 视频流索引
    AVStream *video_st;                         // 视频流
    PacketQueue videoq;                         // 视频队列
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    // 一帧最大间隔. above this, we consider the jump a timestamp discontinuity
    struct SwsContext *sub_convert_ctx;     // 字幕尺寸格式变换
    int eof;                                 // 是否读取结束

    char *filename;                         // 文件名
    int width, height, xleft, ytop;         // 宽、高，x起始坐标,y起始坐标
    int step;                               // =1 步进播放模式, =0 其他模式

    int vfilter_idx;
    AVFilterContext *in_video_filter;   // the first filter in the video chain
    AVFilterContext *out_video_filter;  // the last filter in the video chain
    AVFilterContext *in_audio_filter;   // the first filter in the audio chain
    AVFilterContext *out_audio_filter;  // the last filter in the audio chain
    AVFilterGraph *agraph;              // audio filter graph
    // 保留最近的相应audio、video、subtitle流的steam index
    int last_video_stream, last_audio_stream, last_subtitle_stream;

    SDL_cond *continue_read_thread; // 当读取数据队列满了后进入休眠时,可以通过该condition唤醒读线程
} VideoState;

/* options specified by the user */
static const AVInputFormat *file_iformat;
static const char *input_filename;
static const char *window_title;
static int default_width  = 640;
static int default_height = 480;
static int screen_width  = 0;
static int screen_height = 0;
static int screen_left = SDL_WINDOWPOS_CENTERED;    // 显示视频窗口的x坐标,默认在居中
static int screen_top = SDL_WINDOWPOS_CENTERED;     // 显示视频窗口的y坐标,默认居中
static int audio_disable;
static int video_disable;
static int subtitle_disable;
static const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
static int seek_by_bytes = -1;
static float seek_interval = 10;     // 可以指定seek的间隔
static int display_disable;
static int borderless;
static int alwaysontop; // 是否顶置
static int startup_volume = 100;    // 起始音量
static int show_status = -1;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = AV_NOPTS_VALUE;
static int64_t duration = AV_NOPTS_VALUE;
static int fast = 0;
static int genpts = 0;
static int lowres = 0;
static int decoder_reorder_pts = -1;
static int autoexit;
static int exit_on_keydown;
static int exit_on_mousedown;
static int loop = 1;            // 设置循环次数
static int framedrop = -1;
static int infinite_buffer = -1;
static VideoState::ShowMode show_mode = VideoState::SHOW_MODE_NONE;
static const char *audio_codec_name;
static const char *subtitle_codec_name;
static const char *video_codec_name;
static double rdftspeed = 0.02;
static int64_t cursor_last_shown;
static int cursor_hidden = 0;
static const char **vfilters_list = NULL;
static int nb_vfilters = 0;
static char *afilters = NULL;
static int autorotate = 1;
static int find_stream_info = 1;
static int filter_nbthreads = 0;    // filter线程数量
static int enable_vulkan = 0;
static char *vulkan_params = NULL;
static const char *hwaccel = NULL;

/* current context */
static int is_full_screen;
static int64_t audio_callback_time;

#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_RendererInfo renderer_info = {0};
static SDL_AudioDeviceID audio_dev;

static VkRenderer *vk_renderer;

static const struct TextureFormatEntry {
    enum AVPixelFormat format;
    int texture_fmt;
} sdl_texture_format_map[] = {  // FFmpeg PIX_FMT to SDL_PIX的映射关系
    { AV_PIX_FMT_RGB8,           SDL_PIXELFORMAT_RGB332 },
    { AV_PIX_FMT_RGB444,         SDL_PIXELFORMAT_RGB444 },
    { AV_PIX_FMT_RGB555,         SDL_PIXELFORMAT_RGB555 },
    { AV_PIX_FMT_BGR555,         SDL_PIXELFORMAT_BGR555 },
    { AV_PIX_FMT_RGB565,         SDL_PIXELFORMAT_RGB565 },
    { AV_PIX_FMT_BGR565,         SDL_PIXELFORMAT_BGR565 },
    { AV_PIX_FMT_RGB24,          SDL_PIXELFORMAT_RGB24 },
    { AV_PIX_FMT_BGR24,          SDL_PIXELFORMAT_BGR24 },
    { AV_PIX_FMT_0RGB32,         SDL_PIXELFORMAT_RGB888 },
    { AV_PIX_FMT_0BGR32,         SDL_PIXELFORMAT_BGR888 },
    { AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888 },
    { AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888 },
    { AV_PIX_FMT_RGB32,          SDL_PIXELFORMAT_ARGB8888 },
    { AV_PIX_FMT_RGB32_1,        SDL_PIXELFORMAT_RGBA8888 },
    { AV_PIX_FMT_BGR32,          SDL_PIXELFORMAT_ABGR8888 },
    { AV_PIX_FMT_BGR32_1,        SDL_PIXELFORMAT_BGRA8888 },
    { AV_PIX_FMT_YUV420P,        SDL_PIXELFORMAT_IYUV },
    { AV_PIX_FMT_YUYV422,        SDL_PIXELFORMAT_YUY2 },
    { AV_PIX_FMT_UYVY422,        SDL_PIXELFORMAT_UYVY },
    { AV_PIX_FMT_NONE,           SDL_PIXELFORMAT_UNKNOWN },
};

static int opt_add_vfilter(void *optctx, const char *opt, const char *arg)
{
    int ret = GROW_ARRAY(vfilters_list, nb_vfilters);
    if (ret < 0){
        return ret;
    }

    vfilters_list[nb_vfilters - 1] = av_strdup(arg);
    if (!vfilters_list[nb_vfilters - 1]) {
        return AVERROR(ENOMEM);
    }

    return 0;
}

/*音频采样格式比较函数*/
static inline int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
                   enum AVSampleFormat fmt2, int64_t channel_count2)
{
    /* If channel count == 1, planar and non-planar formats are the same */
    if (channel_count1 == 1 && channel_count2 == 1) {
        return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
    }else{
        return channel_count1 != channel_count2 || fmt1 != fmt2;
    }
}

/*入队的实际操作*/
static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
    MyAVPacketList pkt1;
    int ret;

    if (q->abort_request){ //如果已中止,则放入失败
        return -1;
    }

    pkt1.pkt = pkt;             //指针拷贝,老版本是数据拷贝
    pkt1.serial = q->serial;    //用队列序列号标记节点

    ret = av_fifo_write(q->pkt_list, &pkt1, 1); //入队操作
    if (ret < 0){
        return ret;
    }

    q->nb_packets++;        //packet数量+1
    q->size += pkt1.pkt->size + sizeof(pkt1);       //增加一个队列元素大小
    q->duration += pkt1.pkt->duration;  /*增加一个队列元素的数据播放持续时间*/
    /* XXX: should duplicate packet data in DV case */
    SDL_CondSignal(q->cond);/*条件变量,通知读线程有数据可读*/
    return 0;
}

/*入队函数*/
static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacket *pkt1;
    int ret;

    pkt1 = av_packet_alloc();
    if (!pkt1) {
        av_packet_unref(pkt);
        return -1;
    }
    av_packet_move_ref(pkt1, pkt); /*转移AVPacket的引用计数*/

    SDL_LockMutex(q->mutex);
    ret = packet_queue_put_private(q, pkt1);
    SDL_UnlockMutex(q->mutex);

    if (ret < 0) {
        av_packet_free(&pkt1);
    }
    return ret;
}

/*packet_queue放入空包*/
static int packet_queue_put_nullpacket(PacketQueue *q, AVPacket *pkt, int stream_index)
{
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

/* packet queue handling */
/*队列初始化*/
static int packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->pkt_list = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
    if (!q->pkt_list){
        return AVERROR(ENOMEM);
    }

    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->cond = SDL_CreateCond();
    if (!q->cond) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    q->abort_request = 1;
    return 0;
}

/*清空packet_queue*/
static void packet_queue_flush(PacketQueue *q)
{
    MyAVPacketList pkt1;

    SDL_LockMutex(q->mutex);
    while (av_fifo_read(q->pkt_list, &pkt1, 1) >= 0){
        av_packet_free(&pkt1.pkt);  //释放队列存的所有元素
    }

    q->nb_packets = 0;
    q->size = 0;
    q->duration = 0;
    q->serial++;
    SDL_UnlockMutex(q->mutex);
}

/*销毁packet_queue*/
static void packet_queue_destroy(PacketQueue *q)
{
    packet_queue_flush(q);
    av_fifo_freep2(&q->pkt_list);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

/*packet_queue请求退出*/
static void packet_queue_abort(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;//请求退出
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

/*packet_queue启动*/
static void packet_queue_start(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    q->serial++;
    SDL_UnlockMutex(q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
/*packet_queue读取函数*/
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial)
{
    MyAVPacketList pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        if (av_fifo_read(q->pkt_list, &pkt1, 1) >= 0) {
            q->nb_packets--;
            q->size -= pkt1.pkt->size + sizeof(pkt1);
            q->duration -= pkt1.pkt->duration;
            av_packet_move_ref(pkt, pkt1.pkt); /*转移AVPacket的引用计数*/
            if (serial){
                *serial = pkt1.serial;
            }

            av_packet_free(&pkt1.pkt); /*由于pkt1.pkt已经被转移,这里的释放不会导致出队的数据被销毁*/
            ret = 1;
            break;
        } else if (!block) { //block为非0值 , 走else分支,进入阻塞等待
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

/*解码器初始化*/
static int decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond) {
    memset(d, 0, sizeof(Decoder));
    d->pkt = av_packet_alloc();
    if (!d->pkt){
        return AVERROR(ENOMEM);
    }
    d->avctx = avctx; //解码器上下文
    d->queue = queue; //绑定对应的packet queue
    d->empty_queue_cond = empty_queue_cond; // 绑定read_thread线程的continue_read_thread
    d->start_pts = AV_NOPTS_VALUE;  // 起始设置为无效
    d->pkt_serial = -1; // 起始设置为-1
    return 0;
}

/**
 * 解码,音视频共用一个函数
 * @param d
 * @param frame
 * @param sub
 * @return 1代表成功,0代表解码结束,负数代表有错误
 */
static int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub) {

    int ret = AVERROR(EAGAIN);

    for (;;) {
        if (d->queue->serial == d->pkt_serial) {    //判断是否为同一播放序列
            do {
                if (d->queue->abort_request) { //是否请求退出
                    return -1;
                }

                switch (d->avctx->codec_type) { //选择视频还是音频
                    case AVMEDIA_TYPE_VIDEO:    //视频
                        ret = avcodec_receive_frame(d->avctx, frame);//第一次读取ret一定是AVERROR(EAGAIN) = -11
                        if (ret >= 0) { /*成功读取到数据*/
                            if (decoder_reorder_pts == -1) {    //让解码器重新排序pts 0=关闭 1=开启 -1=自动
                                frame->pts = frame->best_effort_timestamp; //默认采用启发法的结果作为pts,详情查看源码
                            } else if (!decoder_reorder_pts) {
                                frame->pts = frame->pkt_dts;    /*采用dts作为pts*/
                            }
                        }
                        break;
                    case AVMEDIA_TYPE_AUDIO:    //音频
                        ret = avcodec_receive_frame(d->avctx, frame); //第一次读取结果同上
                        if (ret >= 0) { /*成功读取到数据*/

                            AVRational tb = (AVRational){1, frame->sample_rate};

                            if (frame->pts != AV_NOPTS_VALUE) {
                                frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);  //把读取到pts从解码器的时间基准转换到以(1/sample_rate)的时间基准
                                //d->avctx->pkt_timebase实质是AVStream->timebase
                            }else if (d->next_pts != AV_NOPTS_VALUE) {
                                //如果frame->pts不正常则使用上一帧更新的next_pts和next_pts_tb去计算当前的pts
                                // 转成{1,frame->sample_rate}
                                frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                            }

                            if (frame->pts != AV_NOPTS_VALUE) {
                                // 根据当前帧的pts和nb_samples预估下一帧的pts
                                d->next_pts = frame->pts + frame->nb_samples;
                                d->next_pts_tb = tb; // 设置timebase
                            }
                        }
                        break;
                }

                if (ret == AVERROR_EOF) { //解码结束
                    d->finished = d->pkt_serial;    //标记已完成
                    avcodec_flush_buffers(d->avctx);    //刷新内部缓冲区
                    return 0;
                }

                if (ret >= 0){//正常解码成功
                    return 1;
                }
            } while (ret != AVERROR(EAGAIN));//没有帧可读,退出走下面流程送packet
        }

        //获取一个packet,如果播放序列不一致(数据不连续)则过滤掉"过时"的packet
        do {
            //如果没有数据可读则唤醒read_thread,实际是continue_read_thread SDL_cond
            if (d->queue->nb_packets == 0) { // 没有数据可读
                SDL_CondSignal(d->empty_queue_cond);// 通知read_thread读取packet
            }

            if (d->packet_pending) { //如果上一次的packet没有成功送入解码器,再次送入,不读取队列的packet
                d->packet_pending = 0;
            }else {    //从队列读取packet

                int old_serial = d->pkt_serial; //记录解码器的当前序列

                if (packet_queue_get(d->queue, d->pkt, 1, &d->pkt_serial) < 0) {//采用阻塞式读取,并更新解码器的序列
                    return -1;
                }

                if (old_serial != d->pkt_serial) {  //序列不一致
                    avcodec_flush_buffers(d->avctx);//重置编码器
                    d->finished = 0;
                    d->next_pts = d->start_pts; //重置最近一次的pts
                    d->next_pts_tb = d->start_pts_tb; //重置next_pts的时间基准
                }
            }

            if (d->queue->serial == d->pkt_serial) { //如果解码器的序列和队列的序列一致,则跳出当前循环,否则丢弃当前包,继续读取
                break;
            }

            av_packet_unref(d->pkt);
        } while (1);

        if (d->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) { //字幕类型
            int got_frame = 0;
            ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, d->pkt);  //解码字幕包,got_frame非0值是成功
            if (ret < 0) { //字幕解码失败
                ret = AVERROR(EAGAIN); //ret = AVERROR(EAGAIN),让当前循环再次读取字幕包队列拿一个新数据进行解码
            } else {    //字幕解码成功

                /*这里比较判断复杂
                 * 阅读了avcodec_decode_subtitle2源码,avcodec_decode_subtitle2返回值非负值,got_frame绝对不会是0值
                 * 1.got_frame为非零值,字幕包(pkt)为空,标记d->packet_pending,ret = 0,结束本函数(decoder_decode_frame),
                 * 返回解码成功,由于标记了d->packet_pending,下次进入本函数就是冲刷字幕解码器
                 *
                 * 2.got_frame为非零值,字幕包(pkt)数据不为空,ret = 0,结束本函数(decoder_decode_frame),
                 * 下次进入本函数继续从字幕包队列拿行数据进行解码
                 *
                 * */

                if (got_frame && !d->pkt->data) {
                    d->packet_pending = 1;
                }

                ret = got_frame ? 0 : (d->pkt->data ? AVERROR(EAGAIN) : AVERROR_EOF);//第二层判断似乎永远不会进入,不明白作者是什么意图?欢迎大神来回答一下?
            }

            av_packet_unref(d->pkt);//无论字幕是否解码成功,都把字幕包释放

        } else {
            if (d->pkt->buf && !d->pkt->opaque_ref) {
                //搜索整个工程,唯独在此处使用opaque_ref,不明白作者有何意图？
                FrameData *fd;

                d->pkt->opaque_ref = av_buffer_allocz(sizeof(*fd));

                if (!d->pkt->opaque_ref) {
                    return AVERROR(ENOMEM);//-12
                }

                fd = (FrameData*)d->pkt->opaque_ref->data;
                fd->pkt_pos = d->pkt->pos;
            }

            if (avcodec_send_packet(d->avctx, d->pkt) == AVERROR(EAGAIN)) { //发送packet包进行解码
                av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                d->packet_pending = 1;//如果上一包没有成功送入解码器,标记下次继续送该包
            } else {
                av_packet_unref(d->pkt);
            }
        }
    }
}

/*
 * 解码器销毁
 */
static void decoder_destroy(Decoder *d) {
    av_packet_free(&d->pkt);
    avcodec_free_context(&d->avctx);
}

/*
 * frame_queue队列item的引用释放
 */
static void frame_queue_unref_item(Frame *vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

/*
 * frame_queue初始化,音视频的keep_last设置为1,字幕的keep_last设置为0
 */
static int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last)
{
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = SDL_CreateMutex())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    if (!(f->cond = SDL_CreateCond())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);//最大不能超过16个元素 FRAME_QUEUE_SIZE=16
    f->keep_last = !!keep_last; //用于控制字段rindex_shown的生成,详情阅读frame_queue_next函数

    for (int i = 0; i < f->max_size; i++) {
        //给每个AVFrame分配空间
        if (!(f->queue[i].frame = av_frame_alloc())){
            return AVERROR(ENOMEM);
        }
    }

    return 0;
}

/*
 * 销毁frame_queue
 */
static void frame_queue_destroy(FrameQueue *f)
{
    for (int i = 0; i < f->max_size; i++) {
        Frame *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

/*
 * frame_queue条件变量发送函数,用于线程间通讯
 */
static void frame_queue_signal(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

/*
 * 窥探位置(f->rindex + f->rindex_shown)的数据
 * 如果f->rindex_shown为0,frame_queue_peek与frame_queue_peek_last读取的位置一样
 */
static Frame *frame_queue_peek(FrameQueue *f)
{
    //f->rindex_shown是0还是1,影响着改API是读取当前元素还是下一元素
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/*
 * 窥探位置(f->rindex + f->rindex_shown + 1)的数据
 */
static Frame *frame_queue_peek_next(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

/*
 * 窥探位置f->rindex的数据
 */
static Frame *frame_queue_peek_last(FrameQueue *f)
{
    return &f->queue[f->rindex];
}

/*
 * peek 出一个可以写的 Frame,此函数可能会阻塞
 */
static Frame *frame_queue_peek_writable(FrameQueue *f)
{
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);

    while (f->size >= f->max_size && !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }

    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request){
        return NULL;
    }

    return &f->queue[f->windex];
}

/*
 * peek 出一个可以准备播放的 Frame,此函数可能会阻塞
 */
static Frame *frame_queue_peek_readable(FrameQueue *f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request) {
        return NULL;
    }

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/*
 * 更新f->windex
 */
static void frame_queue_push(FrameQueue *f)
{
    if (++f->windex == f->max_size){
        f->windex = 0;
    }
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

/*
 * 先释放frame,并更新f->rindex
 * 首次调用,如果keep_last为1, rindex_show为0时不去更新f->rindex,也不释放当前frame
 * f->keep_last字段就是用来控制f->rindex_shown是否需要置1
 */
static void frame_queue_next(FrameQueue *f)
{
    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1; //f->rindex_shown被置1之后,就一直为1
        return;
    }

    frame_queue_unref_item(&f->queue[f->rindex]);//释放

    if (++f->rindex == f->max_size){
        f->rindex = 0;
    }

    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

/* return the number of undisplayed frames in the queue */
//返回队列中未显示的帧数
static int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;//为什么要减去f->rindex_shown,因为他是返回还未显示的帧而不是队列还有多少帧
    //f->rindex_shown的数量代表已经显示了多少帧
}

/* return last shown position */
//返回最后显示的位置
static int64_t frame_queue_last_pos(FrameQueue *f)
{
    Frame *fp = &f->queue[f->rindex];
    if (f->rindex_shown && fp->serial == f->pktq->serial) {
        return fp->pos;
    }else {
        return -1;
    }
}
/*
 * 解码器请求退出
 */
static void decoder_abort(Decoder *d, FrameQueue *fq)
{
    packet_queue_abort(d->queue);// 终止packet队列,packetQueue的abort_request被置为1
    frame_queue_signal(fq);// 唤醒Frame队列,以便退出
    SDL_WaitThread(d->decoder_tid, NULL);// 等待解码线程退出
    d->decoder_tid = NULL;// 线程ID重置
    packet_queue_flush(d->queue);//清空队列,释放所有数据
}

/*
 *绘制矩阵 x,y是起始坐标,w,h是矩阵宽高
 */
static inline void fill_rectangle(int x, int y, int w, int h)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    if (w && h) {
        SDL_RenderFillRect(renderer, &rect);
    }
}

/*
 * 分配纹理空间
 */
static int realloc_texture(SDL_Texture **texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture)
{
    Uint32 format;
    int access, w, h;
    if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format) {
        void *pixels;
        int pitch;
        if (*texture){
            SDL_DestroyTexture(*texture);
        }

        if (!(*texture = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height))){
            return -1;
        }

        if (SDL_SetTextureBlendMode(*texture, blendmode) < 0){
            return -1;
        }

        if (init_texture) {
            if (SDL_LockTexture(*texture, NULL, &pixels, &pitch) < 0){
                return -1;
            }

            memset(pixels, 0, pitch * new_height);
            SDL_UnlockTexture(*texture);
        }
        av_log(NULL, AV_LOG_VERBOSE, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
    }
    return 0;
}

/**
 * @brief 将帧宽高按照sar最大适配到窗口
 * @param rect      获取到的显示位置和宽高
 * @param scr_xleft 窗口显示起始x位置,这里说的是内部显示的坐标, 不是窗口在整个屏幕的起始位置
 * @param scr_ytop  窗口显示起始y位置
 * @param scr_width  窗口宽度
 * @param scr_height 窗口高度
 * @param pic_width 显示帧宽度
 * @param pic_height 显示帧高度
 * @param pic_sar   显示帧宽高比
 */

static void calculate_display_rect(SDL_Rect *rect,
                                   int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                                   int pic_width, int pic_height, AVRational pic_sar)
{
    AVRational aspect_ratio = pic_sar;
    int64_t width, height, x, y;

    if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0){ /*如果frame的比例是0/1或者比0/1更小*/
        aspect_ratio = av_make_q(1, 1);/*重置为1/1*/
    }

    /*frame的宽高与比例相乘*/
    //aspect_ratio * av_make_q(pic_width, pic_height)
    aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height));

    /* XXX: we suppose the screen has a 1.0 pixel ratio */
    height = scr_height;
    /*通过height和比例计算宽度*/
    width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1; /*& ~1作用是把结果变成偶数*/

    if (width > scr_width) {
        width = scr_width;
        height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1; /*& ~1 与上同理*/
    }
    /*计算渲染的坐标*/
    x = (scr_width - width) / 2;
    y = (scr_height - height) / 2;
    rect->x = scr_xleft + x;
    rect->y = scr_ytop  + y;
    /*通过比例计算后的宽高,设置矩形区域*/
    rect->w = FFMAX((int)width,  1);
    rect->h = FFMAX((int)height, 1);
}

/*
 *pix格式转换,把ffmpeg的格式转换成sdl支持的
 */
static void get_sdl_pix_fmt_and_blendmode(int format, Uint32 *sdl_pix_fmt, SDL_BlendMode *sdl_blendmode)
{
    *sdl_blendmode = SDL_BLENDMODE_NONE;
    *sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;

    if (format == AV_PIX_FMT_RGB32   ||
        format == AV_PIX_FMT_RGB32_1 ||
        format == AV_PIX_FMT_BGR32   ||
        format == AV_PIX_FMT_BGR32_1)
        *sdl_blendmode = SDL_BLENDMODE_BLEND;

    for (int i = 0; i < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; i++) {/*把ffmpeg的格式转换成sdl的格式*/
        if (format == sdl_texture_format_map[i].format) {
            *sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
            return;
        }
    }
}

/*
 * 更新纹理
 */
static int upload_texture(SDL_Texture **tex, AVFrame *frame)
{
    int ret = 0;
    Uint32 sdl_pix_fmt;
    SDL_BlendMode sdl_blendmode;
    // 根据frame中的图像格式(FFmpeg像素格式),获取对应的SDL像素格式和blendmode
    get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
    // 参数tex实际是&is->vid_texture,此处根据获取到的SDL像素格式填充到&is->vid_texture里
    if (realloc_texture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt,
                        frame->width, frame->height, sdl_blendmode, 0) < 0){
        return -1;
    }

    //根据sdl_pix_fmt从AVFrame中取数据填充纹理
    switch (sdl_pix_fmt) {
        // frame格式对应SDL_PIXELFORMAT_IYUV,不用进行图像格式转换,调用SDL_UpdateYUVTexture()更新SDL texture
        case SDL_PIXELFORMAT_IYUV:
            if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
                ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0], frame->linesize[0],
                                                       frame->data[1], frame->linesize[1],
                                                       frame->data[2], frame->linesize[2]);
            } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
                ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
                                                       frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
                                                       frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
            } else {
                av_log(NULL, AV_LOG_ERROR, "Mixed negative and positive linesizes are not supported.\n");
                return -1;
            }
            break;
          // frame格式对应其他SDL像素格式,不用进行图像格式转换，调用SDL_UpdateTexture()更新SDL texture
        default:
            if (frame->linesize[0] < 0) {
                ret = SDL_UpdateTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
            } else {
                ret = SDL_UpdateTexture(*tex, NULL, frame->data[0], frame->linesize[0]);
            }
            break;
    }
    return ret;
}

static enum AVColorSpace sdl_supported_color_spaces[] = {
    AVCOL_SPC_BT709,
    AVCOL_SPC_BT470BG,
    AVCOL_SPC_SMPTE170M,
    AVCOL_SPC_UNSPECIFIED,
};

static void set_sdl_yuv_conversion_mode(AVFrame *frame)
{
#if SDL_VERSION_ATLEAST(2,0,8)
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG){
            mode = SDL_YUV_CONVERSION_JPEG;
        }else if (frame->colorspace == AVCOL_SPC_BT709){
            mode = SDL_YUV_CONVERSION_BT709;
        }else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M){
            mode = SDL_YUV_CONVERSION_BT601;
        }
    }
    SDL_SetYUVConversionMode(mode); /* FIXME: no support for linear transfer */
#endif
}
/**
 * 图片显示
 * @param is
 */

static void video_image_display(VideoState *is)
{
    Frame *sp = NULL;
    SDL_Rect rect;

    // keep_last的作用就出来了,我们是有调用frame_queue_next,但最近出队列的帧并没有真正销毁
    // 所以这里可以读取出来显示
    Frame *vp = frame_queue_peek_last(&is->pictq);
    if (vk_renderer) {
        vk_renderer_display(vk_renderer, vp->frame);
        return;
    }

    if (is->subtitle_st) {
        if (frame_queue_nb_remaining(&is->subpq) > 0) {
            sp = frame_queue_peek(&is->subpq);

            if (vp->pts >= sp->pts + ((float) sp->sub.start_display_time / 1000)) {
                if (!sp->uploaded) {
                    uint8_t* pixels[4];
                    int pitch[4];
                    if (!sp->width || !sp->height) {
                        sp->width = vp->width;
                        sp->height = vp->height;
                    }

                    if (realloc_texture(&is->sub_texture, SDL_PIXELFORMAT_ARGB8888, sp->width, sp->height, SDL_BLENDMODE_BLEND, 1) < 0){
                        return;
                    }

                    for (int i = 0; i < sp->sub.num_rects; i++) {
                        AVSubtitleRect *sub_rect = sp->sub.rects[i];

                        sub_rect->x = av_clip(sub_rect->x, 0, sp->width );
                        sub_rect->y = av_clip(sub_rect->y, 0, sp->height);
                        sub_rect->w = av_clip(sub_rect->w, 0, sp->width  - sub_rect->x);
                        sub_rect->h = av_clip(sub_rect->h, 0, sp->height - sub_rect->y);

                        is->sub_convert_ctx = sws_getCachedContext(is->sub_convert_ctx,
                            sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
                            sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
                            0, NULL, NULL, NULL);

                        if (!is->sub_convert_ctx) {
                            av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
                            return;
                        }

                        if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)pixels, pitch)) {
                            sws_scale(is->sub_convert_ctx, (const uint8_t * const *)sub_rect->data, sub_rect->linesize,
                                      0, sub_rect->h, pixels, pitch);
                            SDL_UnlockTexture(is->sub_texture);
                        }
                    }
                    sp->uploaded = 1;
                }
            } else
                sp = NULL;
        }
    }

    //将帧宽高按照sar最大适配到窗口,并通过rect返回视频帧在窗口的显示位置和宽高
    calculate_display_rect(&rect, is->xleft, is->ytop, is->width, is->height, vp->width, vp->height, vp->sar);
    set_sdl_yuv_conversion_mode(vp->frame);

    if (!vp->uploaded) {
        // 把yuv数据更新到vid_texture
        if (upload_texture(&is->vid_texture, vp->frame) < 0) {
            set_sdl_yuv_conversion_mode(NULL);
            return;
        }
        vp->uploaded = 1;
        vp->flip_v = vp->frame->linesize[0] < 0;
    }

    SDL_RenderCopyEx(renderer, is->vid_texture, nullptr, &rect, 0, nullptr, static_cast<SDL_RendererFlip>(vp->flip_v ? SDL_FLIP_VERTICAL : 0));
    set_sdl_yuv_conversion_mode(nullptr);
    if (sp) {
#if USE_ONEPASS_SUBTITLE_RENDER
        SDL_RenderCopy(renderer, is->sub_texture, nullptr, &rect);
#else

        double xratio = (double)rect.w / (double)sp->width;
        double yratio = (double)rect.h / (double)sp->height;
        for (int i = 0; i < sp->sub.num_rects; i++) {
            SDL_Rect *sub_rect = (SDL_Rect*)sp->sub.rects[i];
            SDL_Rect target = {.x = rect.x + sub_rect->x * xratio,
                               .y = rect.y + sub_rect->y * yratio,
                               .w = sub_rect->w * xratio,
                               .h = sub_rect->h * yratio};
            SDL_RenderCopy(renderer, is->sub_texture, sub_rect, &target);
        }
#endif
    }
}

/**
 *
 * @param a
 * @param b
 * @return
 */
static inline int compute_mod(int a, int b)
{
    return a < 0 ? a%b + b : a%b;
}

/*
 *用于显示音频波形
 */
static void video_audio_display(VideoState *s)
{
    int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
    int ch, channels, h, h2;
    int64_t time_diff;
    int rdft_bits, nb_freq;

    for (rdft_bits = 1; (1 << rdft_bits) < 2 * s->height; rdft_bits++);
    nb_freq = 1 << (rdft_bits - 1);

    /* compute display index : center on currently output samples */
    channels = s->audio_tgt.ch_layout.nb_channels;
    nb_display_channels = channels;
    if (!s->paused) {
        int data_used= s->show_mode == VideoState::SHOW_MODE_WAVES ? s->width : (2*nb_freq);
        n = 2 * channels;
        delay = s->audio_write_buf_size;
        delay /= n;

        /* to be more precise, we take into account the time spent since
           the last buffer computation */
        if (audio_callback_time) {
            time_diff = av_gettime_relative() - audio_callback_time;
            delay -= (time_diff * s->audio_tgt.freq) / 1000000;
        }

        delay += 2 * data_used;
        if (delay < data_used){
            delay = data_used;
        }

        i_start= x = compute_mod(s->sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
        if (s->show_mode == VideoState::SHOW_MODE_WAVES) {
            h = INT_MIN;
            for (i = 0; i < 1000; i += channels) {
                int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
                int a = s->sample_array[idx];
                int b = s->sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
                int c = s->sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
                int d = s->sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
                int score = a - d;
                if (h < score && (b ^ c) < 0) {
                    h = score;
                    i_start = idx;
                }
            }
        }

        s->last_i_start = i_start;
    } else {
        i_start = s->last_i_start;
    }

    if (s->show_mode == VideoState::SHOW_MODE_WAVES) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        /* total height for one channel */
        h = s->height / nb_display_channels;
        /* graph height / 2 */
        h2 = (h * 9) / 20;
        for (ch = 0; ch < nb_display_channels; ch++) {
            i = i_start + ch;
            y1 = s->ytop + ch * h + (h / 2); /* position of center line */
            for (x = 0; x < s->width; x++) {
                y = (s->sample_array[i] * h2) >> 15;
                if (y < 0) {
                    y = -y;
                    ys = y1 - y;
                } else {
                    ys = y1;
                }
                fill_rectangle(s->xleft + x, ys, 1, y);
                i += channels;
                if (i >= SAMPLE_ARRAY_SIZE){
                    i -= SAMPLE_ARRAY_SIZE;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

        for (ch = 1; ch < nb_display_channels; ch++) {
            y = s->ytop + ch * h;
            fill_rectangle(s->xleft, y, s->width, 1);
        }
    } else {
        int err = 0;
        if (realloc_texture(&s->vis_texture, SDL_PIXELFORMAT_ARGB8888, s->width, s->height, SDL_BLENDMODE_NONE, 1) < 0){
            return;
        }

        if (s->xpos >= s->width){
            s->xpos = 0;
        }

        nb_display_channels= FFMIN(nb_display_channels, 2);
        if (rdft_bits != s->rdft_bits) {
            const float rdft_scale = 1.0;
            av_tx_uninit(&s->rdft);
            av_freep(&s->real_data);
            av_freep(&s->rdft_data);
            s->rdft_bits = rdft_bits;
            s->real_data = static_cast<float*>(av_malloc_array(nb_freq, 4 *sizeof(*s->real_data)));
            s->rdft_data = static_cast<AVComplexFloat*>(av_malloc_array(nb_freq + 1, 2 *sizeof(*s->rdft_data)));
            err = av_tx_init(&s->rdft, &s->rdft_fn, AV_TX_FLOAT_RDFT,
                             0, 1 << rdft_bits, &rdft_scale, 0);
        }

        if (err < 0 || !s->rdft_data) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to allocate buffers for RDFT, switching to waves display\n");
            s->show_mode = VideoState::SHOW_MODE_WAVES;
        } else {
            float *data_in[2];
            AVComplexFloat *data[2];
            SDL_Rect rect = {.x = s->xpos, .y = 0, .w = 1, .h = s->height};
            uint32_t *pixels;
            int pitch;
            for (ch = 0; ch < nb_display_channels; ch++) {
                data_in[ch] = s->real_data + 2 * nb_freq * ch;
                data[ch] = s->rdft_data + nb_freq * ch;
                i = i_start + ch;
                for (x = 0; x < 2 * nb_freq; x++) {
                    double w = (x-nb_freq) * (1.0 / nb_freq);
                    data_in[ch][x] = s->sample_array[i] * (1.0 - w * w);
                    i += channels;
                    if (i >= SAMPLE_ARRAY_SIZE)
                        i -= SAMPLE_ARRAY_SIZE;
                }
                s->rdft_fn(s->rdft, data[ch], data_in[ch], sizeof(float));
                data[ch][0].im = data[ch][nb_freq].re;
                data[ch][nb_freq].re = 0;
            }
            /* Least efficient way to do this, we should of course
             * directly access it but it is more than fast enough. */
            if (!SDL_LockTexture(s->vis_texture, &rect, (void **)&pixels, &pitch)) {
                pitch >>= 2;
                pixels += pitch * s->height;
                for (y = 0; y < s->height; y++) {
                    double w = 1 / sqrt(nb_freq);
                    int a = sqrt(w * sqrt(data[0][y].re * data[0][y].re + data[0][y].im * data[0][y].im));
                    int b = (nb_display_channels == 2 ) ? sqrt(w * hypot(data[1][y].re, data[1][y].im))
                                                        : a;
                    a = FFMIN(a, 255);
                    b = FFMIN(b, 255);
                    pixels -= pitch;
                    *pixels = (a << 16) + (b << 8) + ((a+b) >> 1);
                }
                SDL_UnlockTexture(s->vis_texture);
            }
            SDL_RenderCopy(renderer, s->vis_texture, NULL, NULL);
        }
        if (!s->paused){
            s->xpos++;
        }
    }
}

/*
 *关闭流资源
 */
static void stream_component_close(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    AVCodecParameters *codecpar;

    if (stream_index < 0 || stream_index >= ic->nb_streams){
        return;
    }

    codecpar = ic->streams[stream_index]->codecpar;

    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        decoder_abort(&is->auddec, &is->sampq);
        SDL_CloseAudioDevice(audio_dev);
        decoder_destroy(&is->auddec);
        swr_free(&is->swr_ctx);
        av_freep(&is->audio_buf1);
        is->audio_buf1_size = 0;
        is->audio_buf = NULL;

        if (is->rdft) {
            av_tx_uninit(&is->rdft);
            av_freep(&is->real_data);
            av_freep(&is->rdft_data);
            is->rdft = NULL;
            is->rdft_bits = 0;
        }
        break;
    case AVMEDIA_TYPE_VIDEO:
        decoder_abort(&is->viddec, &is->pictq);
        decoder_destroy(&is->viddec);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        decoder_abort(&is->subdec, &is->subpq);
        decoder_destroy(&is->subdec);
        break;
    default:
        break;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;

    switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        is->audio_st = NULL;
        is->audio_stream = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        is->video_st = NULL;
        is->video_stream = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        is->subtitle_st = NULL;
        is->subtitle_stream = -1;
        break;
    default:
        break;
    }
}

/**
 * 关闭流
 * @param is
 */
static void stream_close(VideoState *is)
{
    // 动态(线程/callback)的先停止退出
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    is->abort_request = 1; //请求退出
    SDL_WaitThread(is->read_tid, NULL);

    /* close each stream */
    if (is->audio_stream >= 0) {
        //关闭音频流
        stream_component_close(is, is->audio_stream);
    }

    if (is->video_stream >= 0){
        //关闭视频流
        stream_component_close(is, is->video_stream);
    }

    if (is->subtitle_stream >= 0){
        //关闭字幕流
        stream_component_close(is, is->subtitle_stream);
    }

    avformat_close_input(&is->ic);
    packet_queue_destroy(&is->videoq);
    packet_queue_destroy(&is->audioq);
    packet_queue_destroy(&is->subtitleq);

    /* free all pictures */
    frame_queue_destroy(&is->pictq);
    frame_queue_destroy(&is->sampq);
    frame_queue_destroy(&is->subpq);
    SDL_DestroyCond(is->continue_read_thread);
    sws_freeContext(is->sub_convert_ctx);
    av_free(is->filename);

    if (is->vis_texture){
        SDL_DestroyTexture(is->vis_texture);
    }

    if (is->vid_texture){
        SDL_DestroyTexture(is->vid_texture);
    }

    if (is->sub_texture){
        SDL_DestroyTexture(is->sub_texture);
    }

    av_free(is);
}

static void do_exit(VideoState *is)
{
    if (is) {
        stream_close(is);
    }

    if (renderer){
        SDL_DestroyRenderer(renderer);
    }

    if (vk_renderer) {
        vk_renderer_destroy(vk_renderer);
    }

    if (window){
        SDL_DestroyWindow(window);
    }

    uninit_opts();

    for (int i = 0; i < nb_vfilters; i++){
        av_freep(&vfilters_list[i]);
    }

    av_freep(&vfilters_list);
    av_freep(&video_codec_name);
    av_freep(&audio_codec_name);
    av_freep(&subtitle_codec_name);
    av_freep(&input_filename);
    avformat_network_deinit();
    if (show_status) {
        printf("\n");
    }

    SDL_Quit();
    av_log(NULL, AV_LOG_QUIET, "%s", "");
    exit(0);
}

static void sigterm_handler(int sig)
{
    exit(123);
}

/*
 * 设置默认窗口大小
 */
static void set_default_window_size(int width, int height, AVRational sar)
{
    SDL_Rect rect;
    int max_width  = screen_width  ? screen_width  : INT_MAX;   // 确定是否指定窗口最大宽度
    int max_height = screen_height ? screen_height : INT_MAX;   // 确定是否指定窗口最大高度
    if (max_width == INT_MAX && max_height == INT_MAX){ /*如果没有指定最大宽高*/
        max_height = height;    /*则使用外部传入的高作为最大高度*/
    }
    calculate_display_rect(&rect, 0, 0, max_width, max_height, width, height, sar);
    default_width = rect.w;    // 实际是渲染区域的宽高
    default_height = rect.h;
}

static int video_open(VideoState *is)
{
    int w = screen_width ? screen_width : default_width;
    int h = screen_height ? screen_height : default_height;

    if (!window_title){
        window_title = input_filename;
    }

    SDL_SetWindowTitle(window, window_title); /*设置窗口标题*/

    SDL_SetWindowSize(window, w, h);/*设置窗口大小*/
    SDL_SetWindowPosition(window, screen_left, screen_top);/*设置窗口显示位置,一般都是屏幕中间*/

    if (is_full_screen){
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);//全屏显示
    }

    SDL_ShowWindow(window);//窗口显示

    is->width  = w;
    is->height = h;

    return 0;
}

/* display the current picture, if any */
static void video_display(VideoState *is)
{
    if (!is->width){
        //如果窗口未显示，则显示窗口
        video_open(is);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);//渲染成黑色,并且不透明
    SDL_RenderClear(renderer);//使用绘图颜色清除当前渲染目标。此函数清除整个渲染目标,忽略视口和剪辑矩形。

    if (is->audio_st && is->show_mode != VideoState::SHOW_MODE_VIDEO){
        video_audio_display(is);//图形化显示仅有音轨的文件
    }else if (is->video_st){
        video_image_display(is);//显示一帧视频画面
    }
    SDL_RenderPresent(renderer);//将渲染器上的内容显示在窗口上,并刷新窗口
}

/**
 * 获取到的实际上是:最后一帧的pts + 从处理最后一帧开始到现在的时间,具体参考set_clock_at和get_clock的代码
 * c->pts_drift = 最后一帧的pts - 从处理最后一帧时间
 * clock=c->pts_drift+现在的时候
 * get_clock(&is->vidclk) == is->vidclk.pts , av_gettime_relative() / 1000000.0 - is->vidclk.last_updated + is->vidclk.pts
 */

static double get_clock(Clock *c)
{
    if (*c->queue_serial != c->serial)
        return NAN;// 不是同一个播放序列,时钟是无效
    if (c->paused) {
        return c->pts;  // 暂停的时候返回的是pts
    } else {
        double time = (double )av_gettime_relative() / 1000000.0;
        return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);//c->speed很多时候是1.0 , (time - c->last_updated) * (1.0 - c->speed) = 0
        //c->pts_drift + time 计算出音/视频播放到哪里
    }
    //pts_drift是消逝时间
}

/**
 * 设置时钟的核心实现
 * @param c
 * @param pts
 * @param serial
 * @param time
 */
static void set_clock_at(Clock *c, double pts, int serial, double time)
{
    c->pts = pts;                   /* 当前帧的pts */
    c->last_updated = time;         /* 最后更新的时间,实际上是当前的一个系统时间 */
    c->pts_drift = c->pts - time;   /* 当前帧pts和系统时间的差值(消逝时间),正常播放情况下两者的差值应该是比较固定的,因为两者都是以时间为基准进行线性增长 */
    c->serial = serial;
}

/**
 * 设置时钟
 * @param c
 * @param pts
 * @param serial
 */
static void set_clock(Clock *c, double pts, int serial)
{
    double time = (double )av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

/**
 * 设置速度
 * @param c
 * @param speed
 */
static void set_clock_speed(Clock *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

/**
 * 初始化时钟
 * @param c
 * @param queue_serial
 */
static void init_clock(Clock *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

/**
 * 外部时钟pts与从属时钟的时间差值超过AV_NOSYNC_THRESHOLD(10秒),则对外部时钟进行更新
 * @param c
 * @param slave
 */
static void sync_clock_to_slave(Clock *c, Clock *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD)){
        set_clock(c, slave_clock, slave->serial);
    }
}

/**
 * 获取当前作为同步的主时钟
 * @param is
 * @return
 */
static int get_master_sync_type(VideoState *is) {
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st){
            return AV_SYNC_VIDEO_MASTER;
        }else{
            return AV_SYNC_AUDIO_MASTER; /*如果没有视频成分则使用 audio master*/
        }
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st){
            return AV_SYNC_AUDIO_MASTER;
        }else{
            return AV_SYNC_EXTERNAL_CLOCK; /*没有音频的时候那就用外部时钟*/
        }
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

/* get the current master clock value */
/**
 * 获取主时钟的时间值
 * @param is
 * @return current master clock value
 */
static double get_master_clock(VideoState *is)
{
    double val;

    switch (get_master_sync_type(is)) {
        case AV_SYNC_VIDEO_MASTER:
            val = get_clock(&is->vidclk);
            break;
        case AV_SYNC_AUDIO_MASTER:
            val = get_clock(&is->audclk);
            break;
        default:
            val = get_clock(&is->extclk);
            break;
    }
    return val;
}

static void check_external_clock_speed(VideoState *is) {
   if (is->video_stream >= 0 && is->videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES ||
       is->audio_stream >= 0 && is->audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES) {
       set_clock_speed(&is->extclk, FFMAX(EXTERNAL_CLOCK_SPEED_MIN, is->extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));
   } else if ((is->video_stream < 0 || is->videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) &&
              (is->audio_stream < 0 || is->audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES)) {
       set_clock_speed(&is->extclk, FFMIN(EXTERNAL_CLOCK_SPEED_MAX, is->extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));
   } else {
       double speed = is->extclk.speed;
       if (speed != 1.0)
           set_clock_speed(&is->extclk, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
   }
}

/* seek in the stream */
/**
 * @brief seek stream
 * @param is
 * @param pos  具体seek到的位置
 * @param rel  增量情况
 * @param seek_by_bytes
 */
static void stream_seek(VideoState *is, int64_t pos, int64_t rel, int by_bytes)
{
    if (!is->seek_req) {
        is->seek_pos = pos;// 按时间微秒,按字节 byte
        is->seek_rel = rel;
        is->seek_flags &= ~AVSEEK_FLAG_BYTE; //不按字节的方式去seek
        if (by_bytes) {
            is->seek_flags |= AVSEEK_FLAG_BYTE;// 强制按字节的方式去seek
        }
        is->seek_req = 1; // 请求seek,在read_thread线程seek成功才将其置为0
        SDL_CondSignal(is->continue_read_thread);
    }
}

/* pause or resume the video */
/**
 * 暂停播放具体操作
 * @param is
 */
static void stream_toggle_pause(VideoState *is)
{
    // 如果当前是暂停 -> 恢复播放
    // 正常播放 -> 暂停
    if (is->paused) {// 当前是暂停，那这个时候进来这个函数就是要恢复播放
        /* 恢复暂停状态时也需要恢复时钟，需要更新vidclk */
        // 加上 暂停->恢复 经过的时间
        is->frame_timer += (double )av_gettime_relative() / 1000000.0 - is->vidclk.last_updated;
        if (is->read_pause_return != AVERROR(ENOSYS)) {
            is->vidclk.paused = 0;
        }
        // 设置时钟的意义，暂停状态下读取的是单纯pts
        // 重新矫正video时钟
        set_clock(&is->vidclk, get_clock(&is->vidclk), is->vidclk.serial);
    }
    set_clock(&is->extclk, get_clock(&is->extclk), is->extclk.serial);
    // 切换 pause/resume 两种状态
    is->paused = is->audclk.paused = is->vidclk.paused = is->extclk.paused = !is->paused;
}

/**
 * 暂停播放
 * @param is
 */
static void toggle_pause(VideoState *is)
{
    stream_toggle_pause(is);
    is->step = 0;// 逐帧的时候用
}

/**
 * 静音
 * @param is
 */
static void toggle_mute(VideoState *is)
{
    is->muted = !is->muted;//静音->取消静音
}

/**
 *更新音量
 * @param is
 * @param sign
 * @param step
 */
static void update_volume(VideoState *is, int sign, double step)
{
    double volume_level = is->audio_volume ? (20 * log(is->audio_volume / (double)SDL_MIX_MAXVOLUME) / log(10)) : -1000.0;
    int new_volume = lrint(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) / 20.0));
    is->audio_volume = av_clip(is->audio_volume == new_volume ? (is->audio_volume + sign) : new_volume, 0, SDL_MIX_MAXVOLUME);
}

/**
 * 快进到下一帧
 * @param is
 */
static void step_to_next_frame(VideoState *is)
{
    /* if the stream is paused unpause it, then step */
    if (is->paused){
        stream_toggle_pause(is);
    }
    is->step = 1;
}

/**
 * @brief 计算正在显示帧需要持续播放的时间。
 * @param delay 该参数实际传递的是当前显示帧和待播放帧的间隔。
 * @param is
 * @return 返回当前显示帧要持续播放的时间。为什么要调整返回的delay？为什么不支持使用相邻间隔帧时间？
 */
static double compute_target_delay(double delay, VideoState *is)
{
    double diff = 0.0;
    /* update delay to follow master synchronisation source */
    /* 如果发现当前主Clock源不是video,则计算当前视频时钟与主时钟的差值 */
    if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        /*如果视频是从属的,我们会尝试通过复制或删除帧来纠正较大的延迟*/
        //简单总结就是通过重复播放或者丢弃帧来等待音频或者追赶音频播放的速度
        diff = get_clock(&is->vidclk) - get_master_clock(is);

        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */

        // AV_SYNC_THRESHOLD_MIN = 0.04    AV_SYNC_THRESHOLD_MAX = 0.1  AV_SYNC_FRAMEDUP_THRESHOLD = 0.1
        // 0.04 ~ 0.1
        // delay落在0.04 ~ 0.1的区间
        double sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));

        /*
         * 列出三个可能性,不一定准确:
         * 假如delay=0.04s,diff = -0.5s,那么sync_threshold = 0.04,delay = 0
         *
         * 假如delay=0.04s,diff = 0.5s,那么sync_threshold = 0.04,delay = 2 * delay = 2 * 0.04 = 0.08
         *
         * 假如delay=0.2s,diff = 1s,那么sync_threshold = 0.1s,delay = 0.2 + 1 = 1.2
         */

        if (!isnan(diff) && fabs(diff) < is->max_frame_duration) {
            if (diff <= -sync_threshold){ /*视频比音频慢,视频已经落后了*/
                delay = FFMAX(0, delay + diff);//一般情况下delay都会被置0,几乎不会出现比0还大的情况,我们但他为0就ok
            }else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) { /*视频比音频快,这样情况用于应对一帧时长比较大的情况*/
                delay = delay + diff;//
            }else if (diff >= sync_threshold){ /*视频比音频快,这里直接翻两倍*/
                delay = 2 * delay;
            }
        }
    }

    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",delay, -diff);
    return delay;
}

/**
 * 计算上一帧需要持续的duration,这里有校正算法
 * @param is
 * @param vp
 * @param nextvp
 * @return vp->duration or duration_ or 0.0
 */
static double vp_duration(VideoState *is, Frame *vp, Frame *nextvp) {
    if (vp->serial == nextvp->serial) { // 同一播放序列,序列连续的情况下
        double duration_ = nextvp->pts - vp->pts;
        if (isnan(duration_) || // duration 数值异常
                duration_ <= 0 || // pts值没有递增时,前一帧与后一帧相差为0
                duration_ > is->max_frame_duration) { //超过了最大帧范围
            return vp->duration; /* 异常时以帧时间为基准(1秒/帧率) */
        }else{
            return duration_; //使用两帧pts差值计算duration,一般情况下也是走的这个分支
        }
    } else { // 不同播放序列,序列不连续则返回0
        return 0.0;
    }
}

/**
 * 更新vidio的pts
 * @param is
 * @param pts
 * @param serial
 */
static void update_video_pts(VideoState *is, double pts, int serial)
{
    /* update current video pts */
    set_clock(&is->vidclk, pts, serial);
    sync_clock_to_slave(&is->extclk, &is->vidclk);
}

/* called to display each frame */

/**
 * 非暂停或强制刷新的时候,循环调用video_refresh,refresh_loop_wait_event函数调用本函数
 * @param opaque
 * @param remaining_time
 */
static void video_refresh(void *opaque, double *remaining_time)
{
    VideoState *is = static_cast<decltype(is)>(opaque);
    double time;

    Frame *sp, *sp2;

    if (!is->paused && get_master_sync_type(is) == AV_SYNC_EXTERNAL_CLOCK && is->realtime){ //实时流用的
        check_external_clock_speed(is);
    }

    if (!display_disable && is->show_mode != VideoState ::SHOW_MODE_VIDEO && is->audio_st) { //没有视频只有音频,波形显示
        time = (double )av_gettime_relative() / 1000000.0;
        if (is->force_refresh || is->last_vis_time + rdftspeed < time) {
            video_display(is);
            is->last_vis_time = time;
        }
        *remaining_time = FFMIN(*remaining_time, is->last_vis_time + rdftspeed - time);
    }

    if (is->video_st) {
retry:
        if (frame_queue_nb_remaining(&is->pictq) == 0) {
            // nothing to do, no picture to display in the queue
        } else {
            double last_duration, delay;
            Frame *vp, *lastvp;

            /* dequeue the picture */
            lastvp = frame_queue_peek_last(&is->pictq);
            vp = frame_queue_peek(&is->pictq);

            if (vp->serial != is->videoq.serial) {
                frame_queue_next(&is->pictq);
                goto retry;
            }

            if (lastvp->serial != vp->serial){ //如果出现不同的序列,用当前时间去更新is->frame_timer
                // 新的播放序列重置当前时间
                is->frame_timer = (double )av_gettime_relative() / 1000000.0;
            }

            if (is->paused){
                goto display;
            }

            /* compute nominal last_duration */
            // 经过compute_target_delay方法,计算出待显示帧vp需要等待的时间
            // 如果以video同步，则delay直接等于last_duration。
            // 如果以audio或外部时钟同步，则需要比对主时钟调整待显示帧vp要等待的时间。
            last_duration = vp_duration(is, lastvp, vp); /*计算上一帧应显示时间*/
            delay = compute_target_delay(last_duration, is); /*计算上一帧lastvp还要播放的时间*/

            time = (double )av_gettime_relative() / 1000000.0;
            // is->frame_timer 实际上就是上一帧lastvp的播放时间,
            // is->frame_timer + delay 是待显示帧vp该播放的时间
            if (time < is->frame_timer + delay) { //判断是否继续显示上一帧
                // 当前系统时刻还未到达上一帧的结束时刻,那么还应该继续显示上一帧。
                // 计算出最小等待时间
                *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
                goto display;
            }

            // 走到这一步,说明已经到了或过了该显示的时间,待显示帧vp的状态变更为当前要显示的帧

            // is->frame_timer初始值为0
            is->frame_timer += delay; // 更新当前帧播放的时间
            // AV_SYNC_THRESHOLD_MAX = 0.1

            if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX){
                //is->frame_timer更新时间后,如果和系统时间差距太大,就纠正为系统时间
                is->frame_timer = time;
            }

            SDL_LockMutex(is->pictq.mutex);
            if (!isnan(vp->pts)){
                //更新video时钟
                update_video_pts(is, vp->pts, vp->serial);
            }
            SDL_UnlockMutex(is->pictq.mutex);

            //丢帧逻辑
            if (frame_queue_nb_remaining(&is->pictq) > 1) { /*帧队列最低有两帧才能进行丢弃一帧*/
                Frame *nextvp = frame_queue_peek_next(&is->pictq);
                double _duration = vp_duration(is, vp, nextvp);
                if(!is->step && (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) &&
                                time > is->frame_timer + _duration) {
                    /*time > is->frame_timer + duration检查是否落后一帧了*/
                    /*如果是,下面的执行就是丢帧的操作*/
                    is->frame_drops_late++;/*记录丢帧次数*/
                    frame_queue_next(&is->pictq);
                    goto retry; //回到函数开始位置,继续重试
                    //这里不能直接while丢帧,因为很可能audio clock重新对时了,这样delay值需要重新计算
                }
            }

            if (is->subtitle_st) {
                while (frame_queue_nb_remaining(&is->subpq) > 0) {
                    sp = frame_queue_peek(&is->subpq);

                    if (frame_queue_nb_remaining(&is->subpq) > 1){
                        sp2 = frame_queue_peek_next(&is->subpq);
                    }else {
                        sp2 = NULL;
                    }

                    if (sp->serial != is->subtitleq.serial
                            || (is->vidclk.pts > (sp->pts + ((float) sp->sub.end_display_time / 1000)))
                            || (sp2 && is->vidclk.pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
                    {
                        if (sp->uploaded) {

                            for (int i = 0; i < sp->sub.num_rects; i++) {
                                AVSubtitleRect *sub_rect = sp->sub.rects[i];
                                uint8_t *pixels;
                                int pitch;

                                if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)&pixels, &pitch)) {
                                    for (int j = 0; j < sub_rect->h; j++, pixels += pitch)
                                        memset(pixels, 0, sub_rect->w << 2);
                                    SDL_UnlockTexture(is->sub_texture);
                                }
                            }
                        }
                        frame_queue_next(&is->subpq);
                    } else {
                        break;
                    }
                }
            }

            frame_queue_next(&is->pictq);/*待播放帧变成当前帧,当前vp帧出队列*/
            is->force_refresh = 1;//强制刷新

            if (is->step && !is->paused){
                stream_toggle_pause(is); //逐帧的时候那继续进入暂停状态
            }
        }
display:
        /* display picture */
        if (!display_disable && is->force_refresh && is->show_mode == VideoState ::SHOW_MODE_VIDEO && is->pictq.rindex_shown){
            video_display(is); // 重点是显示
        }
    }

    is->force_refresh = 0;

    /*下是打印输出到一些信息*/
    if (show_status) {
        AVBPrint buf;
        static int64_t last_time;
        int64_t cur_time;
        int aqsize, vqsize, sqsize;
        double av_diff;

        cur_time = av_gettime_relative();
        if (!last_time || (cur_time - last_time) >= 30000) {
            aqsize = 0;
            vqsize = 0;
            sqsize = 0;
            if (is->audio_st){
                aqsize = is->audioq.size;
            }
            if (is->video_st){
                vqsize = is->videoq.size;
            }
            if (is->subtitle_st){
                sqsize = is->subtitleq.size;
            }
            av_diff = 0;
            if (is->audio_st && is->video_st){
                av_diff = get_clock(&is->audclk) - get_clock(&is->vidclk);
            }else if (is->video_st){
                av_diff = get_master_clock(is) - get_clock(&is->vidclk);
            }else if (is->audio_st){
                av_diff = get_master_clock(is) - get_clock(&is->audclk);
            }

            av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
            av_bprintf(&buf,
                      "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB \r",
                      get_master_clock(is),
                      (is->audio_st && is->video_st) ? "A-V" : (is->video_st ? "M-V" : (is->audio_st ? "M-A" : "   ")),
                      av_diff,
                      is->frame_drops_early + is->frame_drops_late,
                      aqsize / 1024,
                      vqsize / 1024,
                      sqsize);

            if (show_status == 1 && AV_LOG_INFO > av_log_get_level()){
                fprintf(stderr, "%s", buf.str);
            }else{
                av_log(NULL, AV_LOG_INFO, "%s", buf.str);
            }
            fflush(stderr);
            av_bprint_finalize(&buf, NULL);
            last_time = cur_time;
        }
    }
}

static int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double _duration, int64_t pos, int serial)
{
    Frame *vp;

#if defined(DEBUG_SYNC)
    printf("frame_type=%c pts=%0.3f\n",
           av_get_picture_type_char(src_frame->pict_type), pts);
#endif

    if (!(vp = frame_queue_peek_writable(&is->pictq))) { /*检查队列是否有空间可写,有空间则返回一个可写的自定义的Frame*/
        return -1;
    }

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = _duration;
    vp->pos = pos;
    vp->serial = serial;

    set_default_window_size(vp->width, vp->height, vp->sar);

    av_frame_move_ref(vp->frame, src_frame); //src_frame转移到vp->frame
    frame_queue_push(&is->pictq); //更新写索引
    return 0;
}

/**
 * 获取视频帧
 * @param is
 * @param frame
 * @return 1代表成功,0代表解码结束,负数则失败,有错误
 */
static int get_video_frame(VideoState *is, AVFrame *frame)
{
    int got_picture;

    if ((got_picture = decoder_decode_frame(&is->viddec, frame, NULL)) < 0){
        return -1;
    }

    if (got_picture) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE){
            //pts不为无效值,用视频流的时间基准计算pts的时间
            dpts = av_q2d(is->video_st->time_base) * frame->pts;
            //计算出秒为单位的pts
        }

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
        //猜测缩放比例

        if (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) { // 允许drop帧
            if (frame->pts != AV_NOPTS_VALUE) { // pts值有效
                double diff = dpts - get_master_clock(is);
                if (!isnan(diff) && // 差值有效
                    fabs(diff) < AV_NOSYNC_THRESHOLD && //差值在可同步范围呢
                    diff - is->frame_last_filter_delay < 0 && // 和过滤器有关系
                    is->viddec.pkt_serial == is->vidclk.serial && // 同一序列的包
                    is->videoq.nb_packets) { // packet队列至少有1帧数据
                    is->frame_drops_early++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }

    return got_picture;
}

/**
 * 配置滤镜
 * @param graph
 * @param filtergraph
 * @param source_ctx
 * @param sink_ctx
 * @return 负数为失败
 */
static int configure_filtergraph(AVFilterGraph *graph, const char *filtergraph,
                                 AVFilterContext *source_ctx, AVFilterContext *sink_ctx)
{
    int ret, i;
    int nb_filters = graph->nb_filters;
    AVFilterInOut *outputs = NULL, *inputs = NULL;

    if (filtergraph) {
        outputs = avfilter_inout_alloc(); //分配滤镜输出
        inputs  = avfilter_inout_alloc(); //分配滤镜输入
        if (!outputs || !inputs) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        outputs->name       = av_strdup("in"); //buffersrc
        outputs->filter_ctx = source_ctx;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;

        inputs->name        = av_strdup("out"); // buffersink
        inputs->filter_ctx  = sink_ctx;
        inputs->pad_idx     = 0;
        inputs->next        = NULL;

        if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, NULL)) < 0){
            goto fail;
        }
    } else {
        if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0){
            goto fail;
        }
    }

    /* Reorder the filters to ensure that inputs of the custom filters are merged first */
    for (i = 0; i < graph->nb_filters - nb_filters; i++){
        FFSWAP(AVFilterContext*, graph->filters[i], graph->filters[i + nb_filters]);
    }

    ret = avfilter_graph_config(graph, NULL);
fail:
    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);
    return ret;
}

/**
 * 配置视频滤镜
 * @param graph
 * @param is
 * @param vfilters
 * @param frame
 * @return 负数为失败
 */
static int configure_video_filters(AVFilterGraph *graph, VideoState *is, const char *vfilters, AVFrame *frame)
{
    enum AVPixelFormat pix_fmts[FF_ARRAY_ELEMS(sdl_texture_format_map)];
    char sws_flags_str[512] = ""; //
    char buffersrc_args[256]; //输入
    int ret;
    AVFilterContext *filt_src = NULL, *filt_out = NULL, *last_filter = NULL;
    AVCodecParameters *codecpar = is->video_st->codecpar;
    AVRational fr = av_guess_frame_rate(is->ic, is->video_st, NULL);//猜测帧率
    const AVDictionaryEntry *e = NULL;
    int nb_pix_fmts = 0;

    AVBufferSrcParameters *par = av_buffersrc_parameters_alloc();//分配AVBufferSrcParameters上下文

    if (!par){
        return AVERROR(ENOMEM);
    }

    for (int i = 0; i < renderer_info.num_texture_formats; i++) {
        for (int j = 0; j < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; j++) {
            if (renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt) {
                pix_fmts[nb_pix_fmts++] = sdl_texture_format_map[j].format;
                break;
            }
        }
    }

    pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;

    while ((e = av_dict_iterate(sws_dict, e))) {
        if (!strcmp(e->key, "sws_flags")) {
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", "flags", e->value);
        } else{
            av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", e->key, e->value);
        }
    }

    size_t len = strlen(sws_flags_str);
    if (len){
        sws_flags_str[len - 1] = 0;
    }

    graph->scale_sws_opts = av_strdup(sws_flags_str);

    snprintf(buffersrc_args, sizeof(buffersrc_args), //拼接过滤器命令
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d:"
             "colorspace=%d:range=%d",
             frame->width, frame->height, frame->format,
             is->video_st->time_base.num, is->video_st->time_base.den,
             codecpar->sample_aspect_ratio.num, FFMAX(codecpar->sample_aspect_ratio.den, 1),
             frame->colorspace, frame->color_range);

    if (fr.num && fr.den){
        av_strlcatf(buffersrc_args, sizeof(buffersrc_args), ":frame_rate=%d/%d", fr.num, fr.den);
        //buffersrc_args 加入 "frame_rate = fr.num / fr.den"
    }

    if ((ret = avfilter_graph_create_filter(&filt_src,
                                            avfilter_get_by_name("buffer"),
                                            "ffplay_buffer", buffersrc_args, NULL,
                                            graph)) < 0) {
        goto fail;
    }

    par->hw_frames_ctx = frame->hw_frames_ctx;
    ret = av_buffersrc_parameters_set(filt_src, par);
    if (ret < 0){
        goto fail;
    }

    ret = avfilter_graph_create_filter(&filt_out,
                                       avfilter_get_by_name("buffersink"),
                                       "ffplay_buffersink", NULL, NULL, graph); //配置并创建输出滤镜
    if (ret < 0){
        goto fail;
    }

    if ((ret = av_opt_set_int_list(filt_out, "pix_fmts", pix_fmts,  AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0){
        goto fail;
    }

    if (!vk_renderer &&
        (ret = av_opt_set_int_list(filt_out, "color_spaces", sdl_supported_color_spaces,  AVCOL_SPC_UNSPECIFIED, AV_OPT_SEARCH_CHILDREN)) < 0){
        goto fail;
    }

    last_filter = filt_out;

/* Note: this macro adds a filter before the lastly added filter, so the
 * processing order of the filters is in reverse */
#define INSERT_FILT(name, arg) do {                                          \
    AVFilterContext *filt_ctx;                                               \
                                                                             \
    ret = avfilter_graph_create_filter(&filt_ctx,                            \
                                       avfilter_get_by_name(name),           \
                                       "ffplay_" name, arg, NULL, graph);    \
    if (ret < 0){                                                            \
        goto fail;                                                           \
    }                                                                        \
    ret = avfilter_link(filt_ctx, 0, last_filter, 0);                        \
    if (ret < 0){                                                            \
        goto fail;                                                           \
    }                                                                        \
    last_filter = filt_ctx;                                                  \
} while (0)

    if (autorotate) {
        double theta = 0.0;
        int32_t *displaymatrix = NULL;
        AVFrameSideData *sd = av_frame_get_side_data(frame, AV_FRAME_DATA_DISPLAYMATRIX);
        if (sd){
            displaymatrix = (int32_t *)sd->data;
        }

        if (!displaymatrix) {
            const AVPacketSideData *psd = av_packet_side_data_get(is->video_st->codecpar->coded_side_data,
                                                                  is->video_st->codecpar->nb_coded_side_data,
                                                                  AV_PKT_DATA_DISPLAYMATRIX);
            if (psd){
                displaymatrix = (int32_t *)psd->data;
            }
        }

        theta = get_rotation(displaymatrix);

        if (fabs(theta - 90) < 1.0) {
            INSERT_FILT("transpose", "clock");
        } else if (fabs(theta - 180) < 1.0) {
            INSERT_FILT("hflip", NULL);
            INSERT_FILT("vflip", NULL);
        } else if (fabs(theta - 270) < 1.0) {
            INSERT_FILT("transpose", "cclock");
        } else if (fabs(theta) > 1.0) {
            char rotate_buf[64];
            snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
            INSERT_FILT("rotate", rotate_buf);
        }
    }

    if ((ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0){
        goto fail;
    }

    is->in_video_filter  = filt_src;
    is->out_video_filter = filt_out;

fail:
    av_freep(&par);
    return ret;
}

/**
 * 配置音频滤镜
 * @param is
 * @param afilters
 * @param force_output_format
 * @return 负数为失败
 */
static int configure_audio_filters(VideoState *is, const char *afilters, int force_output_format)
{
    static const enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int sample_rates[2] = { 0, -1 };
    AVFilterContext *filt_asrc = NULL, *filt_asink = NULL;
    char aresample_swr_opts[512] = "";
    const AVDictionaryEntry *e = NULL;
    AVBPrint bp;
    char asrc_args[256];
    int ret;

    avfilter_graph_free(&is->agraph);//先释放滤镜图,因为本函数可能在其他地方调用
    if (!(is->agraph = avfilter_graph_alloc())){ //分配滤镜图
        return AVERROR(ENOMEM);
    }

    is->agraph->nb_threads = filter_nbthreads;

    av_bprint_init(&bp, 0, AV_BPRINT_SIZE_AUTOMATIC);

    while ((e = av_dict_iterate(swr_opts, e))){
        av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
    }

    size_t len = strlen(aresample_swr_opts);
    if (len){
        aresample_swr_opts[len - 1] = 0;
    }

    av_opt_set(is->agraph, "aresample_swr_opts", aresample_swr_opts, 0);

    av_channel_layout_describe_bprint(&is->audio_filter_src.ch_layout, &bp);

    ret = snprintf(asrc_args, sizeof(asrc_args), //拼接滤镜参数
                   "sample_rate=%d:sample_fmt=%s:time_base=%d/%d:channel_layout=%s",
                   is->audio_filter_src.freq, av_get_sample_fmt_name(is->audio_filter_src.fmt),
                   1, is->audio_filter_src.freq, bp.str);

    ret = avfilter_graph_create_filter(&filt_asrc, //创建"abuffer"输入滤镜
                                       avfilter_get_by_name("abuffer"), "ffplay_abuffer",
                                       asrc_args, NULL, is->agraph);
    if (ret < 0){
        goto end;
    }

    ret = avfilter_graph_create_filter(&filt_asink, //创建输出滤镜
                                       avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
                                       NULL, NULL, is->agraph);
    if (ret < 0){
        goto end;
    }

    if ((ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts,  AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0){
        //filt_asink添加参数sample_fmts
        goto end;
    }

    if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0){
        goto end;
    }

    if (force_output_format) { //强制输出格式
        av_bprint_clear(&bp);
        av_channel_layout_describe_bprint(&is->audio_tgt.ch_layout, &bp);
        sample_rates[0] = is->audio_tgt.freq;
        if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0){
            goto end;
        }

        if ((ret = av_opt_set(filt_asink, "ch_layouts", bp.str, AV_OPT_SEARCH_CHILDREN)) < 0){
            goto end;
        }

        if ((ret = av_opt_set_int_list(filt_asink, "sample_rates" ,sample_rates,-1, AV_OPT_SEARCH_CHILDREN)) < 0){
            goto end;
        }
    }

    if ((ret = configure_filtergraph(is->agraph, afilters, filt_asrc, filt_asink)) < 0){
        goto end;
    }

    is->in_audio_filter  = filt_asrc;
    is->out_audio_filter = filt_asink;

end:
    if (ret < 0){
        avfilter_graph_free(&is->agraph);
    }
    av_bprint_finalize(&bp, NULL);

    return ret;
}

/**
 * 音频线程
 * @param arg
 * @return
 */
static int audio_thread(void *arg)
{
    VideoState *is = static_cast<decltype(is)>(arg);
    AVFrame *frame = av_frame_alloc();
    Frame *af;
    int last_serial = -1;
    int reconfigure;
    int got_frame = 0;
    AVRational tb;
    int ret = 0;

    if (!frame){
        return AVERROR(ENOMEM);
    }

    do {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame, NULL)) < 0){ //解码音频并存放到音频帧队列
            goto the_end;
        }

        if (got_frame) { //解码成功
                tb = (AVRational){1, frame->sample_rate}; //以1/sample_rate为时间基准

                reconfigure = //对比预设的滤镜参数和实际解码出来的帧的参数是否匹配
                    cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.ch_layout.nb_channels,
                                   static_cast<AVSampleFormat>(frame->format), frame->ch_layout.nb_channels)    ||
                    av_channel_layout_compare(&is->audio_filter_src.ch_layout, &frame->ch_layout) ||
                    is->audio_filter_src.freq           != frame->sample_rate ||
                    is->auddec.pkt_serial               != last_serial;

                if (reconfigure) { //预设的滤镜参数和实际解码出来的帧的参数不一样
                    char buf1[1024], buf2[1024];
                    av_channel_layout_describe(&is->audio_filter_src.ch_layout, buf1, sizeof(buf1)); //获取滤镜的预设通道布局描述
                    av_channel_layout_describe(&frame->ch_layout, buf2, sizeof(buf2)); //获取解码帧的通道布局描述
                    av_log(NULL, AV_LOG_DEBUG,
                           "Audio frame changed from rate:%d ch:%d fmt:%s layout:%s serial:%d to rate:%d ch:%d fmt:%s layout:%s serial:%d\n",
                           is->audio_filter_src.freq, is->audio_filter_src.ch_layout.nb_channels, av_get_sample_fmt_name(is->audio_filter_src.fmt), buf1, last_serial,
                           frame->sample_rate, frame->ch_layout.nb_channels, av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame->format)), buf2, is->auddec.pkt_serial);

                    is->audio_filter_src.fmt            = static_cast<AVSampleFormat>(frame->format); //更新滤镜的采样格式
                    ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &frame->ch_layout); //更新滤镜的通道布局参数

                    if (ret < 0){
                        goto the_end;
                    }

                    is->audio_filter_src.freq           = frame->sample_rate; //更新滤镜的采样率
                    last_serial                         = is->auddec.pkt_serial;

                    if ((ret = configure_audio_filters(is, afilters, 1)) < 0){ // 重新配置音频滤镜
                        goto the_end;
                    }
                }
                //这里说明一下,就算不需要滤镜,ffplay也会分配一个空滤镜,解码后的帧都要经过滤镜

            if ((ret = av_buffersrc_add_frame(is->in_audio_filter, frame)) < 0){ //把解码后的帧输入到滤镜
                goto the_end;
            }

            while ((ret = av_buffersink_get_frame_flags(is->out_audio_filter, frame, 0)) >= 0) { //循环从滤镜读取帧

                FrameData *fd = frame->opaque_ref ? (FrameData*)frame->opaque_ref->data : NULL; //不懂为何这样才能获取pkt_pos

                tb = av_buffersink_get_time_base(is->out_audio_filter); //用过滤后的帧的时间基准

                if (!(af = frame_queue_peek_writable(&is->sampq))) { //窥探队列可读帧,返回一个可写帧,可能会阻塞,如果队列满,这里肯定会阻塞,等待队列有空位
                    goto the_end;
                }

                af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb); //把帧pts刻度转为以秒为单位的实际时间
                af->pos = fd ? fd->pkt_pos : -1;
                af->serial = is->auddec.pkt_serial;
                af->duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate}); //计算经过滤镜后的播放时长

                av_frame_move_ref(af->frame, frame); //资源转移
                frame_queue_push(&is->sampq); //入队

                if (is->audioq.serial != is->auddec.pkt_serial){
                    break;
                }
            }
            if (ret == AVERROR_EOF) { //解码结束
                is->auddec.finished = is->auddec.pkt_serial;
            }
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);

 the_end:
    avfilter_graph_free(&is->agraph); //线程退出释放滤镜
    av_frame_free(&frame); //释放帧
    return ret;
}

/**
 * 创建解码线程,audio/video有独立的线程,启动包(未解码)队列,,audio/video有独立的队列
 * @param d
 * @param fn
 * @param thread_name
 * @param arg
 * @return
 */

static int decoder_start(Decoder *d, int (*fn)(void *), const char *thread_name, void* arg)
{
    packet_queue_start(d->queue);
    d->decoder_tid = SDL_CreateThread(fn, thread_name, arg);
    if (!d->decoder_tid) {
        av_log(NULL, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    return 0;
}

/**
 * 视频线程
 * @param arg
 * @return
 */
static int video_thread(void *arg)
{
    VideoState *is = static_cast<decltype(is)>(arg);
    AVFrame *frame = av_frame_alloc();
    double pts;
    //double duration;
    int ret;
    AVRational tb = is->video_st->time_base; /*流的time_base*/
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL); //猜测视频帧率

    AVFilterGraph *graph = NULL;
    AVFilterContext *filt_out = NULL, *filt_in = NULL;
    int last_w = 0;
    int last_h = 0;
    enum AVPixelFormat last_format = static_cast<AVPixelFormat>(-2);
    int last_serial = -1;
    int last_vfilter_idx = 0;

    if (!frame){
        return AVERROR(ENOMEM);
    }

    for (;;) {
        ret = get_video_frame(is, frame);
        if (ret < 0){
            goto the_end;
        }
        if (!ret){
            continue;
        }

        if (   last_w != frame->width
            || last_h != frame->height
            || last_format != frame->format
            || last_serial != is->viddec.pkt_serial
            || last_vfilter_idx != is->vfilter_idx) { //对比前一帧和下一帧的宽高,格式和序列是否一样,不一样就重建滤镜,这里有个疑问?为什么序列不一样需要重建滤镜

            av_log(NULL, AV_LOG_DEBUG,
                   "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                   last_w, last_h,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                   frame->width, frame->height,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)), "none"), is->viddec.pkt_serial);

            avfilter_graph_free(&graph);//释放旧滤镜

            graph = avfilter_graph_alloc(); //分配新滤镜

            if (!graph) {
                ret = AVERROR(ENOMEM);
                goto the_end;
            }

            graph->nb_threads = filter_nbthreads; //滤镜处理线程数

            if ((ret = configure_video_filters(graph, is, vfilters_list ? vfilters_list[is->vfilter_idx] : NULL, frame)) < 0) {
                //滤镜配置失败
                SDL_Event event;
                event.type = FF_QUIT_EVENT; //退出事件
                event.user.data1 = is;
                SDL_PushEvent(&event); //插入事件队列
                goto the_end;
            }

            /*******************************更新参数*************************************/
            filt_in  = is->in_video_filter;
            filt_out = is->out_video_filter;
            last_w = frame->width;
            last_h = frame->height;
            last_format = static_cast<AVPixelFormat>(frame->format);
            last_serial = is->viddec.pkt_serial;
            last_vfilter_idx = is->vfilter_idx;
            frame_rate = av_buffersink_get_frame_rate(filt_out);
            /*******************************更新参数*************************************/
        }

        ret = av_buffersrc_add_frame(filt_in, frame); //解码后的帧进入滤镜处理

        if (ret < 0){
            goto the_end;
        }

        while (ret >= 0) { //循环读取滤镜的帧,并存入video_frame_queue

            is->frame_last_returned_time = (double )av_gettime_relative() / 1000000.0;

            ret = av_buffersink_get_frame_flags(filt_out, frame, 0);

            if (ret < 0) {
                if (ret == AVERROR_EOF){ //读取完成
                    is->viddec.finished = is->viddec.pkt_serial; //标记完成
                }
                ret = 0;
                break;
            }

            FrameData *fd = frame->opaque_ref ? (FrameData*)frame->opaque_ref->data : NULL; //获取pkt_pos

            is->frame_last_filter_delay = (double )av_gettime_relative() / 1000000.0 - is->frame_last_returned_time; //记录从滤镜读取一帧数据需要多久,用于丢帧(get_video_frame函数使用)

            // AV_NOSYNC_THRESHOLD / 10.0 = 1.0
            if (fabs(is->frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0){
                is->frame_last_filter_delay = 0;
            }

            tb = av_buffersink_get_time_base(filt_out);

            double _duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0); /*计算帧间隔,如果分子或分母为0,先用0代替,播放的时候还会再算一次*/

            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : (double )frame->pts * av_q2d(tb); /*转成单位秒*/
            ret = queue_picture(is, frame, pts, _duration, fd ? fd->pkt_pos : -1, is->viddec.pkt_serial); //写入video_frame_queue
            av_frame_unref(frame); /*queue_picture 已经把frame mov_ref了*/
            if (is->videoq.serial != is->viddec.pkt_serial){
                break;
            }
        }

        if (ret < 0){
            goto the_end;
        }
    }
 the_end:
    avfilter_graph_free(&graph);
    av_frame_free(&frame);
    return 0;
}
/**
 * 字幕线程
 * @param arg
 * @return
 */
static int subtitle_thread(void *arg)
{
    VideoState *is = static_cast<decltype(is)>(arg);

    for (;;) {
        Frame *sp;

        if (!(sp = frame_queue_peek_writable(&is->subpq))){ //窥探出一个可写字幕帧
            return 0;
        }

        int got_subtitle;
        if ((got_subtitle = decoder_decode_frame(&is->subdec, NULL, &sp->sub)) < 0){
            break;
        }

        double pts = 0;

        if (got_subtitle && sp->sub.format == 0) { //0 = graphics,不明白这里要这么做
            if (sp->sub.pts != AV_NOPTS_VALUE){
                pts = (double )sp->sub.pts / (double)AV_TIME_BASE;
            }

            sp->pts = pts;
            sp->serial = is->subdec.pkt_serial;
            sp->width = is->subdec.avctx->width;
            sp->height = is->subdec.avctx->height;
            sp->uploaded = 0;

            /* now we can update the picture count */
            frame_queue_push(&is->subpq);

        } else if (got_subtitle) {
            avsubtitle_free(&sp->sub); //
        }else{}
    }
    return 0;
}

/* copy samples for viewing in editor window */
//复制样本以便在编辑器窗口中查看
/**
 * 用于音频波形显示
 * @param is
 * @param samples
 * @param samples_size
 */
static void update_sample_display(VideoState *is, short *samples, int samples_size)
{
    int size = samples_size / sizeof(short);
    while (size > 0) {
        int len = SAMPLE_ARRAY_SIZE - is->sample_array_index;
        if (len > size){
            len = size;
        }

        memcpy(is->sample_array + is->sample_array_index, samples, len * sizeof(short));
        samples += len;
        is->sample_array_index += len;
        if (is->sample_array_index >= SAMPLE_ARRAY_SIZE) {
            is->sample_array_index = 0;
        }
        size -= len;
    }
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */

/**
 * 如果视频时钟/外部时钟作为主时钟,本函数用于同步音频
 * @param is
 * @param nb_samples
 * @return
 */

static int synchronize_audio(VideoState *is, int nb_samples)
{
    int wanted_nb_samples = nb_samples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if (get_master_sync_type(is) != AV_SYNC_AUDIO_MASTER) { //如果是视频或者外部时钟,则采用加权平均法去进行同步,把时间差控制在阈值范围内

        double diff = get_clock(&is->audclk) - get_master_clock(is);

        if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {

            is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum; //加权总和
            //其实这是一种降权的操作,主要作用是让前面的差异权重越来越小,后面的差异权重越来越大,我把 diff 分为 3 次差异讲解
            //a_diff 为第一次差异,b_diff 为第二次差异,c_diff 为第三次差异
            //is->audio_diff_cum = c_diff * 1 + ((b_diff + (a_diff * 0.79432) ) * 0.79432)
            //可以看到,最开始的差异 a_diff 乘了两次 0.79,所以 a_diff 会变得越来越小,b_diff 乘了一次 0.79432,c_diff 还是原来的 c_diff , c_diff 的权重最大是1
            //参考链接:https://ffmpeg.xianwaizhiyin.net/ffplay/audio_sync.html

            if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                //AUDIO_DIFF_AVG_NB = 20
                /* not enough measures to have a correct estimate */
                is->audio_diff_avg_count++; //累加到20
            } else {
                /* estimate the A-V difference */
                double avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef); //avg_diff是加权平均结果
                //参考链接:https://ffmpeg.xianwaizhiyin.net/ffplay/audio_sync.html

                int min_nb_samples, max_nb_samples;

                if (fabs(avg_diff) >= is->audio_diff_threshold) { //加权平均结果有没有超出阈值范围

                    // SAMPLE_CORRECTION_PERCENT_MAX = 10

                    wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq); //
                    min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }

                //替换后
//                if (fabs(avg_diff) >= is->audio_diff_threshold) {
//                    wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq);
//                    min_nb_samples = ((nb_samples * 0.9);
//                    max_nb_samples = ((nb_samples * 1.1);
//                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
//                }
//                 这是为了每次调整样本数,不能把 AVFrame 的样本数减少或者增加超过 10%,因为音频的连续性很强,调整幅度太大,耳朵容易察觉到

                av_log(NULL, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",
                        diff, avg_diff, wanted_nb_samples - nb_samples,
                        is->audio_clock, is->audio_diff_threshold);
            }
        } else {
            /* too big difference : may be initial PTS errors, so
               reset A-V filter */
            is->audio_diff_avg_count = 0;
            is->audio_diff_cum       = 0;
        }
    }

    return wanted_nb_samples;
}

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in is->audio_buf, with size in bytes given by the return
 * value.
 */

/**
 *
 * @param is
 * @return
 */

static int audio_decode_frame(VideoState *is)
{
    int resampled_data_size;
    av_unused double audio_clock0;
    int wanted_nb_samples;
    Frame *af;

    if (is->paused){
        return -1;
    }

    do {
#if defined(_WIN32)
        while (frame_queue_nb_remaining(&is->sampq) == 0) {
            if ((av_gettime_relative() - audio_callback_time) > 1000000LL * is->audio_hw_buf_size / is->audio_tgt.bytes_per_sec / 2)
                return -1;
            av_usleep (1000);
        }
#endif
        if (!(af = frame_queue_peek_readable(&is->sampq))) { //读取一帧
            return -1;
        }
        frame_queue_next(&is->sampq);
    } while (af->serial != is->audioq.serial);

    //根据frame中指定的音频参数获取缓冲区的大小 af->frame->channels * af->frame->nb_samples * frame->format_size
    int data_size = av_samples_get_buffer_size(NULL, af->frame->ch_layout.nb_channels,
                                           af->frame->nb_samples,
                                           static_cast<AVSampleFormat>(af->frame->format), 1);

    wanted_nb_samples = synchronize_audio(is, af->frame->nb_samples); //获取样本数校正值:若同步时钟是音频,则不调整样本数;否则根据同步需要调整样本数

    // is->audio_tgt是SDL可接受的音频帧数,是audio_open()中取得的参数
    // 在audio_open()函数中又有 "is->audio_src = is->audio_tgt"
    // 此处表示:如果frame中的音频参数 == is->audio_src == is->audio_tgt
    // 那音频重采样的过程就免了(因此时is->swr_ctr是NULL)
    // 否则使用frame(源)和is->audio_tgt(目标)中的音频参数来设置is->swr_ctx
    // 并使用frame中的音频参数来赋值is->audio_src

/*******************************************************************重采样有关*************************************************************************/
    if (af->frame->format        != is->audio_src.fmt                                       ||
        av_channel_layout_compare(&af->frame->ch_layout, &is->audio_src.ch_layout) ||
        af->frame->sample_rate   != is->audio_src.freq                                      ||
        (wanted_nb_samples       != af->frame->nb_samples && !is->swr_ctx)) { //与预设的参数不一样,进行重采样

        swr_free(&is->swr_ctx); //先释放原有重采样器

        int ret = swr_alloc_set_opts2(&is->swr_ctx,
                            &is->audio_tgt.ch_layout,
                            is->audio_tgt.fmt,
                            is->audio_tgt.freq,
                            &af->frame->ch_layout,
                            static_cast<AVSampleFormat>(af->frame->format),
                            af->frame->sample_rate,
                            0, NULL); //分配新重采样器

        if (ret < 0 || swr_init(is->swr_ctx) < 0) {

            av_log(NULL, AV_LOG_ERROR,
                   "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    af->frame->sample_rate, av_get_sample_fmt_name(static_cast<AVSampleFormat>(af->frame->format)), af->frame->ch_layout.nb_channels,
                    is->audio_tgt.freq, av_get_sample_fmt_name(is->audio_tgt.fmt), is->audio_tgt.ch_layout.nb_channels);

            swr_free(&is->swr_ctx);

            return -1;
        }
/**********************************用重采样后的参数赋值给is->audio_src***************************/
        if (av_channel_layout_copy(&is->audio_src.ch_layout, &af->frame->ch_layout) < 0) {
            return -1;
        }

        is->audio_src.freq = af->frame->sample_rate;
        is->audio_src.fmt = static_cast<AVSampleFormat>(af->frame->format);
/**********************************用重采样后的参数赋值给is->audio_src***************************/
    }

    if (is->swr_ctx) {
        const uint8_t **in = (const uint8_t **)af->frame->extended_data;
        uint8_t **out = &is->audio_buf1;
        int out_count = (int64_t)wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate + 256; /*计算输出的sample个数*/
        int out_size  = av_samples_get_buffer_size(NULL, is->audio_tgt.ch_layout.nb_channels, out_count, is->audio_tgt.fmt, 0);

        if (out_size < 0) {
            av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }

        if (wanted_nb_samples != af->frame->nb_samples) {
            if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,
                                        wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0) { //延时补偿

                av_log(NULL, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                return -1;
            }
        }

        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);

        if (!is->audio_buf1){
            return AVERROR(ENOMEM);
        }

        int len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);

        if (len2 < 0) {
            av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }

        if (len2 == out_count) {
            av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if (swr_init(is->swr_ctx) < 0) {
                swr_free(&is->swr_ctx);
            }
        }

        is->audio_buf = is->audio_buf1;//is->audio_buf指向重采样后的数据
        resampled_data_size = len2 * is->audio_tgt.ch_layout.nb_channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
/******************************************************************重采样有关****************************************************************************************/
    } else { //此处是不需重采样的处理
        is->audio_buf = af->frame->data[0];
        resampled_data_size = data_size;
    }

    audio_clock0 = is->audio_clock;
    /* update the audio clock with the pts */
    if (!isnan(af->pts)) {
        is->audio_clock = af->pts + (double) af->frame->nb_samples / af->frame->sample_rate;
        /*由于frame的pts是从0开始,所以每次需要加上(double) af->frame->nb_samples / af->frame->sample_rate */
        /*第一个frame的pts是0,并非它的时间就是0,pts是一个刻度值*/
    }else {
        is->audio_clock = NAN;
    }

    is->audio_clock_serial = af->serial; //更新序列
#ifdef DEBUG
    {
        static double last_clock;
        printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
               is->audio_clock - last_clock,
               is->audio_clock, audio_clock0);
        last_clock = is->audio_clock;
    }
#endif
    return resampled_data_size;
}

/* prepare a new audio buffer */

/**
 *
 * @param opaque 指向user的数据,本程序指向VideoState
 * @param stream SDL内部使用,用户往stream拷贝PCM数据
 * @param len SDL给出需拷贝的长度
 */

static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
    VideoState *is = static_cast<decltype(is)>(opaque);
    int audio_size, len1;

    audio_callback_time = av_gettime_relative(); //获取进入while之前的时间,因为while可能产生延迟

    while (len > 0) {
        /* (1)如果is->audio_buf_index < is->audio_buf_size则说明上次拷贝还剩余一些数据
          * 先拷贝到stream再调用audio_decode_frame
          * (2)如果audio_buf消耗完了,则调用audio_decode_frame重新填充audio_buf */

        if (is->audio_buf_index >= is->audio_buf_size) { /*如果is->audio_buf_index < is->audio_buf_size,先把之前剩余的数据拷贝到stream*/
           audio_size = audio_decode_frame(is);
           if (audio_size < 0) {
                /* if error, just output silence */
               is->audio_buf = NULL;
               is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
           } else {
               if (is->show_mode != VideoState ::SHOW_MODE_VIDEO)
                   update_sample_display(is, (int16_t *)is->audio_buf, audio_size);
               is->audio_buf_size = audio_size; /*读到多少字节数据*/
           }
           is->audio_buf_index = 0;
        }

        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len){ //超出缓冲区给的长度,先拷贝缓冲区给出的长度
            len1 = len;
        }
        //根据audio_volume决定如何输出audio_buf
        /* 判断是否为静音,以及当前音量的大小,如果音量为最大则直接拷贝数据 */
        if (!is->muted && is->audio_buf && is->audio_volume == SDL_MIX_MAXVOLUME){
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        }else {
            memset(stream, 0, len1);
            // 3.调整音量
            /* 如果处于mute状态则直接使用stream填0数据, 暂停时is->audio_buf = NULL */
            if (!is->muted && is->audio_buf){
                SDL_MixAudioFormat(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, AUDIO_S16SYS, len1, is->audio_volume);
            }
        }
        len -= len1;
        stream += len1;
        /* 更新is->audio_buf_index,指向audio_buf中未被拷贝到stream的数据(剩余数据)的起始位置 */
        is->audio_buf_index += len1;
    }

    is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index; //记录还有多少未被写入SDL缓冲区
    /* Let's assume the audio driver that is used by SDL has two periods. */
    if (!isnan(is->audio_clock)) {
        //更新音频时钟
        set_clock_at(&is->audclk, is->audio_clock - (double)(2 * is->audio_hw_buf_size + is->audio_write_buf_size) / is->audio_tgt.bytes_per_sec,
                     is->audio_clock_serial, (double )audio_callback_time / 1000000.0);
        sync_clock_to_slave(&is->extclk, &is->audclk);
    }
}

/**
 * 打开音频
 * @param opaque
 * @param wanted_channel_layout
 * @param wanted_sample_rate
 * @param audio_hw_params
 * @return
 */
static int audio_open(void *opaque, AVChannelLayout *wanted_channel_layout, int wanted_sample_rate, struct AudioParams *audio_hw_params)
{
    SDL_AudioSpec wanted_spec, spec;
    static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    //next_nb_channels[],这其实是一个map表,声道切换映射表,举个例子,如果音响设备不支持 7 声道的数据播放,肯定不能直接报错,还要尝试一下其他声道能不能成功打开设备吧,这个其他声道就是next_nb_channels[]

    /*
     *  next_nb_channels[7] = 6,从7声道切换到6声道打开音频设备
        next_nb_channels[6] = 4,从6声道切换到4声道打开音频设备
        next_nb_channels[5] = 6,从5声道切换到6声道打开音频设备
        next_nb_channels[4] = 2,从4声道切换到2声道打开音频设备
        next_nb_channels[3] = 6,从3声道切换到6声道打开音频设备
        next_nb_channels[2] = 1,从双声道切换到单声道打开音频设备
        next_nb_channels[1] = 0,单声道都打不开音频设备,无法再切换,需要降低采样率播放
        next_nb_channels[0] = 0,0声道都打不开音频设备,无法再切换,需要降低采样率播放
     */

    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;
    int wanted_nb_channels = wanted_channel_layout->nb_channels;

    const char *env = SDL_getenv("SDL_AUDIO_CHANNELS");

    if (env) {
        // 若环境变量有设置,优先从环境变量取得声道数和声道布局
        wanted_nb_channels = atoi(env);
        av_channel_layout_uninit(wanted_channel_layout);
        av_channel_layout_default(wanted_channel_layout, wanted_nb_channels);
    }

    if (wanted_channel_layout->order != AV_CHANNEL_ORDER_NATIVE) {
        av_channel_layout_uninit(wanted_channel_layout);
        av_channel_layout_default(wanted_channel_layout, wanted_nb_channels);
    }

    //根据channel_layout获取nb_channels,当传入参数wanted_nb_channels不匹配时,此处会作修正
    wanted_nb_channels = wanted_channel_layout->nb_channels;
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;

    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
        return -1;
    }

    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq){ // 从采样率数组中找到第一个不大于传入参数wanted_sample_rate的值
        // 音频采样格式有两大类型:planar和packed,假设一个双声道音频文件,一个左声道采样点记作L,一个右声道采样点记作R,则:
        // planar存储格式 :(plane1)LLLLLLLL...LLLL (plane2)RRRRRRRR...RRRR
        // packed存储格式 :(plane1)LRLRLRLR...........................LRLR
        // 在这两种采样类型下,又细分多种采样格式,如AV_SAMPLE_FMT_S16、AV_SAMPLE_FMT_S16P等,
        // 注意SDL2.0目前不支持planar格式
        // channel_layout是int64_t类型,表示音频声道布局,每bit代表一个特定的声道,参考channel_layout.h中的定义,一目了然
        // 数据量(bits/秒) = 采样率(Hz) * 采样深度(bit) * 声道数
        next_sample_rate_idx--;
    }

    wanted_spec.format = AUDIO_S16SYS; //固定格式
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    //函数是求对数,以2为底,也就是2乘以自身多少次等于括号里面的结果,这个多少次会进行取整操作的,av_log2不会返回小数
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = opaque;

    // 打开音频设备并创建音频处理线程,期望的参数是wanted_spec,实际得到的硬件参数是spec
    // 1) SDL提供两种使音频设备取得音频数据方法:
    //    a. push,SDL以特定的频率调用回调函数,在回调函数中取得音频数据
    //    b. pull,用户程序以特定的频率调用SDL_QueueAudio(),向音频设备提供数据,此种情况wanted_spec.callback=NULL
    // 2) 音频设备打开后播放静音,不启动回调,调用SDL_PauseAudio(0)后启动回调,开始正常播放音频
    // SDL_OpenAudioDevice()第一个参数为NULL时,等价于SDL_OpenAudio()

    while (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec,
                                             &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
        //打开设备出错,不是马上退出,而是更换参数继续测试,不过一般不会进入到这个循环体
        av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
               wanted_spec.channels, wanted_spec.freq, SDL_GetError());

        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];

        if (!wanted_spec.channels) {

            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];// 经过debug,next_sample_rate_idx已经到了0,如果这里再减,感觉会有问题
            wanted_spec.channels = wanted_nb_channels;

            if (!wanted_spec.freq) {
                av_log(NULL, AV_LOG_ERROR,
                       "No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        av_channel_layout_default(wanted_channel_layout, wanted_spec.channels);
    }

    if (spec.format != AUDIO_S16SYS) {
        av_log(NULL, AV_LOG_ERROR,
               "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }

    if (spec.channels != wanted_spec.channels) {
        av_channel_layout_uninit(wanted_channel_layout);
        av_channel_layout_default(wanted_channel_layout, spec.channels);
        if (wanted_channel_layout->order != AV_CHANNEL_ORDER_NATIVE) {
            av_log(NULL, AV_LOG_ERROR,
                   "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }

    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;

    if (av_channel_layout_copy(&audio_hw_params->ch_layout, wanted_channel_layout) < 0){
        return -1;
    }
    /*计算一个采样点占用多少个字节*/
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->ch_layout.nb_channels, 1, audio_hw_params->fmt, 1);
    /*通过采样法计算每一秒需要多少个字节去填充*/
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->ch_layout.nb_channels, audio_hw_params->freq, audio_hw_params->fmt, 1);

    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
        return -1;
    }

    return spec.size;
}

static int create_hwaccel(AVBufferRef **device_ctx)
{
    enum AVHWDeviceType type;
    int ret;
    AVBufferRef *vk_dev;

    *device_ctx = NULL;

    if (!hwaccel)
        return 0;

    type = av_hwdevice_find_type_by_name(hwaccel);
    if (type == AV_HWDEVICE_TYPE_NONE)
        return AVERROR(ENOTSUP);

    ret = vk_renderer_get_hw_dev(vk_renderer, &vk_dev);
    if (ret < 0)
        return ret;

    ret = av_hwdevice_ctx_create_derived(device_ctx, type, vk_dev, 0);
    if (!ret)
        return 0;

    if (ret != AVERROR(ENOSYS))
        return ret;

    av_log(NULL, AV_LOG_WARNING, "Derive %s from vulkan not supported.\n", hwaccel);
    ret = av_hwdevice_ctx_create(device_ctx, type, NULL, NULL, 0);
    return ret;
}


/* open a given stream. Return 0 if OK */
/**
 *
 * @param is
 * @param stream_index
 * @return  Return 0 if OK
 */
static int stream_component_open(VideoState *is, int stream_index)
{
    AVFormatContext *ic = is->ic;
    const char *forced_codec_name = NULL;
    AVDictionary *opts = NULL;
    const AVDictionaryEntry *t = NULL;
    int sample_rate;
    AVChannelLayout ch_layout {  };
    int ret = 0;
    int stream_lowres = lowres;
    const AVCodec *codec{};

    if (stream_index < 0 || stream_index >= ic->nb_streams) {
        return -1;
    }

    AVCodecContext *avctx = avcodec_alloc_context3(NULL); //分配解码器上下文
    if (!avctx){
        return AVERROR(ENOMEM);
    }

    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar); //把流的解码信息拷贝到解码器
    if (ret < 0){
        goto fail;
    }

    avctx->pkt_timebase = ic->streams[stream_index]->time_base; // 设置pkt_timebase

    codec = avcodec_find_decoder(avctx->codec_id); //根本流提供的信息寻找匹配的解码器

    switch(avctx->codec_type){
        case AVMEDIA_TYPE_AUDIO   : is->last_audio_stream    = stream_index; forced_codec_name =    audio_codec_name; break;
        case AVMEDIA_TYPE_SUBTITLE: is->last_subtitle_stream = stream_index; forced_codec_name = subtitle_codec_name; break;
        case AVMEDIA_TYPE_VIDEO   : is->last_video_stream    = stream_index; forced_codec_name =    video_codec_name; break;
    }

    if (forced_codec_name){ //用户通过参数手动强制解码器会用到
        codec = avcodec_find_decoder_by_name(forced_codec_name);
    }

    if (!codec) {
        if (forced_codec_name) {
            av_log(NULL, AV_LOG_WARNING,
                   "No codec could be found with name '%s'\n", forced_codec_name);
        }else{
            av_log(NULL, AV_LOG_WARNING,
                   "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        }
        ret = AVERROR(EINVAL);
        goto fail;
    }

    avctx->codec_id = codec->id;

    if (stream_lowres > codec->max_lowres) {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
                codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }

    avctx->lowres = stream_lowres;

    if (fast){
        avctx->flags2 |= AV_CODEC_FLAG2_FAST;
    }

    ret = filter_codec_opts(codec_opts, avctx->codec_id, ic,
                            ic->streams[stream_index], codec, &opts);
    //把命令行参数的相关参数提取出来
    //ffpaly -b:v 2000k -i juren-5s.mp4
    //上面的命令,指定了解码器的码率，但是他指定的是视频的码率，当 stream_component_open() 打开视频流的时候,这个码率参数才会被 filter_codec_opts() 提取出来

    //而stream_component_open() 打开音频流的时候,b:v 不会被提取出来,因为这个参数是跟 视频流 相关的
    if (ret < 0){
        goto fail;
    }

    if (!av_dict_get(opts, "threads", NULL, 0)){ //设置解码器内部线程数
        av_dict_set(&opts, "threads", "auto", 0);
    }

    if (stream_lowres) {
        av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    }

    av_dict_set(&opts, "flags", "+copy_opaque", AV_DICT_MULTIKEY);

    if (avctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        ret = create_hwaccel(&avctx->hw_device_ctx); //视频硬件加速
        if (ret < 0){
            goto fail;
        }
    }

    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) { //打开解码器
        goto fail;
    }

    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret =  AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT; //AVDISCARD_DEFAULT丢弃无用的数据包,在read_thread中设置为AVDISCARD_ALL
    //chatgpt回答两处地方不同的设置是因为,有助于在流初始化和处理之间进行平衡,确保既能正确处理流数据,又能在不需要时优化资源利用
    //本函数是在read_thread线程调用,且在discard被设置为AVDISCARD_ALL后调用,意味着discard先被设置为忽略所有流,本函数必须把它设置为AVDISCARD_ALL以外的值
    switch (avctx->codec_type) {

    case AVMEDIA_TYPE_AUDIO: //音频相关解码设置
        {
            is->audio_filter_src.freq = avctx->sample_rate;

            ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &avctx->ch_layout);

            if (ret < 0) {
                goto fail;
            }

            is->audio_filter_src.fmt = avctx->sample_fmt;

            if ((ret = configure_audio_filters(is, afilters, 0)) < 0) { //配置音频滤镜,即使不使用滤镜,也会创建空滤镜
                goto fail;
            }

            AVFilterContext *sink = is->out_audio_filter;

            sample_rate = av_buffersink_get_sample_rate(sink); //从出口滤镜获取采样率

            ret = av_buffersink_get_ch_layout(sink, &ch_layout); //从出口滤镜获取通道布局

            if (ret < 0){
                goto fail;
            }
        }

        /* prepare audio output */
        //调用audio_open打开sdl音频输出,实际打开的设备参数保存在audio_tgt,返回值表示输出设备的缓冲区大小
        if ((ret = audio_open(is, &ch_layout, sample_rate, &is->audio_tgt)) < 0) {
            goto fail;
        }

        is->audio_hw_buf_size = ret; //记录audio_open返回的大小
        is->audio_src = is->audio_tgt; //暂且将数据源参数等同于目标输出参数
        is->audio_buf_size  = 0;
        is->audio_buf_index = 0;

        /* init averaging filter */
        /*************************外部或者视频作为主时钟时,以下三个字段用于同步音频****************************************/
        //AUDIO_DIFF_AVG_NB = 20
        is->audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB); //计算结果大概是0.794 //这个变量实际上是 等比数列 里面的 公比q,此变量用于非音频作为主时钟同步音频时采用的,如果音频是主时钟,该变量不采用
        is->audio_diff_avg_count = 0; //平均次数累加
        /* since we do not have a precise anough audio FIFO fullness,
           we correct audio sync only if larger than this threshold */
        is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec; //音频同步阈值
        /***********************************************************************************************************/
        is->audio_stream = stream_index; //记录audio_stream 索引
        is->audio_st = ic->streams[stream_index]; //记录audio_stream的指针

        if ((ret = decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread)) < 0){
            //音频解码器初始化
            goto fail;
        }

        if (is->ic->iformat->flags & AVFMT_NOTIMESTAMPS) { //检查输入的媒体文件是否没有时间基准
            is->auddec.start_pts = is->audio_st->start_time;
            is->auddec.start_pts_tb = is->audio_st->time_base;
        }

        if ((ret = decoder_start(&is->auddec, audio_thread, "audio_decoder", is)) < 0){
            //启动音频解码线程
            goto out;
        }

        SDL_PauseAudioDevice(audio_dev, 0); //0=播放,!0暂停
        break;

    case AVMEDIA_TYPE_VIDEO: //视频解码相关设置

        is->video_stream = stream_index; //记录video_stream索引
        is->video_st = ic->streams[stream_index]; //记录video_stream指针

        if ((ret = decoder_init(&is->viddec, avctx, &is->videoq, is->continue_read_thread)) < 0){
            //视频解码器初始化
            goto fail;
        }

        if ((ret = decoder_start(&is->viddec, video_thread, "video_decoder", is)) < 0){
            //启动视频解码线程
            goto out;
        }

        is->queue_attachments_req = 1; //请求封面
        break;

    case AVMEDIA_TYPE_SUBTITLE: //字幕解码相关设置

        is->subtitle_stream = stream_index; //记录subtitle_stream的索引
        is->subtitle_st = ic->streams[stream_index]; //记录subtitle_stream的指针

        if ((ret = decoder_init(&is->subdec, avctx, &is->subtitleq, is->continue_read_thread)) < 0) {
            //字幕解码器初始化
            goto fail;
        }

        if ((ret = decoder_start(&is->subdec, subtitle_thread, "subtitle_decoder", is)) < 0){
            //启动字幕解码线程
            goto out;
        }

        break;
    default:
        break;
    }
    goto out;

fail:
    avcodec_free_context(&avctx);
out:
    av_channel_layout_uninit(&ch_layout);
    av_dict_free(&opts); //释放参数

    return ret;
}

/**
 *这里是设置给ffmpeg内部,当ffmpeg内部当执行耗时操作时(一般是在执行while或者for循环的数据读取时)就会调用该函数
 * 由于ffmpeg的IO可能会阻塞,阻塞的时候则调用decode_interrupt_cb去控制否终止阻塞操作
 * 此处通过返回abort_request的值,通常在程序退出时候,才会返回1,运行过程基本上为0
 * @param ctx
 * @return 1终止阻塞,0继续阻塞等待
 */
static int decode_interrupt_cb(void *ctx)
{
    static int64_t s_pre_time = 0;
    auto cur_time  {av_gettime_relative() / 1000};
    //fprintf(stderr,"decode_interrupt_cb interval: %lldms",cur_time-s_pre_time);
    s_pre_time = cur_time;
    VideoState *is = static_cast<decltype(is)>(ctx);
    return is->abort_request;
}

/**
 * 查看流是否有足够的包
 * @param st
 * @param stream_id
 * @param queue
 * @return
 */
static int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue) {
    //MIN_FRAMES = 25
    return stream_id < 0 || // 不对该流作检查,认为该流有足够的数据数据
           queue->abort_request || // 请求退出
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) || /*是否为封面图*/
           queue->nb_packets > MIN_FRAMES && //packet > 25
           (!queue->duration || av_q2d(st->time_base) * (double )queue->duration > 1.0); // 满足PacketQueue总时长为0
}

/**
 * 判断是否为实时流
 * @param s
 * @return 1 true 0 false
 */
static int is_realtime(AVFormatContext *s)
{
    if(!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp")){
        return 1;
    }

    if(s->pb && (!strncmp(s->url, "rtp:", 4)|| !strncmp(s->url, "udp:", 4))){
        return 1;
    }

    return 0;
}

/* this thread gets the stream from the disk or the network */
/**
 * 读线程,主要功能是做解复用,从码流中分离音视频packet,并插入缓存队列
 * @param arg
 * @return 0正常退出,负数则为失败
 */

static int read_thread(void *arg)
{
    VideoState *is = static_cast<decltype(is)>(arg);
    AVFormatContext *ic = NULL;
    int err, i, ret;
    int st_index[AVMEDIA_TYPE_NB];
    AVPacket *pkt = NULL;


    const AVDictionaryEntry *t;
    SDL_mutex *wait_mutex = SDL_CreateMutex(); //创建锁
    int scan_all_pmts_set = 0;


    if (!wait_mutex) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    memset(st_index, -1, sizeof(st_index));
    is->eof = 0;

    pkt = av_packet_alloc();
    if (!pkt) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate packet.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    ic = avformat_alloc_context(); //分配格式上下文
    if (!ic) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    /* 设置中断回调函数,如果出错或者退出，就根据目前程序设置的状态选择继续check或者直接退出 */
    /* 当执行耗时操作时(一般是在执行while或者for循环的数据读取时),会调用interrupt_callback.callback
     * 回调函数中返回1则代表ffmpeg结束耗时操作退出当前函数的调用
     * 回调函数中返回0则代表ffmpeg内部继续执行耗时操作，直到完成既定的任务(比如读取到既定的数据包)
     */

    ic->interrupt_callback.callback = decode_interrupt_cb; //设置IO中断回调,用于控制是否终止阻塞操作
    ic->interrupt_callback.opaque = is;

    if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
        //特定选项处理
        av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }

    err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
    //打开文件,主要是探测协议类型,如果是网络文件则创建网络链接等
    if (err < 0) {
        print_error(is->filename, err);
        ret = -1;
        goto fail;
    }

    if (scan_all_pmts_set) {
        av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);
    }

    if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    is->ic = ic;

    if (genpts) {
        ic->flags |= AVFMT_FLAG_GENPTS;
    }

    if (find_stream_info) {
        AVDictionary **opts;
        int orig_nb_streams = ic->nb_streams;

        err = setup_find_stream_info_opts(ic, codec_opts, &opts);
        if (err < 0) {
            av_log(NULL, AV_LOG_ERROR,
                   "Error setting up avformat_find_stream_info() options\n");
            ret = err;
            goto fail;
        }

        err = avformat_find_stream_info(ic, opts); //获取媒体文件的详细信息,对于一些信息不全的媒体文件特别有用

        for (i = 0; i < orig_nb_streams; i++) {
            av_dict_free(&opts[i]);
        }

        av_freep(&opts);

        if (err < 0) {
            av_log(NULL, AV_LOG_WARNING,
                   "%s: could not find codec parameters\n", is->filename);
            ret = -1;
            goto fail;
        }
    }

    if (ic->pb) {
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end
    }

    if (seek_by_bytes < 0) {
        seek_by_bytes = !(ic->iformat->flags & AVFMT_NO_BYTE_SEEK) &&
                        !!(ic->iformat->flags & AVFMT_TS_DISCONT) &&
                        strcmp("ogg", ic->iformat->name);
    }

    is->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0; //最大帧长

    if (!window_title && (t = av_dict_get(ic->metadata, "title", NULL, 0))) {
        window_title = av_asprintf("%s - %s", t->value, input_filename);
    }

    /* if seeking requested, we execute it */
    //检测是否指定播放起始时间
    if (start_time != AV_NOPTS_VALUE) {

        int64_t timestamp = start_time;
        /* add the stream start time */
        if (ic->start_time != AV_NOPTS_VALUE) {
            timestamp += ic->start_time;
        }
        //seek的指定的位置开始播放
        ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if (ret < 0) {
            av_log(NULL, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n",
                    is->filename, (double)timestamp / AV_TIME_BASE);
        }
    }
    /* 是否为实时流媒体 */
    is->realtime = is_realtime(ic);

    if (show_status) {
        av_dump_format(ic, 0, is->filename, 0);
    }

    //查找AVStream
    //根据用户指定来查找流
    for (i = 0; i < ic->nb_streams; i++) {
        AVStream *st = ic->streams[i];
        enum AVMediaType type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1){
            if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0){
                st_index[type] = i;
            }
        }
    }

    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if (wanted_stream_spec[i] && st_index[i] == -1) {
            av_log(NULL, AV_LOG_ERROR, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string(static_cast<AVMediaType>(i)));
            st_index[i] = INT_MAX;
        }
    }
/****************************************************如果有某个流被关闭,下面对应的if不会执行************************************************************/
    if (!video_disable) {
        st_index[AVMEDIA_TYPE_VIDEO] =
                av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
                                    st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
    }

    if (!audio_disable) {
        st_index[AVMEDIA_TYPE_AUDIO] =
                av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                    st_index[AVMEDIA_TYPE_AUDIO],
                                    st_index[AVMEDIA_TYPE_VIDEO],
                                    NULL, 0);
    }

    if (!video_disable && !subtitle_disable) {
        st_index[AVMEDIA_TYPE_SUBTITLE] =
                av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
                                    st_index[AVMEDIA_TYPE_SUBTITLE],
                                    (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                     st_index[AVMEDIA_TYPE_AUDIO] :
                                     st_index[AVMEDIA_TYPE_VIDEO]),
                                    NULL, 0);
    }
/***********************************************************************************************************************************/
    is->show_mode = show_mode;
    //从待处理流中获取相关参数,设置显示窗口的宽度、高度及宽高比
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = st->codecpar;
        //根据流和帧宽高比猜测视频帧的像素宽高比(像素的宽高比,注意不是图像的)
        AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
        if (codecpar->width){
            //设置显示窗口的大小和宽高比
            set_default_window_size(codecpar->width, codecpar->height, sar);
        }
    }

    /* open the streams */
    //打开视频、音频解码器,在此会打开相应解码器,并创建相应的解码线程
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_AUDIO]);
    }

    ret = -1;
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {

        ret = stream_component_open(is, st_index[AVMEDIA_TYPE_VIDEO]);
    }

    if (is->show_mode == VideoState ::SHOW_MODE_NONE) {
        //选择怎么显示,如果视频打开成功,就显示视频画面,否则,显示音频对应的频谱图
        is->show_mode = ret >= 0 ? VideoState::SHOW_MODE_VIDEO : VideoState::SHOW_MODE_RDFT;
    }

    if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        stream_component_open(is, st_index[AVMEDIA_TYPE_SUBTITLE]);
    }

    if (is->video_stream < 0 && is->audio_stream < 0) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n",is->filename);
        ret = -1;
        goto fail;
    }

    if (infinite_buffer < 0 && is->realtime){
        //如果是实时流
        infinite_buffer = 1;
    }

    for (;;) {
        if (is->abort_request) { //检查是否退出
            break;
        }
        if (is->paused != is->last_paused) { //检测是否暂停/继续
            is->last_paused = is->paused;
            if (is->paused) {
                is->read_pause_return = av_read_pause(ic);//如果是网络流,就暂停网络流
            }else {
                av_read_play(ic); //网络流继续读取
            }
        }
#if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
        if (is->paused &&
                (!strcmp(ic->iformat->name, "rtsp") ||
                 (ic->pb && !strncmp(input_filename, "mmsh:", 5)))) {
            /* wait 10 ms to avoid trying to get another packet */
            /* XXX: horrible */
            // 等待10ms,避免立马尝试下一个Packet
            SDL_Delay(10);
            continue;
        }
#endif
        //检测是否seek
        if (is->seek_req) { //是否有seek请求
            int64_t seek_target = is->seek_pos; //目标位置
            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
// FIXME the +-2 is due to rounding being not done in the correct direction in generation
//      of the seek_pos/seek_rel variables
            //修复由于四舍五入,没有再seek_pos/seek_rel变量的正确方向上进行
            ret = avformat_seek_file(is->ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
            //is->seek_flags决定是按照时间seek还是按字节seek

            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR,
                       "%s: error while seeking\n", is->ic->url);
            }else{
                //seek的时候，要把原先的数据情况，并重启解码器，put flush_pkt的目的是告知解码线程需要
                if (is->audio_stream >= 0){
                    packet_queue_flush(&is->audioq);
                }
                if (is->subtitle_stream >= 0){
                    packet_queue_flush(&is->subtitleq);
                }
                if (is->video_stream >= 0){
                    packet_queue_flush(&is->videoq);
                }

                if (is->seek_flags & AVSEEK_FLAG_BYTE) {
                   set_clock(&is->extclk, NAN, 0);
                } else {
                   set_clock(&is->extclk, (double )seek_target / (double)AV_TIME_BASE, 0);
                }
            }
            is->seek_req = 0;
            is->queue_attachments_req = 1;
            is->eof = 0;
            if (is->paused) { //如果是暂停状态,则seek后马上播放下一帧,不然在暂停状态seek后还会停留在seek之前的画面
                step_to_next_frame(is); //如果是暂停状态seek,播放下一帧
            }
        }

        //检测video是否为attached_pic
        if (is->queue_attachments_req) { /*seek后请求封面*/
            // attached_pic 附带的图片,比如说一些MP3,AAC音频文件附带的专辑封面,所以需要注意的是音频文件不一定只存在音频流本身
            if (is->video_st && is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                if ((ret = av_packet_ref(pkt, &is->video_st->attached_pic)) < 0){
                    goto fail;
                }
                packet_queue_put(&is->videoq, pkt);
                packet_queue_put_nullpacket(&is->videoq, pkt, is->video_stream);
            }
            is->queue_attachments_req = 0;
        }

        /* if the queue are full, no need to read more */
        //检测队列是否已经有足够数据
        //缓存队列有足够的包，不需要继续读取数据
        if (infinite_buffer < 1 &&
              (is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE
            || (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
                stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
                stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq)))) {
            /* wait 10 ms */
            SDL_LockMutex(wait_mutex);
            //如果没有唤醒则超时10ms退出,比如在seek操作时这里会被唤醒
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }

        //检测码流是否已经播放结束
        if (!is->paused && // 非暂停,这里的执行是因为码流读取完毕后 插入空包所致
            (!is->audio_st || (is->auddec.finished == is->audioq.serial && frame_queue_nb_remaining(&is->sampq) == 0)) &&
            (!is->video_st || (is->viddec.finished == is->videoq.serial && frame_queue_nb_remaining(&is->pictq) == 0))) {

            if (loop != 1 && (!loop || --loop)) { //如果loop > 1,则循环播放,loop是循环次数
                stream_seek(is, start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
            } else if (autoexit) { //是否自动退出
                ret = AVERROR_EOF;
                goto fail;
            }else{}
        }

        //读取媒体数据,得到的是音视频分离后、解码前的数据
        ret = av_read_frame(ic, pkt);

        //检测数据是否读取完毕
        if (ret < 0) {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof) { //文件读取完毕

                // 插入空包说明码流数据读取完毕了,之前讲解码的时候说过刷空包是为了从解码器把所有帧都读出来
                if (is->video_stream >= 0){
                    packet_queue_put_nullpacket(&is->videoq, pkt, is->video_stream);
                }

                if (is->audio_stream >= 0){
                    packet_queue_put_nullpacket(&is->audioq, pkt, is->audio_stream);
                }

                if (is->subtitle_stream >= 0){
                    packet_queue_put_nullpacket(&is->subtitleq, pkt, is->subtitle_stream);
                }

                is->eof = 1; //文件读取完毕
            }

            if (ic->pb && ic->pb->error) {
                if (autoexit){
                    goto fail;
                }else{
                    break;
                }
            }
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        } else {
            is->eof = 0;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */
        /*检查数据包是否在用户指定的播放范围内,然后排队,否则丢弃*/
        int64_t stream_start_time = ic->streams[pkt->stream_index]->start_time; //流的start_time
        int64_t pkt_ts = (pkt->pts == AV_NOPTS_VALUE) ? pkt->dts : pkt->pts;

        const double t_Delta  = (double )(pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0));
        const double st_tb = av_q2d(ic->streams[pkt->stream_index]->time_base);
        const double _start_time = (double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / (double )AV_TIME_BASE;

        int pkt_in_play_range = (duration == AV_NOPTS_VALUE) || (t_Delta * st_tb - _start_time <= ((double)duration / AV_TIME_BASE));
        //pkt_in_play_range非0,代表读取的数据包在播放范围

        if (pkt->stream_index == is->audio_stream && pkt_in_play_range) {
            packet_queue_put(&is->audioq, pkt);
        } else if (pkt->stream_index == is->video_stream && pkt_in_play_range
                   && !(is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
            packet_queue_put(&is->videoq, pkt);
        } else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range) {
            packet_queue_put(&is->subtitleq, pkt);
        } else {
            av_packet_unref(pkt);
        }
    }

    ret = 0;
 fail:
    if (ic && !is->ic){
        avformat_close_input(&ic);
    }

    av_packet_free(&pkt);
    if (ret != 0) {
        SDL_Event event;
        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    SDL_DestroyMutex(wait_mutex);
    return 0;
}

/**
 * stream_open
 * @param filename
 * @param iformat
 * @return VideoState *
 */
static VideoState *stream_open(const char *filename,
                               const AVInputFormat *iformat)
{
    VideoState *is = static_cast<decltype(is)>(av_mallocz(sizeof(VideoState))); /* 分配VideoState并初始化 */
    //VideoState起始所有成员变量都为0
    if (!is){
        return nullptr;
    }

    is->last_video_stream = is->video_stream = -1;
    is->last_audio_stream = is->audio_stream = -1;
    is->last_subtitle_stream = is->subtitle_stream = -1;
    is->filename = av_strdup(filename);

    if (!is->filename){
        goto fail;
    }

    is->iformat = iformat;
    is->ytop    = 0;
    is->xleft   = 0;

    /* start video display */
    //包队列和帧队列初始化
    //VIDEO_PICTURE_QUEUE_SIZE = 3 三帧 I P B帧
    if (frame_queue_init(&is->pictq, &is->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0){
        goto fail;
    }

    //SUBPICTURE_QUEUE_SIZE = 16
    if (frame_queue_init(&is->subpq, &is->subtitleq, SUBPICTURE_QUEUE_SIZE, 0) < 0){
        goto fail;
    }

    //SAMPLE_QUEUE_SIZE = 9
    if (frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0){
        goto fail;
    }

    if (packet_queue_init(&is->videoq) < 0 ||
        packet_queue_init(&is->audioq) < 0 ||
        packet_queue_init(&is->subtitleq) < 0){
        goto fail;
    }

    if (!(is->continue_read_thread = SDL_CreateCond())) { //创建条件变量
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        goto fail;
    }

    /*初始化时钟*/
    /*指向队列的序列*/
    init_clock(&is->vidclk, &is->videoq.serial);
    init_clock(&is->audclk, &is->audioq.serial);
    init_clock(&is->extclk, &is->extclk.serial);
    is->audio_clock_serial = -1;

    //音量初始化
    if (startup_volume < 0) {
        av_log(NULL, AV_LOG_WARNING, "-volume=%d < 0, setting to 0\n", startup_volume);
    }

    if (startup_volume > 100) {
        av_log(NULL, AV_LOG_WARNING, "-volume=%d > 100, setting to 100\n", startup_volume);
    }
    //SDL_MIX_MAXVOLUME = 128
    startup_volume = av_clip(startup_volume, 0, 100);
    startup_volume = av_clip(SDL_MIX_MAXVOLUME * startup_volume / 100, 0, SDL_MIX_MAXVOLUME);
    is->audio_volume = startup_volume;
    is->muted = 0;
    is->av_sync_type = av_sync_type; //默认是音频作为主时钟
    is->read_tid     = SDL_CreateThread(read_thread, "read_thread", is); //创建读线程

    if (!is->read_tid) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError());
fail:
        stream_close(is);
        return NULL;
    }
    return is;
}

/**
 *这个函数的目的是在多媒体播放时,根据用户的操作,切换到下一个指定类型的流(视频、音频或字幕)
 * @param is
 * @param codec_type
 */

static void stream_cycle_channel(VideoState *is, int codec_type)
{
    AVFormatContext *ic = is->ic;
    int start_index;
    int old_index;

    int nb_streams = (int )is->ic->nb_streams;

    if (codec_type == AVMEDIA_TYPE_VIDEO) {
        start_index = is->last_video_stream;
        old_index = is->video_stream;
    } else if (codec_type == AVMEDIA_TYPE_AUDIO) {
        start_index = is->last_audio_stream;
        old_index = is->audio_stream;
    } else {
        start_index = is->last_subtitle_stream;
        old_index = is->subtitle_stream;
    }

    int stream_index = start_index;

    AVProgram *p = NULL;
    if (codec_type != AVMEDIA_TYPE_VIDEO && is->video_stream != -1) {
        p = av_find_program_from_stream(ic, NULL, is->video_stream);
        if (p) {
            nb_streams = p->nb_stream_indexes;
            for (start_index = 0; start_index < nb_streams; start_index++){
                if (p->stream_index[start_index] == stream_index){
                    break;
                }
            }

            if (start_index == nb_streams){
                start_index = -1;
            }

            stream_index = start_index;
        }
    }

    for (;;) {
        if (++stream_index >= nb_streams){
            if (codec_type == AVMEDIA_TYPE_SUBTITLE){
                stream_index = -1;
                is->last_subtitle_stream = -1;
                goto the_end;
            }

            if (start_index == -1){
                return;
            }

            stream_index = 0;
        }

        if (stream_index == start_index){
            return;
        }

        AVStream *st = is->ic->streams[p ? p->stream_index[stream_index] : stream_index];

        if (st->codecpar->codec_type == codec_type) {
            /* check that parameters are OK */
            switch (codec_type) {
            case AVMEDIA_TYPE_AUDIO:
                if (st->codecpar->sample_rate != 0 &&
                    st->codecpar->ch_layout.nb_channels != 0){
                    goto the_end;
                }
                break;
            case AVMEDIA_TYPE_VIDEO:
            case AVMEDIA_TYPE_SUBTITLE:
                goto the_end;
            default:
                break;
            }
        }
    }

 the_end:
    if (p && stream_index != -1){
        stream_index = p->stream_index[stream_index];
    }

    av_log(nullptr, AV_LOG_INFO, "Switch %s stream from #%d to #%d\n",
           av_get_media_type_string(static_cast<AVMediaType>(codec_type)),
           old_index,
           stream_index);

    stream_component_close(is, old_index);
    stream_component_open(is, stream_index);
}

/**
 * 进入全屏和退出全屏
 * @param is
 */
static void toggle_full_screen(VideoState *is)
{
    is_full_screen = !is_full_screen;
    SDL_SetWindowFullscreen(window, is_full_screen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

/**
 *
 * @param is
 */
static void toggle_audio_display(VideoState *is)
{
    int next = is->show_mode;

    do {
        next = (next + 1) % VideoState ::SHOW_MODE_NB;
    } while (next != is->show_mode && (next == VideoState ::SHOW_MODE_VIDEO && !is->video_st || next != VideoState ::SHOW_MODE_VIDEO && !is->audio_st));

    if (is->show_mode != next) {
        is->force_refresh = 1;
        is->show_mode = static_cast<VideoState::ShowMode>(next);
    }
}

/**
 * 刷新循环事件
 * @param is
 * @param event
 */
static void refresh_loop_wait_event(VideoState *is, SDL_Event *event) {

    double remaining_time = 0.0;
    //休眠等待,remaining_time的计算在video_refresh中

    SDL_PumpEvents();

    /*
     * SDL_PeepEvents check是否事件,比如鼠标移入显示区等
     * 从事件队列中拿一个事件,放到event中,如果没有事件,则进入循环中
     * SDL_PeekEvents用于读取事件,在调用该函数之前,必须调用SDL_PumpEvents搜集键盘等事件
     */

    while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {

        if (!cursor_hidden && av_gettime_relative() - cursor_last_shown > CURSOR_HIDE_DELAY) {
            SDL_ShowCursor(0);
            cursor_hidden = 1;
        }

        /*
         * remaining_time就是用来进行音视频同步的
         * 在video_refresh函数中,根据当前帧显示时刻(display time)和实际时刻(actual time)
         * 计算需要sleep的时间,保证帧按时显示
         */

        if (remaining_time > 0.0){ //sleep控制画面输出的时机
            av_usleep((int64_t)(remaining_time * 1000000.0)); // remaining_time <= REFRESH_RATE
        }

        remaining_time = REFRESH_RATE;

        // 显示模式不等于SHOW_MODE_NONE , 非暂停状态 ,强制刷新状态
        if (is->show_mode != VideoState ::SHOW_MODE_NONE && (!is->paused || is->force_refresh)){
            video_refresh(is, &remaining_time);
        }
        /* 从输入设备中搜集事件,推动这些事件进入事件队列,更新事件队列的状态,
         * 不过它还有一个作用是进行视频子系统的设备状态更新,如果不调用这个函数,
         * 所显示的视频会在大约10秒后丢失色彩,没有调用SDL_PumpEvents,将不会
         * 有任何的输入设备事件进入队列,这种情况下,SDL就无法响应任何的键盘等硬件输入。
        */

        SDL_PumpEvents();
    }
}

/**
 * seek节目章节
 * @param is
 * @param incr
 */

static void seek_chapter(VideoState *is, int incr)
{
    int64_t pos = get_master_clock(is) * AV_TIME_BASE;

    if (!is->ic->nb_chapters){
        return;
    }

    /* find the current chapter */
    int i;
    for (i = 0; i < is->ic->nb_chapters; i++) {
        AVChapter *ch = is->ic->chapters[i];
        if (av_compare_ts(pos, AV_TIME_BASE_Q, ch->start, ch->time_base) < 0) {
            i--;
            break;
        }
    }

    i += incr;
    i = FFMAX(i, 0);
    if (i >= is->ic->nb_chapters){
        return;
    }

    av_log(NULL, AV_LOG_VERBOSE, "Seeking to chapter %d.\n", i);
    stream_seek(is, av_rescale_q(is->ic->chapters[i]->start, is->ic->chapters[i]->time_base,
                                 AV_TIME_BASE_Q), 0, 0);
}

/* handle an event sent by the GUI */
/**
 * 事件循环
 * @param cur_stream
 */
static void event_loop(VideoState *cur_stream)
{
    SDL_Event event;
    double incr, pos, frac;

    for (;;) {
        double x;
        refresh_loop_wait_event(cur_stream, &event); //video是在这里显示的
        switch (event.type) { //键盘事件
        case SDL_KEYDOWN:
            if (exit_on_keydown || event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                do_exit(cur_stream);
                break;
            }
            // If we don't yet have a window, skip all key events, because read_thread might still be initializing...
            //如果我们还没有窗口，请跳过所有关键事件，因为 read_thread 可能仍在初始化...
            if (!cur_stream->width){
                continue;
            }

            switch (event.key.keysym.sym) {
            case SDLK_f://全屏
                toggle_full_screen(cur_stream);
                cur_stream->force_refresh = 1;
                break;
            case SDLK_p://p健
            case SDLK_SPACE: //按空格键触发暂停/恢复
                toggle_pause(cur_stream);
                break;
            case SDLK_m://静音
                toggle_mute(cur_stream);
                break;
            case SDLK_KP_MULTIPLY:
            case SDLK_0://音量增大
                update_volume(cur_stream, 1, SDL_VOLUME_STEP);
                break;
            case SDLK_KP_DIVIDE:
            case SDLK_9://音量减小
                update_volume(cur_stream, -1, SDL_VOLUME_STEP);
                break;
            case SDLK_s: // S: Step to next frame
                //逐帧播放
                step_to_next_frame(cur_stream);
                break;
            case SDLK_a://切换音频流,粤语<===>国语
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
                break;
            case SDLK_v://切换视频流
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);
                break;
            case SDLK_c://所有流一起切换
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE);
                break;
            case SDLK_t://切换字幕流
                stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE);
                break;
            case SDLK_w://切换视频滤镜
                if (cur_stream->show_mode == VideoState ::SHOW_MODE_VIDEO && cur_stream->vfilter_idx < nb_vfilters - 1) {
                    if (++cur_stream->vfilter_idx >= nb_vfilters){
                        cur_stream->vfilter_idx = 0;
                    }
                } else {
                    cur_stream->vfilter_idx = 0;
                    toggle_audio_display(cur_stream); //
                }
                break;
            case SDLK_PAGEUP://方向上,快进60s
                if (cur_stream->ic->nb_chapters <= 1) {
                    incr = 600.0;
                    goto do_seek;
                }
                seek_chapter(cur_stream, 1);
                break;
            case SDLK_PAGEDOWN: //方向下,快退60s
                if (cur_stream->ic->nb_chapters <= 1) {
                    incr = -600.0;
                    goto do_seek;
                }
                seek_chapter(cur_stream, -1);
                break;
            case SDLK_LEFT://方向左,快退10s
                incr = seek_interval ? -seek_interval : -10.0;
                goto do_seek;
            case SDLK_RIGHT://方向右,快进10s
                incr = seek_interval ? seek_interval : 10.0;
                goto do_seek;
            case SDLK_UP: //方向上,快进60s
                incr = 60.0;
                goto do_seek;
            case SDLK_DOWN: //方向下,快退60s
                incr = -60.0;
            do_seek:
                    if (seek_by_bytes) { //按字节seek
                        pos = -1;
                        if (pos < 0 && cur_stream->video_stream >= 0){
                            pos = frame_queue_last_pos(&cur_stream->pictq);
                        }

                        if (pos < 0 && cur_stream->audio_stream >= 0){
                            pos = frame_queue_last_pos(&cur_stream->sampq);
                        }

                        if (pos < 0){
                            pos = avio_tell(cur_stream->ic->pb);
                        }

                        if (cur_stream->ic->bit_rate){
                            incr *= cur_stream->ic->bit_rate / 8.0;
                        }else{
                            incr *= 180000.0;
                        }

                        pos += incr;
                        stream_seek(cur_stream, pos, incr, 1);
                    } else { //按时间seek
                        pos = get_master_clock(cur_stream);

                        if (isnan(pos)){
                            pos = (double)cur_stream->seek_pos / AV_TIME_BASE;
                        }

                        pos += incr; // 现在是秒的单位
                        if (cur_stream->ic->start_time != AV_NOPTS_VALUE && pos < cur_stream->ic->start_time / (double)AV_TIME_BASE) {
                            //此处做一个时间校准,pos位置比流的起始时间还小,则用流的起始时间进行校准
                            pos = cur_stream->ic->start_time / (double)AV_TIME_BASE;
                        }

                        stream_seek(cur_stream, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
                        //这里会把时间换算成微妙,因为ffmpeg内部接受的是微秒
                    }
                break;
            default:
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN: /* 鼠标按下事件 */
            if (exit_on_mousedown) {
                do_exit(cur_stream);
                break;
            }
            if (event.button.button == SDL_BUTTON_LEFT) { //鼠标左键双击,进入和退出全屏
                static int64_t last_mouse_left_click = 0;
                if (av_gettime_relative() - last_mouse_left_click <= 500000) {
                    toggle_full_screen(cur_stream);
                    cur_stream->force_refresh = 1;
                    last_mouse_left_click = 0;
                } else {
                    last_mouse_left_click = av_gettime_relative();
                }
            }
        case SDL_MOUSEMOTION: //鼠标移动
            if (cursor_hidden) {
                SDL_ShowCursor(1);
                cursor_hidden = 0;
            }
            cursor_last_shown = av_gettime_relative();
            if (event.type == SDL_MOUSEBUTTONDOWN) { //
                if (event.button.button != SDL_BUTTON_RIGHT){ //不是右键按下,没有效果
                    break;
                }
                x = event.button.x;
            } else {
                if (!(event.motion.state & SDL_BUTTON_RMASK)){
                    break;
                }
                x = event.motion.x;
            }
            if (seek_by_bytes || cur_stream->ic->duration <= 0) {
                //cur_stream->ic->duration <= 0 小于当前流的总持续时间,从文件
                uint64_t size = avio_size(cur_stream->ic->pb); //整个文件的字节
                stream_seek(cur_stream, size*x/cur_stream->width, 0, 1);
                //用当前整个文件大小和鼠标相对坐标相乘,这个时候就要用字节去seek
            } else {
                int64_t ts;
                int ns, hh, mm, ss;
                int tns, thh, tmm, tss;
                tns  = cur_stream->ic->duration / 1000000LL;
                thh  = tns / 3600;
                tmm  = (tns % 3600) / 60;
                tss  = (tns % 60);
                frac = x / cur_stream->width;
                ns   = frac * tns;
                hh   = ns / 3600;
                mm   = (ns % 3600) / 60;
                ss   = (ns % 60);
                av_log(NULL, AV_LOG_INFO,
                       "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", frac*100,
                        hh, mm, ss, thh, tmm, tss);
                ts = frac * cur_stream->ic->duration;
                if (cur_stream->ic->start_time != AV_NOPTS_VALUE){
                    ts += cur_stream->ic->start_time;
                }
                stream_seek(cur_stream, ts, 0, 0);
            }
            break;
        case SDL_WINDOWEVENT: /* 窗口事件 */
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: //窗口大小改变
                    screen_width  = cur_stream->width  = event.window.data1;
                    screen_height = cur_stream->height = event.window.data2;
                    if (cur_stream->vis_texture) {
                        SDL_DestroyTexture(cur_stream->vis_texture);
                        cur_stream->vis_texture = NULL;
                    }
                    if (vk_renderer){
                        vk_renderer_resize(vk_renderer, screen_width, screen_height);
                    }
                case SDL_WINDOWEVENT_EXPOSED: //窗口裸露
                    cur_stream->force_refresh = 1;
            }
            break;
        case SDL_QUIT:
        case FF_QUIT_EVENT: /* ffplay自定义事件,用于主动退出 */
            do_exit(cur_stream);
            break;
        default:
            break;
        }
    }
}

static int opt_width(void *optctx, const char *opt, const char *arg)
{
    double num;
    int ret = parse_number(opt, arg, OPT_TYPE_INT64, 1, INT_MAX, &num);
    if (ret < 0){
        return ret;
    }

    screen_width = num;
    return 0;
}

static int opt_height(void *optctx, const char *opt, const char *arg)
{
    double num;
    int ret = parse_number(opt, arg, OPT_TYPE_INT64, 1, INT_MAX, &num);
    if (ret < 0){
        return ret;
    }

    screen_height = num;
    return 0;
}

static int opt_format(void *optctx, const char *opt, const char *arg)
{
    file_iformat = av_find_input_format(arg);
    if (!file_iformat) {
        av_log(NULL, AV_LOG_FATAL, "Unknown input format: %s\n", arg);
        return AVERROR(EINVAL);
    }
    return 0;
}

static int opt_sync(void *optctx, const char *opt, const char *arg)
{
    if (!strcmp(arg, "audio")){
        av_sync_type = AV_SYNC_AUDIO_MASTER;
    }else if (!strcmp(arg, "video")){
        av_sync_type = AV_SYNC_VIDEO_MASTER;
    }else if (!strcmp(arg, "ext")){
        av_sync_type = AV_SYNC_EXTERNAL_CLOCK;
    }else {
        av_log(NULL, AV_LOG_ERROR, "Unknown value for %s: %s\n", opt, arg);
        exit(1);
    }
    return 0;
}

static int opt_show_mode(void *optctx, const char *opt, const char *arg)
{
    show_mode = !strcmp(arg, "video") ? VideoState::SHOW_MODE_VIDEO :
                !strcmp(arg, "waves") ? VideoState::SHOW_MODE_WAVES :
                !strcmp(arg, "rdft" ) ? VideoState::SHOW_MODE_RDFT  : VideoState::SHOW_MODE_NONE;

    if (show_mode == VideoState::SHOW_MODE_NONE) {
        double num;
        int ret = parse_number(opt, arg, OPT_TYPE_INT, 0, VideoState::SHOW_MODE_NB - 1, &num);
        if (ret < 0){
            return ret;
        }
        show_mode = static_cast<VideoState::ShowMode>(num);
    }
    return 0;
}

static int opt_input_file(void *optctx, const char *filename)
{
    if (input_filename) {
        av_log(nullptr, AV_LOG_FATAL,
               "Argument '%s' provided as input filename, but '%s' was already specified.\n",
                filename, input_filename);
        return AVERROR(EINVAL);
    }
    if (!strcmp(filename, "-")){
        filename = "fd:";
    }

    input_filename = av_strdup(filename);

    if (!input_filename){
        return AVERROR(ENOMEM);
    }

    return 0;
}

static int opt_codec(void *optctx, const char *opt, const char *arg)
{
   const char *spec = strchr(opt, ':');

   if (!spec) {
       av_log(NULL, AV_LOG_ERROR,
              "No media specifier was specified in '%s' in option '%s'\n",
               arg, opt);
       return AVERROR(EINVAL);
   }
   spec++;
   const char **name = NULL;
   switch (spec[0]) {
   case 'a' : name = &audio_codec_name;    break;
   case 's' : name = &subtitle_codec_name; break;
   case 'v' : name = &video_codec_name;    break;
   default:
       av_log(NULL, AV_LOG_ERROR,
              "Invalid media specifier '%s' in option '%s'\n", spec, opt);
       return AVERROR(EINVAL);
   }

   av_freep(name);
   *name = av_strdup(arg);
   return *name ? 0 : AVERROR(ENOMEM);
}

static int dummy;

static const OptionDef options[] = {
    CMDUTILS_COMMON_OPTIONS
    { "x",                  OPT_TYPE_FUNC, OPT_FUNC_ARG, { .func_arg = opt_width }, "force displayed width", "width" },
    { "y",                  OPT_TYPE_FUNC, OPT_FUNC_ARG, { .func_arg = opt_height }, "force displayed height", "height" },
    { "fs",                 OPT_TYPE_BOOL,            0, { &is_full_screen }, "force full screen" },
    { "an",                 OPT_TYPE_BOOL,            0, { &audio_disable }, "disable audio" },
    { "vn",                 OPT_TYPE_BOOL,            0, { &video_disable }, "disable video" },
    { "sn",                 OPT_TYPE_BOOL,            0, { &subtitle_disable }, "disable subtitling" },
    { "ast",                OPT_TYPE_STRING, OPT_EXPERT, { &wanted_stream_spec[AVMEDIA_TYPE_AUDIO] }, "select desired audio stream", "stream_specifier" },
    { "vst",                OPT_TYPE_STRING, OPT_EXPERT, { &wanted_stream_spec[AVMEDIA_TYPE_VIDEO] }, "select desired video stream", "stream_specifier" },
    { "sst",                OPT_TYPE_STRING, OPT_EXPERT, { &wanted_stream_spec[AVMEDIA_TYPE_SUBTITLE] }, "select desired subtitle stream", "stream_specifier" },
    { "ss",                 OPT_TYPE_TIME,            0, { &start_time }, "seek to a given position in seconds", "pos" },
    { "t",                  OPT_TYPE_TIME,            0, { &duration }, "play  \"duration\" seconds of audio/video", "duration" },
    { "bytes",              OPT_TYPE_INT,             0, { &seek_by_bytes }, "seek by bytes 0=off 1=on -1=auto", "val" },
    { "seek_interval",      OPT_TYPE_FLOAT,           0, { &seek_interval }, "set seek interval for left/right keys, in seconds", "seconds" },
    { "nodisp",             OPT_TYPE_BOOL,            0, { &display_disable }, "disable graphical display" },
    { "noborder",           OPT_TYPE_BOOL,            0, { &borderless }, "borderless window" },
    { "alwaysontop",        OPT_TYPE_BOOL,            0, { &alwaysontop }, "window always on top" },
    { "volume",             OPT_TYPE_INT,             0, { &startup_volume}, "set startup volume 0=min 100=max", "volume" },
    { "f",                  OPT_TYPE_FUNC, OPT_FUNC_ARG, { .func_arg = opt_format }, "force format", "fmt" },
    { "stats",              OPT_TYPE_BOOL,   OPT_EXPERT, { &show_status }, "show status", "" },
    { "fast",               OPT_TYPE_BOOL,   OPT_EXPERT, { &fast }, "non spec compliant optimizations", "" },
    { "genpts",             OPT_TYPE_BOOL,   OPT_EXPERT, { &genpts }, "generate pts", "" },
    { "drp",                OPT_TYPE_INT,    OPT_EXPERT, { &decoder_reorder_pts }, "let decoder reorder pts 0=off 1=on -1=auto", ""},
    { "lowres",             OPT_TYPE_INT,    OPT_EXPERT, { &lowres }, "", "" },
    { "sync",               OPT_TYPE_FUNC, OPT_FUNC_ARG | OPT_EXPERT, { .func_arg = opt_sync }, "set audio-video sync. type (type=audio/video/ext)", "type" },
    { "autoexit",           OPT_TYPE_BOOL,   OPT_EXPERT, { &autoexit }, "exit at the end", "" },
    { "exitonkeydown",      OPT_TYPE_BOOL,   OPT_EXPERT, { &exit_on_keydown }, "exit on key down", "" },
    { "exitonmousedown",    OPT_TYPE_BOOL,   OPT_EXPERT, { &exit_on_mousedown }, "exit on mouse down", "" },
    { "loop",               OPT_TYPE_INT,    OPT_EXPERT, { &loop }, "set number of times the playback shall be looped", "loop count" },
    { "framedrop",          OPT_TYPE_BOOL,   OPT_EXPERT, { &framedrop }, "drop frames when cpu is too slow", "" },
    { "infbuf",             OPT_TYPE_BOOL,   OPT_EXPERT, { &infinite_buffer }, "don't limit the input buffer size (useful with realtime streams)", "" },
    { "window_title",       OPT_TYPE_STRING,          0, { &window_title }, "set window title", "window title" },
    { "left",               OPT_TYPE_INT,    OPT_EXPERT, { &screen_left }, "set the x position for the left of the window", "x pos" },
    { "top",                OPT_TYPE_INT,    OPT_EXPERT, { &screen_top }, "set the y position for the top of the window", "y pos" },
    { "vf",                 OPT_TYPE_FUNC, OPT_FUNC_ARG | OPT_EXPERT, { .func_arg = opt_add_vfilter }, "set video filters", "filter_graph" },
    { "af",                 OPT_TYPE_STRING,          0, { &afilters }, "set audio filters", "filter_graph" },
    { "rdftspeed",          OPT_TYPE_INT, OPT_AUDIO | OPT_EXPERT, { &rdftspeed }, "rdft speed", "msecs" },
    { "showmode",           OPT_TYPE_FUNC, OPT_FUNC_ARG, { .func_arg = opt_show_mode}, "select show mode (0 = video, 1 = waves, 2 = RDFT)", "mode" },
    { "i",                  OPT_TYPE_BOOL,            0, { &dummy}, "read specified file", "input_file"},
    { "codec",              OPT_TYPE_FUNC, OPT_FUNC_ARG, { .func_arg = opt_codec}, "force decoder", "decoder_name" },
    { "acodec",             OPT_TYPE_STRING, OPT_EXPERT, {    &audio_codec_name }, "force audio decoder",    "decoder_name" },
    { "scodec",             OPT_TYPE_STRING, OPT_EXPERT, { &subtitle_codec_name }, "force subtitle decoder", "decoder_name" },
    { "vcodec",             OPT_TYPE_STRING, OPT_EXPERT, {    &video_codec_name }, "force video decoder",    "decoder_name" },
    { "autorotate",         OPT_TYPE_BOOL,            0, { &autorotate }, "automatically rotate video", "" },
    { "find_stream_info",   OPT_TYPE_BOOL, OPT_INPUT | OPT_EXPERT, { &find_stream_info },
        "read and decode the streams to fill missing information with heuristics" },
    { "filter_threads",     OPT_TYPE_INT,    OPT_EXPERT, { &filter_nbthreads }, "number of filter threads per graph" },
    { "enable_vulkan",      OPT_TYPE_BOOL,            0, { &enable_vulkan }, "enable vulkan renderer" },
    { "vulkan_params",      OPT_TYPE_STRING, OPT_EXPERT, { &vulkan_params }, "vulkan configuration using a list of key=value pairs separated by ':'" },
    { "hwaccel",            OPT_TYPE_STRING, OPT_EXPERT, { &hwaccel }, "use HW accelerated decoding" },
    { NULL, },
};

static void show_usage(void)
{
    av_log(NULL, AV_LOG_INFO, "Simple media player\n");
    av_log(NULL, AV_LOG_INFO, "usage: %s [options] input_file\n", program_name);
    av_log(NULL, AV_LOG_INFO, "\n");
}

/**
 * ffplay帮助说明
 * @param opt
 * @param arg
 */
void show_help_default(const char *opt, const char *arg)
{
    av_log_set_callback(log_callback_help);
    show_usage();
    show_help_options(options, "Main options:", 0, OPT_EXPERT);
    show_help_options(options, "Advanced options:", OPT_EXPERT, 0);
    printf("\n");
    show_help_children(avcodec_get_class(), AV_OPT_FLAG_DECODING_PARAM);
    show_help_children(avformat_get_class(), AV_OPT_FLAG_DECODING_PARAM);
    show_help_children(avfilter_get_class(), AV_OPT_FLAG_FILTERING_PARAM);
    printf("\nWhile playing:\n"
           "q, ESC              quit\n"
           "f                   toggle full screen\n"
           "p, SPC              pause\n"
           "m                   toggle mute\n"
           "9, 0                decrease and increase volume respectively\n"
           "/, *                decrease and increase volume respectively\n"
           "a                   cycle audio channel in the current program\n"
           "v                   cycle video channel\n"
           "t                   cycle subtitle channel in the current program\n"
           "c                   cycle program\n"
           "w                   cycle video filters or show modes\n"
           "s                   activate frame-step mode\n"
           "left/right          seek backward/forward 10 seconds or to custom interval if -seek_interval is set\n"
           "down/up             seek backward/forward 1 minute\n"
           "page down/page up   seek backward/forward 10 minutes\n"
           "right mouse click   seek to percentage in file corresponding to fraction of width\n"
           "left double-click   toggle full screen\n"
           );
}

/* Called from the main */
int main(int argc, char **argv)
{
    int ret;
    VideoState *is;

    init_dynload();

    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    parse_loglevel(argc, argv, options);

    /* register all codecs, demux and protocols */
#if CONFIG_AVDEVICE
    avdevice_register_all();
#endif
    avformat_network_init();

    //注册程序终止信号
    signal(SIGINT , sigterm_handler); /* Interrupt (ANSI).    */
    signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */

    show_banner(argc, argv, options);
    //对传递的参数进行初始化
    ret = parse_options(NULL, argc, argv, options, opt_input_file);

    if (ret < 0){
        exit(ret == AVERROR_EXIT ? 0 : 1);
    }

    if (!input_filename) {
        show_usage();
        av_log(NULL, AV_LOG_FATAL, "An input file must be specified\n");
        av_log(NULL, AV_LOG_FATAL,
               "Use -h to get full help or, even better, run 'man %s'\n", program_name);
        exit(1);
    }

    /* 是否显示视频 */
    if (display_disable) {
        video_disable = 1;
    }

    //SDL的初始化
    int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

    if (audio_disable){ //没有音频
        flags &= ~SDL_INIT_AUDIO;
    }else {
        /* Try to work around an occasional ALSA buffer underflow issue when the
         * period size is NPOT due to ALSA resampling by forcing the buffer size. */
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE")){
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE","1", 1);
        }
    }

    if (display_disable){ //如果显示关闭
        flags &= ~SDL_INIT_VIDEO;
    }

    if (SDL_Init (flags)) { //初始化SDL
        av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
        exit(1);
    }

    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE); //忽略系统特定事件
    SDL_EventState(SDL_USEREVENT, SDL_IGNORE); //忽略用户事件

    if (!display_disable) { //如果有显示,创建窗口
        int flags = SDL_WINDOW_HIDDEN;
        if (alwaysontop) {
#if SDL_VERSION_ATLEAST(2, 0, 5)
            flags |= SDL_WINDOW_ALWAYS_ON_TOP;
#else
            av_log(NULL, AV_LOG_WARNING, "Your SDL version doesn't support SDL_WINDOW_ALWAYS_ON_TOP. Feature will be inactive.\n");
#endif
        }

        if (borderless){
            flags |= SDL_WINDOW_BORDERLESS;
        }else{
            flags |= SDL_WINDOW_RESIZABLE;
        }

#ifdef SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif
        if (hwaccel && !enable_vulkan) {
            av_log(NULL, AV_LOG_INFO, "Enable vulkan renderer to support hwaccel %s\n", hwaccel);
            enable_vulkan = 1;
        }
        if (enable_vulkan) {
            vk_renderer = vk_get_renderer();
            if (vk_renderer) {
#if SDL_VERSION_ATLEAST(2, 0, 6)
                flags |= SDL_WINDOW_VULKAN;
#endif
            } else {
                av_log(NULL, AV_LOG_WARNING, "Doesn't support vulkan renderer, fallback to SDL renderer\n");
                enable_vulkan = 0;
            }
        }

        window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, default_width, default_height, flags);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

        if (!window) {
            av_log(NULL, AV_LOG_FATAL, "Failed to create window: %s", SDL_GetError());
            do_exit(NULL);
        }

        if (vk_renderer) { //有硬件加速则采用硬件加速
            AVDictionary *dict = NULL;

            if (vulkan_params){
                av_dict_parse_string(&dict, vulkan_params, "=", ":", 0);
            }

            ret = vk_renderer_create(vk_renderer, window, dict);
            av_dict_free(&dict);
            if (ret < 0) {
                char errbuf[AV_ERROR_MAX_STRING_SIZE]{};
                av_log(NULL, AV_LOG_FATAL, "Failed to create vulkan renderer, %s\n", av_make_error_string(errbuf,AV_ERROR_MAX_STRING_SIZE,ret));
                do_exit(NULL);
            }
        } else {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); //创建渲染器
            if (!renderer) {
                av_log(NULL, AV_LOG_WARNING, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
                renderer = SDL_CreateRenderer(window, -1, 0);
            }
            if (renderer) {
                if (!SDL_GetRendererInfo(renderer, &renderer_info))
                    av_log(NULL, AV_LOG_VERBOSE, "Initialized %s renderer.\n", renderer_info.name);
            }
            if (!renderer || !renderer_info.num_texture_formats) {
                av_log(NULL, AV_LOG_FATAL, "Failed to create window or renderer: %s", SDL_GetError());
                do_exit(NULL);
            }
        }
    }
    //通过stream_open函数,开启read_thread读取线程
    is = stream_open(input_filename, file_iformat);

    if (!is) {
        av_log(NULL, AV_LOG_FATAL, "Failed to initialize VideoState!\n");
        do_exit(NULL);
    }

    //事件响应
    event_loop(is);

    /* never returns */

    return 0;
}
