#ifndef XCAMERA_RECODE_HPP_
#define XCAMERA_RECODE_HPP_

#include <xtools.hpp>

class XCameraRecode final: public XThread {
    void Main() override;
public:
    void set_rtsp_url(const std::string &rtsp_url) {
        std::unique_lock locker(m_mutex_);
        m_rtsp_url_ = rtsp_url;
    }

    void set_save_path(const std::string &save_path) {
        std::unique_lock locker(m_mutex_);
        m_save_path_ = save_path;
    }

private:
    std::mutex m_mutex_;
    std::string m_rtsp_url_,
        m_save_path_;
public:
    explicit XCameraRecode() = default;
    X_DISABLE_COPY_MOVE(XCameraRecode)
};
















#endif