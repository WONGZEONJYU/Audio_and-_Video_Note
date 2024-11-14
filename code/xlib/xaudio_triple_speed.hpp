#ifndef XAUDIOTRIPLESPEED_HPP
#define XAUDIOTRIPLESPEED_HPP

#include "xsonic.hpp"

class XLIB_API Audio_Playback_Speed {

X_DISABLE_COPY_MOVE(Audio_Playback_Speed)

public:
    explicit Audio_Playback_Speed() = default;
    inline bool Open(const int &sampleRate,const int &numChannels) {
        is_init = m_son_.Open(sampleRate,numChannels);
        return is_init;
    }

    inline void Set_Speed(const double &speed) {
        if (is_init) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            m_son_.sonicSetSpeed(static_cast<float>(speed));
        }
    }

    [[nodiscard]] inline auto sonicSamplesAvailable() const {
        if (!is_init) {
            return -1;
        }
        return m_son_.sonicSamplesAvailable();
    }

    template<typename T>
    inline std::enable_if_t<std::is_fundamental_v<T>,bool>
    Send(const T *src,const int &sample) {

        if (!is_init) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            return {};
        }

        if constexpr (std::is_same_v<T,float>) {
            return m_son_.sonicWriteFloatToStream(src,sample);
        }else if constexpr (std::is_same_v<T,short>) {
            return m_son_.sonicWriteShortToStream(src,sample);
        }else if constexpr (std::is_same_v<T,uint8_t>) {
            return m_son_.sonicWriteUnsignedCharToStream(src,sample);
        } else {
            static_assert(false,GET_STR("not support type"));
        }
        return {};
    }

    template<typename T>
    inline std::enable_if_t<std::is_fundamental_v<T>,bool>
    Receive(T * dst,const int &sample) {

        if (!is_init) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            return {};
        }

        if constexpr (std::is_same_v<T,float>) {
            return m_son_.sonicReadFloatFromStream(dst,sample);
        }else if constexpr (std::is_same_v<T,short>) {
            return m_son_.sonicReadShortFromStream(dst,sample);
        }else if constexpr (std::is_same_v<T,uint8_t>) {
            return m_son_.sonicReadUnsignedCharFromStream(dst,sample);
        }else {
            static_assert(false,GET_STR("not support type"));
        }
        return {};
    }

private:
    XSonic m_son_;
    bool is_init{};
};

#endif
