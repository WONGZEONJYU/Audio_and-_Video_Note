#ifndef AVPACKET_H
#define AVPACKET_H

extern "C"{
#include "libavcodec/avcodec.h"
}


/*
//简单的创建一个AVPacket，将其字段设为默认值（data为空，没有数据缓存空间），data的指针需要另外去赋值。
AVPacket *av_packet_alloc(void);

int av_new_packet(AVPacket *pkt, int size);
//释放使用av_packet_alloc创建的AVPacket，如果该Packet有引用计数（packet->buf不为空），则先调用av_packet_unref(&packet)。
//只有当引用计数为0时，才会在调用av_packet_free()时释放data的缓存。
void av_packet_free(AVPacket **pkt);
// 初始化packet的值为默认值，该函数不会影响buffer引用的数据缓存空间和size，需要单独处理。AVPacket中的buf为空。
// 比如 av_get_packet里调用av_init_packet
void av_init_packet(AVPacket *pkt);

// 使用引用计数的浅拷贝
// 该函数会先拷贝所有非缓存类数据，然后创建一个src->buf的新的引用计数。如果src已经设置了引用计数发（src->buf不为空），则直接将其引用计数+1；
// 如果src没有设置引用计数（src->buf为空），则为dst创建一个新的引用计数buf，并复制src->data到buf->buffer中。
// 最后，复制src的其他字段到dst中。所以av_packet_ref()是将2个AVPacket共用一个缓存的。
int av_packet_ref(AVPacket *dst, const AVPacket *src);

// 使用引用计数清理数据
// 将缓存空间的引用计数-1，并将Packet中的其他字段设为初始值。如果引用计数为0，自动的释放缓存空间。
void av_packet_unref(AVPacket *pkt);

// 把src整个结构体直接赋值给dst，所以引用计数没有发生变化，并且src被av_init_packet重置
void av_packet_move_ref(AVPacket *dst, AVPacket *src);

//其功能是 av_packet_alloc() + av_packet_ref()；
//先创建一个新的AVPacket，然后再进行计数引用+数据拷贝，使得新的AVPacket指向老的AVPacket同一个data。
AVPacket *av_packet_clone(const AVPacket *src);
*/


void av_packet_test();


#endif // AVPACKET_H
