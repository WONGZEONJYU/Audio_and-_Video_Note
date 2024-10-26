#include "xcamera_config.hpp"
#include <fstream>

using namespace std;

XCameraConfig *XCameraConfig::Instance() {
    static XCameraConfig instance;
    return addressof(instance);
}

void XCameraConfig::Push(const XCameraData &data) {
    unique_lock lock(m_mutex_);
    m_cams_.push_back(data);
}

XCameraData XCameraConfig::GetCam(const int &index) const {
    if (index < 0 || index >= m_cams_.size()){
        return {};
    }
    unique_lock lock(const_cast<decltype(m_mutex_)&>(m_mutex_));
    return m_cams_[index];
}

uint32_t XCameraConfig::GetCamCount() const {
    unique_lock lock(const_cast<decltype(m_mutex_)&>(m_mutex_));
    return m_cams_.size();
}

bool XCameraConfig::SetCam(const int &index, const XCameraData &data){
    if (index < 0 || index >= m_cams_.size()){
        return {};
    }
    unique_lock lock(m_mutex_);
    m_cams_[index] = data;
    return true;
}

bool XCameraConfig::DelCam(const int &index){
    if (index < 0 || index >= m_cams_.size()){
        return {};
    }
    unique_lock lock(m_mutex_);
    m_cams_.erase(m_cams_.begin() + index);
    return true;
}

bool XCameraConfig::Save(const string_view &path) const{

    if (path.empty()){
        PRINT_ERR_TIPS(GET_STR(path error!));
        return {};
    }

    ofstream ofs(path.data(),ios::binary);
    if (!ofs){
        PRINT_ERR_TIPS(GET_STR(path open failed!));
        return {};
    }
    unique_lock lock(const_cast<decltype(m_mutex_)&>(m_mutex_));
    for (const auto &item:m_cams_) {
        ofs.write(reinterpret_cast<const char *>(addressof(item)), sizeof(item));
    }
    ofs.close();
    return true;
}

bool XCameraConfig::Load(const string_view &path) {

    if (path.empty()){
        PRINT_ERR_TIPS(GET_STR(path error!));
        return {};
    }

    ifstream ifs(path.data(),ios::binary);
    if (!ifs){
        PRINT_ERR_TIPS(GET_STR(path open failed!));
        return {};
    }

    XCameraData data;
    unique_lock lock(m_mutex_);
    m_cams_.clear();
    while (true){
        ifs.read(reinterpret_cast<char *>(addressof(data)), sizeof(data));
        if (sizeof(data) != ifs.gcount()){
            ifs.close();
            return true;
        }
        m_cams_.push_back(data);
    }
}
