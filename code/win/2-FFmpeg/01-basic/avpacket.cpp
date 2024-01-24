#include "avpacket.h"
#include <iostream>

static constexpr auto MEM_ITEM_SIZE {20*1024*102};
//static constexpr auto AVPACKET_LOOP_COUNT {1000};

// 测试 内存泄漏
/**
 * @brief 测试av_packet_alloc和av_packet_free的配对使用
 */
static void av_packet_test1()
{
    auto pkt{av_packet_alloc()};
    av_new_packet(pkt, MEM_ITEM_SIZE); // 引用计数初始化为1
    memccpy(pkt->data, (void *)&av_packet_test1, 1, MEM_ITEM_SIZE);
    av_packet_unref(pkt);       // 要不要调用
    av_packet_free(&pkt);       // 如果不free将会发生内存泄漏,内部调用了 av_packet_unref
}

/**
 * @brief 测试误用av_init_packet将会导致内存泄漏
 */
static void av_packet_test2()
{
    auto pkt {av_packet_alloc()};
    av_new_packet(pkt, MEM_ITEM_SIZE);
    memccpy(pkt->data, (void *)&av_packet_test1, 1, MEM_ITEM_SIZE);
//    av_init_packet(pkt);        // 这个时候init就会导致内存无法释放
    /*av_init_packet会把引用计数置为0,av_packet_free就不会释放*/
    /*ffmpeg库提示该函数被弃用*/
    av_packet_free(&pkt);
}

/**
 * @brief 测试av_packet_move_ref后，可以av_init_packet
 */
static void av_packet_test3()
{
    auto pkt {av_packet_alloc()};
    av_new_packet(pkt, MEM_ITEM_SIZE);
    memccpy(pkt->data, (void *)&av_packet_test1, 1, MEM_ITEM_SIZE);
    auto pkt2 {av_packet_alloc()};   // 必须先alloc
    av_packet_move_ref(pkt2, pkt);//内部其实也调用了av_init_packet
    av_init_packet(pkt);
    av_packet_free(&pkt);
    av_packet_free(&pkt2);
}

/**
 * @brief 测试av_packet_clone
 */
static void av_packet_test4()
{
    // av_packet_alloc()没有必要，因为av_packet_clone内部有调用 av_packet_alloc
    auto pkt {av_packet_alloc()};
    av_new_packet(pkt, MEM_ITEM_SIZE);
    memccpy(pkt->data, (void *)&av_packet_test1, 1, MEM_ITEM_SIZE);
    auto pkt2 {av_packet_clone(pkt)}; // av_packet_alloc() + av_packet_ref()
    av_init_packet(pkt);
    av_packet_free(&pkt);
    av_packet_free(&pkt2);
}

/**
 * @brief 测试av_packet_ref
 */
static void av_packet_test5()
{
    auto pkt {av_packet_alloc()}; //
    if(pkt->buf){       // 打印referenc-counted，必须保证传入的是有效指针
        /*printf("%s(%d) ref_count(pkt) = %d\n", __FUNCTION__, __LINE__,
               av_buffer_get_ref_count(pkt->buf));*/
        std::cout << __FUNCTION__ << " (" << __LINE__ << ") " << "ref_count(pkt) = " <<
                     av_buffer_get_ref_count(pkt->buf) << "\n";
    }

    av_new_packet(pkt, MEM_ITEM_SIZE);

    if(pkt->buf){        // 打印referenc-counted，必须保证传入的是有效指针
        /*printf("%s(%d) ref_count(pkt) = %d\n", __FUNCTION__, __LINE__,
               av_buffer_get_ref_count(pkt->buf));*/
        std::cout << __FUNCTION__ << " (" << __LINE__ << ") " << "ref_count(pkt) = " <<
                     av_buffer_get_ref_count(pkt->buf) << "\n";
    }

    memccpy(pkt->data, (void *)&av_packet_test1, 1, MEM_ITEM_SIZE);

    auto pkt2 {av_packet_alloc()};   // 必须先alloc
    av_packet_move_ref(pkt2, pkt); // av_packet_move_ref
//    av_init_packet(pkt);  //av_packet_move_ref

    av_packet_ref(pkt, pkt2);
    av_packet_ref(pkt, pkt2);     // 多次ref如果没有对应多次unref将会内存泄漏
    if(pkt->buf){      // 打印referenc-counted，必须保证传入的是有效指针
        /*printf("%s(%d) ref_count(pkt) = %d\n", __FUNCTION__, __LINE__,
               av_buffer_get_ref_count(pkt->buf));*/
        std::cout << __FUNCTION__ << " (" << __LINE__ << ") " << "ref_count(pkt) = " <<
                     av_buffer_get_ref_count(pkt->buf) << "\n";
    }
    if(pkt2->buf) {       // 打印referenc-counted，必须保证传入的是有效指针
    /*    printf("%s(%d) ref_count(pkt) = %d\n", __FUNCTION__, __LINE__,
               av_buffer_get_ref_count(pkt2->buf));*/
        std::cout << __FUNCTION__ << " (" << __LINE__ << ") " << "ref_count(pkt) = " <<
                     av_buffer_get_ref_count(pkt2->buf) << "\n";
    }
    av_packet_unref(pkt);   // 将为2
    av_packet_unref(pkt);   // 做第二次是没有用的
    if(pkt->buf){
        std::cout << std::string("pkt->buf没有被置NULL\n");
    }else{
        std::cout << std::string("pkt->buf已经被置NULL\n");
    }

    if(pkt2->buf) {       // 打印referenc-counted，必须保证传入的是有效指针
        /*printf("%s(%d) ref_count(pkt) = %d\n", __FUNCTION__, __LINE__,
               av_buffer_get_ref_count(pkt2->buf));*/
        std::cout << __FUNCTION__ << " (" << __LINE__ << ") " << "ref_count(pkt) = " <<
                     av_buffer_get_ref_count(pkt2->buf) << "\n";
    }

    av_packet_unref(pkt2);
    av_packet_free(&pkt);
    av_packet_free(&pkt2);
}

/**
 * @brief 测试AVPacket整个结构体赋值, 和av_packet_move_ref类似
 */
static void av_packet_test6()
{
    auto pkt {av_packet_alloc()};
    av_new_packet(pkt, MEM_ITEM_SIZE);
    memccpy(pkt->data, (void *)&av_packet_test1, 1, MEM_ITEM_SIZE);

    auto pkt2 {av_packet_alloc()};   // 必须先alloc
    *pkt2 = *pkt;   // 有点类似 pkt可以重新分配内存
    av_init_packet(pkt);
    av_packet_free(&pkt);
    av_packet_free(&pkt2);
}

void av_packet_test(){
    //av_packet_test1();
    //av_packet_test2();
    //av_packet_test3();
    //av_packet_test4();
    av_packet_test5();
    //av_packet_test6();
}
