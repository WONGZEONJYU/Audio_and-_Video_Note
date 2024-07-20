//
// Created by Administrator on 2024/7/20.
//

#ifndef PLAYER_VIMAGE_HPP
#define PLAYER_VIMAGE_HPP

class VImage final {

public:
    VImage(const int &w,const int &h,unsigned char *data):
            m_w(w),m_h{h},m_data{data}{
    }

    VImage(VImage&& obj) noexcept:m_w{obj.m_w},m_h{obj.m_h},m_data{obj.m_data}{
        obj.m_w = 0;
        obj.m_h = 0;
        obj.m_data = nullptr;
    }

    VImage& operator=(VImage&& obj) noexcept{
        if (&obj != this){
            m_w = obj.m_w;
            m_h = obj.m_h;
            m_data = obj.m_data;
            obj.m_w = 0;
            obj.m_h = 0;
            obj.m_data = nullptr;
        }
        return *this;
    }

    [[nodiscard]] auto data() const{return m_data;}
    [[nodiscard]] auto width() const{return m_w;}
    [[nodiscard]] auto height() const{return m_h;}

private:
    unsigned char *m_data{};
    int m_w{},m_h{};
public:
    VImage(const VImage&) = delete;
    VImage& operator=(const VImage&) = delete;
};

#endif //PLAYER_VIMAGE_HPP
