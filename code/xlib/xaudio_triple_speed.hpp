#ifndef XAUDIOTRIPLESPEED_HPP
#define XAUDIOTRIPLESPEED_HPP

#include "xsonic.hpp"

class XLIB_API Audio_Playback_Speed {

X_DISABLE_COPY_MOVE(Audio_Playback_Speed)

public:
    explicit Audio_Playback_Speed() = default;
    inline bool Open(const int &sampleRate,const int &numChannels) {
        std::unique_lock locker(m_mux_);
        m_is_init_ = m_son_.Open(sampleRate,numChannels);
        return m_is_init_;
    }

    inline void Set_Speed(const double &speed) {

        if (!m_is_init_) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            return;
        }
        std::unique_lock locker(m_mux_);
        m_son_.sonicSetSpeed(static_cast<float>(speed));
    }

    [[nodiscard]] inline auto sonicSamplesAvailable() const {
        if (!m_is_init_) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            return -1;
        }
        std::unique_lock locker(const_cast<decltype(m_mux_) &>(m_mux_));
        return m_son_.sonicSamplesAvailable();
    }

    template<typename T>
    inline bool Send(const T *src,const int &samples) {

        if (!m_is_init_) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            return {};
        }

        std::unique_lock locker(m_mux_);

        if constexpr (std::is_same_v<T,float>) {
            return m_son_.sonicWriteFloatToStream(src,samples);
        }else if constexpr (std::is_same_v<T,short>) {
            return m_son_.sonicWriteShortToStream(src,samples);
        }else if constexpr (std::is_same_v<T,uint8_t>) {
            return m_son_.sonicWriteUnsignedCharToStream(src,samples);
        } else if constexpr (std::is_same_v<T,int8_t>){
            return m_son_.sonicWriteCharToStream(src,samples);
        }else if constexpr (std::is_same_v<T,uint16_t>) {
            return m_son_.sonicWriteUnsignedShortToStream(src,samples);
        }else if constexpr (std::is_same_v<T,uint32_t>) {
            return m_son_.sonicWriteU32ToStream(src,samples);
        }else if constexpr (std::is_same_v<T,int32_t>) {
            return m_son_.sonicWriteS32ToStream(src,samples);
        }else if constexpr (std::is_same_v<T,uint64_t>) {
            return m_son_.sonicWriteU64ToStream(src,samples);
        }else if constexpr (std::is_same_v<T,int64_t>) {
            return m_son_.sonicWriteS64ToStream(src,samples);
        }else if constexpr (std::is_same_v<T,double>) {
            return m_son_.sonicWriteDoubleToStream(src,samples);
        }else {
            static_assert(false,GET_STR("not support type"));
        }
        return {};
    }

    template<typename T>
    inline int Receive(T *dst,const int &samples) {

        if (!m_is_init_) {
            PRINT_ERR_TIPS(GET_STR(Uninitialized));
            return -1;
        }

        std::unique_lock locker(m_mux_);

        if constexpr (std::is_same_v<T,float>) {
            return m_son_.sonicReadFloatFromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,short>) {
            return m_son_.sonicReadShortFromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,uint8_t>) {
            return m_son_.sonicReadUnsignedCharFromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,int8_t>) {
            return m_son_.sonicReadSignedCharFromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,uint16_t>) {
            return m_son_.sonicReadUnsignedShortFromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,uint32_t>) {
            return m_son_.sonicReadU32FromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,int32_t>) {
            return m_son_.sonicReadS32FromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,uint64_t>) {
            return m_son_.sonicReadU64FromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,int64_t>) {
            return m_son_.sonicReadS64FromStream(dst,samples);
        }else if constexpr (std::is_same_v<T,double>) {
            return m_son_.sonicReadDoubleFromStream(dst,samples);
        }else {
            static_assert(false,GET_STR("not support type"));
            return -1;
        }
    }

    [[nodiscard]] auto get_channels() const {
        std::unique_lock locker(const_cast<decltype(m_mux_) &>(m_mux_));
        return m_son_.sonicGetNumChannels();
    }

    [[nodiscard]] auto get_samples_rate() const {
        std::unique_lock locker(const_cast<decltype(m_mux_) &>(m_mux_));
        return m_son_.sonicGetSampleRate();
    }

    inline explicit operator bool() const {
        return m_is_init_;
    }

    inline auto operator !() const {
        return !m_is_init_;
    }

    [[maybe_unused]] bool FlushStream(){
        return m_son_.sonicFlushStream();
    }

private:
    std::mutex m_mux_;
    XSonic m_son_;
    std::atomic_bool m_is_init_{};
};

#endif
