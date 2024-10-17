#ifndef INC_124_TEST_XFORMAT_XDEMUX_HPP
#define INC_124_TEST_XFORMAT_XDEMUX_HPP

#include "xformat.hpp"

class XAVPacket;

class XDemux : public XFormat{

public:
    static AVFormatContext *Open(const std::string &url);

    /**
     * 读取一包数据,线程安全
     * @param packet
     * @return ture or false
     */
    bool Read(XAVPacket &packet);

    /**
     * Seek音视频帧
     * @param pts
     * @param stream_index
     * @return
     */
    bool Seek(const int64_t &pts,const int &stream_index);

    explicit XDemux() = default;
    X_DISABLE_COPY_MOVE(XDemux)
};

#endif //INC_124_TEST_XFORMAT_XDEMUX_HPP
