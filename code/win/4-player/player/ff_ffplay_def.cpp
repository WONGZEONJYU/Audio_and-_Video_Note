//
// Created by Administrator on 2024/7/1.
//

#include "ff_ffplay_def.hpp"
#include <algorithm>

#define PacketQueue_
#define FrameQueue_

#if defined(PacketQueue_)

/* packet queue handling */
/*队列初始化*/
int packet_queue_init(PacketQueue *q)
{
    //memset(q, 0, sizeof(PacketQueue));
    std::fill_n(reinterpret_cast<uint8_t*>(q),sizeof(PacketQueue),0);
    q->pkt_list = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
    if (!q->pkt_list){
        return AVERROR(ENOMEM);
    }

    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->cond = SDL_CreateCond();
    if (!q->cond) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    q->abort_request = 1;
    return 0;
}

/*清空packet_queue*/
void packet_queue_flush(PacketQueue *q)
{
    MyAVPacketList pkt1{};

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
void packet_queue_destroy(PacketQueue *q)
{
    packet_queue_flush(q);
    av_fifo_freep2(&q->pkt_list);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

/*packet_queue请求退出*/
void packet_queue_abort(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;//请求退出
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

/*packet_queue启动*/
void packet_queue_start(PacketQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    q->serial++;
    SDL_UnlockMutex(q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
/*packet_queue读取函数*/
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial)
{
    MyAVPacketList pkt1{};
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

/*入队的实际操作*/
static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
    MyAVPacketList pkt1{};
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
int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    auto pkt1{av_packet_alloc()};
    int ret;

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
int packet_queue_put_nullpacket(PacketQueue *q, AVPacket *pkt, int stream_index)
{
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

#endif

#if defined(FrameQueue_)
/*
 * frame_queue初始化,音视频的keep_last设置为1,字幕的keep_last设置为0
 */
int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last)
{
    //memset(f, 0, sizeof(FrameQueue));
    std::fill_n(reinterpret_cast<uint8_t*>(f),sizeof(FrameQueue),0);
    if (!(f->mutex = SDL_CreateMutex())) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    if (!(f->cond = SDL_CreateCond())) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);//最大不能超过16个元素 FRAME_QUEUE_SIZE=16
    f->keep_last = !!keep_last; //用于控制字段rindex_shown的生成,详情阅读frame_queue_next函数

    for (int i {}; i < f->max_size; i++) {
        //给每个AVFrame分配空间
        if (!(f->queue[i].frame = av_frame_alloc())){
            return AVERROR(ENOMEM);
        }
    }

    return 0;
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
 * 销毁frame_queue
 */
void frame_queue_destroy(FrameQueue *f)
{
    for (int i {}; i < f->max_size; i++) {
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
Frame *frame_queue_peek(FrameQueue *f)
{
    //f->rindex_shown是0还是1,影响着改API是读取当前元素还是下一元素
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/*
 * 窥探位置(f->rindex + f->rindex_shown + 1)的数据
 */
Frame *frame_queue_peek_next(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

/*
 * 窥探位置f->rindex的数据
 */
Frame *frame_queue_peek_last(FrameQueue *f)
{
    return &f->queue[f->rindex];
}

/*
 * peek 出一个可以写的 Frame,此函数可能会阻塞
 */
Frame *frame_queue_peek_writable(FrameQueue *f)
{
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);

    while (f->size >= f->max_size && !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }

    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request){
        return nullptr;
    }

    return &f->queue[f->windex];
}

/*
 * peek 出一个可以准备播放的 Frame,此函数可能会阻塞
 */
Frame *frame_queue_peek_readable(FrameQueue *f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request) {
        return nullptr;
    }

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/*
 * 更新f->windex
 */
void frame_queue_push(FrameQueue *f)
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
void frame_queue_next(FrameQueue *f)
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
int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;//为什么要减去f->rindex_shown,因为他是返回还未显示的帧而不是队列还有多少帧
    //f->rindex_shown的数量代表已经显示了多少帧
}

/* return last shown position */
//返回最后显示的位置
int64_t frame_queue_last_pos(FrameQueue *f)
{
    Frame *fp = &f->queue[f->rindex];
    if (f->rindex_shown && fp->serial == f->pktq->serial) {
        return fp->pos;
    }else {
        return -1;
    }
}

#endif

/**
 * 获取到的实际上是:最后一帧的pts + 从处理最后一帧开始到现在的时间,具体参考set_clock_at和get_clock的代码
 * c->pts_drift = 最后一帧的pts - 从处理最后一帧时间
 * clock=c->pts_drift+现在的时候
 * get_clock(&is->vidclk) == is->vidclk.pts , av_gettime_relative() / 1000000.0 - is->vidclk.last_updated + is->vidclk.pts
 */
double get_clock(Clock *c)
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
void set_clock_at(Clock *c, double pts, int serial, double time)
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
void set_clock(Clock *c, double pts, int serial)
{
    double time = (double )av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

/**
 * 设置速度
 * @param c
 * @param speed
 */
void set_clock_speed(Clock *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

/**
 * 初始化时钟
 * @param c
 * @param queue_serial
 */
void init_clock(Clock *c, int *queue_serial)
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
void sync_clock_to_slave(Clock *c, Clock *slave)
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

/**
 * 获取主时钟的时间值
 * @param is
 * @return current master clock value
 */





