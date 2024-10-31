#ifndef XCAMERA_RECORDE_HPP_
#define XCAMERA_RECORDE_HPP_

#include <xtools.hpp>

class XCameraRecorde final: public XThread {
    void Main() override;
public:
    inline void set_rtsp_url(const std::string &rtsp_url) {
        //std::unique_lock locker(m_mutex_);
        m_rtsp_url_ = rtsp_url;
    }

    inline void set_save_path(const std::string &save_path) {
        //std::unique_lock locker(m_mutex_);
        m_save_path_ = save_path;
    }

private:
    std::mutex m_mutex_;
    std::string m_rtsp_url_,
        m_save_path_;
public:
    explicit XCameraRecorde() = default;
    X_DISABLE_COPY_MOVE(XCameraRecorde)
};

#endif