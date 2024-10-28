#ifndef XCAMERA_CONFIG_HPP_
#define XCAMERA_CONFIG_HPP_

#include <xhelper.hpp>
#include <string_view>

struct XCameraData {
    char m_name_[4096]{}, //摄像头名称
    m_url[sizeof(m_name_)]{},//摄像头主码流
    m_sub_url[sizeof(m_name_)]{},//摄像头子码流
    m_save_path[sizeof(m_name_)]{};//保存路径
};

class XCameraConfig final{

    explicit XCameraConfig() = default;
public:
    void Push(const XCameraData &date);
    [[nodiscard]] XCameraData GetCam(const int &index) const;
    [[nodiscard]] uint32_t GetCamCount() const;
    bool SetCam(const int &index, const XCameraData &data);
    bool DelCam(const int &index);
    [[nodiscard]]bool Save(const std::string_view &path) const;
    [[nodiscard]]bool Load(const std::string_view &path);

private:
    std::vector<XCameraData> m_cams_;
    std::mutex m_mutex_;
public:
    static XCameraConfig *Instance();
    X_DISABLE_COPY_MOVE(XCameraConfig)
};

#define XCamera_Config_() XCameraConfig::Instance()

#endif
