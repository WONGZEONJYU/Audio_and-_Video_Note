#ifndef XCAMERA_RECORD_HPP_
#define XCAMERA_RECORD_HPP_

#include <xtools.hpp>

class XCameraRecord final: public XThread {
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

    inline void set_file_sec(const int &file_sec) {
        //std::unique_lock locker(m_mutex_);
        m_file_src_ = file_sec;
    }

private:
    std::mutex m_mutex_;
    std::string m_rtsp_url_,
        m_save_path_;
    uint64_t m_file_src_{5}; //多少秒创建一个新文件

public:
    explicit XCameraRecord() = default;
    ~XCameraRecord() override = default;
    X_DISABLE_COPY_MOVE(XCameraRecord)
};

#endif
