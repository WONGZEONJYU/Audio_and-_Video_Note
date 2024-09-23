//
// Created by wong on 2024/9/22.
//

#ifndef INC_124_TEST_XFORMAT_XMUX_HPP
#define INC_124_TEST_XFORMAT_XMUX_HPP

#include "xformat.hpp"

class XAVPacket;

class XMux : public XFormat{

public:
    static AVFormatContext *Open(const std::string &url);

    /**
     * 写入头部信息
     * @return ture or false
     */
    bool WriteHead();

    /**
     * 写入包数据
     * @param packet
     * @return ture or false
     */
    bool Write(XAVPacket &packet);

    /**
     * 写入尾部信息
     * @return ture or false
     */
    bool WriteEnd();

public:
    explicit XMux() = default;
    X_DISABLE_COPY_MOVE(XMux)

};


#endif
