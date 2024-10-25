#ifndef XCAMERACONFIG_HPP
#define XCAMERACONFIG_HPP

#include <string_view>
#include <xhelper.hpp>

struct XCameraDate {
    std::string_view m_name_{},
    m_url{},//摄像头主码流
    m_sub_url{},//摄像头子码流
    m_save_path_{};//保存路径
};

class XCameraConfig final{

    explicit XCameraConfig() = default;
public:
    void Push(const XCameraDate &date);
    [[nodiscard]] XCameraDate GetCam(const uint32_t &index) const;
    [[nodiscard]] uint32_t GetCamCount() const;
    bool SetCam(const uint32_t &index, const XCameraDate &date);
    bool DelCam(const uint32_t &index);
    
private:
    std::vector<XCameraDate> m_cams_;
    std::mutex m_mutex_;
public:
    static XCameraConfig *Instance();
    X_DISABLE_COPY_MOVE(XCameraConfig)
};

#define XCamera_Config_() XCameraConfig::Instance()

#endif
