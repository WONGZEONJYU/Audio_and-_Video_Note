//
// Created by Administrator on 2024/7/1.
//

#ifndef PLAYER_FF_FFPLAY_DEF_HPP
#define PLAYER_FF_FFPLAY_DEF_HPP

extern "C"{
#include <libavutil/avstring.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/mem.h>
#include <libavutil/pixdesc.h>
#include <libavutil/dict.h>
#include <libavutil/fifo.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libavutil/bprint.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/tx.h>
#include <libswresample/swresample.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include <SDL.h>
#include <SDL_thread.h>

struct MyAVPacketList { /*PacketQueue队列存放的元素模型*/
    AVPacket *pkt;  /*解封装后的packet*/
    int serial;     /*播放序列,用于快进快退*/
};

struct PacketQueue { /*包队列*/
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
};

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
struct Clock {
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
};

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
struct FrameQueue {
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
};

/* packet queue handling */
/*队列初始化*/
int packet_queue_init(PacketQueue *q);

/*清空packet_queue*/
void packet_queue_flush(PacketQueue *q);

/*销毁packet_queue*/
void packet_queue_destroy(PacketQueue *q);

/*packet_queue请求退出*/
void packet_queue_abort(PacketQueue *q);

/*packet_queue启动*/
void packet_queue_start(PacketQueue *q);

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
/*packet_queue读取函数*/
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);

/*packet_queue放入空包*/
int packet_queue_put_nullpacket(PacketQueue *q, AVPacket *pkt, int stream_index);

/*入队函数*/
int packet_queue_put(PacketQueue *q, AVPacket *pkt);



/*
 * frame_queue初始化,音视频的keep_last设置为1,字幕的keep_last设置为0
 */
int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last);
/*
 * 销毁frame_queue
 */
void frame_queue_destroy(FrameQueue *f);
/*
 * 窥探位置(f->rindex + f->rindex_shown)的数据
 * 如果f->rindex_shown为0,frame_queue_peek与frame_queue_peek_last读取的位置一样
 */
Frame *frame_queue_peek(FrameQueue *f);

/*
 * 窥探位置(f->rindex + f->rindex_shown + 1)的数据
 */
Frame *frame_queue_peek_next(FrameQueue *f);

/*
 * 窥探位置f->rindex的数据
 */
Frame *frame_queue_peek_last(FrameQueue *f);

/*
 * peek 出一个可以写的 Frame,此函数可能会阻塞
 */
Frame *frame_queue_peek_writable(FrameQueue *f);

/*
 * peek 出一个可以准备播放的 Frame,此函数可能会阻塞
 */
Frame *frame_queue_peek_readable(FrameQueue *f);

/*
 * 更新f->windex
 */
void frame_queue_push(FrameQueue *f);

/*
 * 先释放frame,并更新f->rindex
 * 首次调用,如果keep_last为1, rindex_show为0时不去更新f->rindex,也不释放当前frame
 * f->keep_last字段就是用来控制f->rindex_shown是否需要置1
 */
void frame_queue_next(FrameQueue *f);

/* return the number of undisplayed frames in the queue */
//返回队列中未显示的帧数
int frame_queue_nb_remaining(FrameQueue *f);

/* return last shown position */
//返回最后显示的位置
int64_t frame_queue_last_pos(FrameQueue *f);

#endif //PLAYER_FF_FFPLAY_DEF_HPP
