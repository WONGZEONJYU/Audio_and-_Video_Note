//
// Created by Administrator on 2024/4/19.
//

#ifndef INC_15_MP4_MUXER_SHAREAVFRAME_HPP
#define INC_15_MP4_MUXER_SHAREAVFRAME_HPP

struct AVFrame;
#include <memory>

struct XShareAVFrame;
using ShareAVFrame_sp_type = std::shared_ptr<XShareAVFrame>;

struct XShareAVFrame final {

    static ShareAVFrame_sp_type create() noexcept(false);
    XShareAVFrame(const XShareAVFrame&) = delete;
    XShareAVFrame& operator=(const XShareAVFrame&) = delete;
    ~XShareAVFrame() noexcept(true);
    operator AVFrame* () const {return m_frame;}

    AVFrame* operator->() const {return m_frame;}
    AVFrame operator*() const {return *m_frame;}
private:
    explicit XShareAVFrame() noexcept(true);
    void Construct() const noexcept(false);
    void DeConstruct() noexcept(true);
    AVFrame * m_frame{};
};

ShareAVFrame_sp_type new_ShareAVFrame() noexcept(false);

#endif //INC_15_MP4_MUXER_SHAREAVFRAME_HPP
