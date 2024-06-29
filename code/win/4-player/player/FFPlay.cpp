//
// Created by Administrator on 2024/6/27.
//

#include "FFPlay.hpp"


void FFPlay::prepare_async(const std::string &url) {
    m_url = url;
    //无需初始化队列,队列在构造的时候已经构造完成
    //stream_open()
}
