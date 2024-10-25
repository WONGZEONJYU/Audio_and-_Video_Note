#include "xcamera_config.hpp"
using namespace std;

XCameraConfig *XCameraConfig::Instance() {
    static XCameraConfig instance;
    return addressof(instance);
}

void XCameraConfig::Push(const XCameraDate &date) {
    unique_lock lock(m_mutex_);
    m_cams_.push_back(date);
}

XCameraDate XCameraConfig::GetCam(const uint32_t &index) const {
    if (index >= m_cams_.size()) {
        return {};
    }
    unique_lock lock(const_cast<decltype(m_mutex_)&>(m_mutex_));
    return m_cams_[index];
}

uint32_t XCameraConfig::GetCamCount() const {
    unique_lock lock(const_cast<decltype(m_mutex_)&>(m_mutex_));
    return m_cams_.size();
}
