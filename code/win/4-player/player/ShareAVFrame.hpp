//
// Created by Administrator on 2024/4/19.
//

#ifndef INC_15_MP4_MUXER_SHAREAVFRAME_HPP
#define INC_15_MP4_MUXER_SHAREAVFRAME_HPP

struct AVFrame;
#include <memory>

struct ShareAVFrame final {

    using ShareAVFrame_sp_type = std::shared_ptr<ShareAVFrame>;
    static ShareAVFrame_sp_type create() noexcept(false);
    ShareAVFrame(const ShareAVFrame&) = delete;
    ShareAVFrame& operator=(const ShareAVFrame&) = delete;
    ~ShareAVFrame() noexcept(true);
    explicit operator AVFrame* () {return m_frame;}
    AVFrame* operator->(){return m_frame;}
    AVFrame operator*() {return *m_frame;}
private:
    explicit ShareAVFrame() noexcept(true);
    void Construct() const noexcept(false);
    void DeConstruct() noexcept(true);
    AVFrame * m_frame{};
};

using ShareAVFrame_sp_type = typename ShareAVFrame::ShareAVFrame_sp_type;

ShareAVFrame_sp_type new_ShareAVFrame() noexcept(false);

#endif //INC_15_MP4_MUXER_SHAREAVFRAME_HPP
