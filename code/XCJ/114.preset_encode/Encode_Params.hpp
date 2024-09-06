//
// Created by Administrator on 2024/4/26.
//

#ifndef INC_15_MP4_MUXER_H264_ENCODE_PARAMS_HPP
#define INC_15_MP4_MUXER_H264_ENCODE_PARAMS_HPP

#define PARAMETERS(x) #x

struct PRESET {
    static inline constexpr auto first{PARAMETERS(preset)};
protected:
    constexpr PRESET() = default;
};

static inline constexpr struct ULTRAFAST final : PRESET {
    inline constexpr ULTRAFAST() = default;
    static inline constexpr auto second{PARAMETERS(ultrafast)};
}ultrafast{};

static inline constexpr struct SUPERFAST final : PRESET {
    inline constexpr SUPERFAST() = default;
    static inline constexpr auto second{PARAMETERS(superfast)};
}superfast{};

static inline constexpr struct VERYFAST final : PRESET{
    inline constexpr VERYFAST() = default;
    static inline constexpr auto second{PARAMETERS(veryfast)};
}veryfast{};

static inline constexpr struct FASTER final : PRESET{
    inline constexpr FASTER() = default;
    static inline constexpr auto second{PARAMETERS(faster)};
}faster{};

static inline constexpr struct FAST final : PRESET{
    inline constexpr FAST() = default;
    static inline constexpr auto second{PARAMETERS(fast)};
}fast{};

static inline constexpr struct MEDIUM final : PRESET{
    inline constexpr MEDIUM() = default;
    static inline constexpr auto second{PARAMETERS(medium)};
}medium{};

static inline constexpr struct SLOW final : PRESET{
    inline constexpr SLOW() = default;
    static inline constexpr auto second{PARAMETERS(slow)};
}slow{};

static inline constexpr struct SLOWER final : PRESET{
    inline constexpr SLOWER() = default;
    static inline constexpr auto second{PARAMETERS(slower)};
}slower{};

static inline constexpr struct VERYSLOW final : PRESET{
    inline constexpr VERYSLOW() = default;
    static inline constexpr auto second{PARAMETERS(veryslow)};
}veryslow{};

static inline constexpr struct PLACEBO final : PRESET{
    inline constexpr PLACEBO() = default;
    static inline constexpr auto second{PARAMETERS(placebo)};
}placebo{};

struct TUNE {
    static inline constexpr auto first{PARAMETERS(tune)};
protected:
    inline constexpr TUNE() = default;
};

static inline constexpr struct FILM final : TUNE{
    inline constexpr FILM() = default;
    static inline constexpr auto second{PARAMETERS(film)};
}film{};

static inline constexpr struct ANIMATION final : TUNE{
    inline constexpr ANIMATION() = default;
    static inline constexpr auto second{PARAMETERS(animation)};
}animation{};

static inline constexpr struct GRAIN final : TUNE{
    inline constexpr GRAIN() = default;
    static inline constexpr auto second{PARAMETERS(grain)};
}grain{};

static inline constexpr struct STILLIMAGE final : TUNE {
    inline constexpr STILLIMAGE() = default;
    static inline constexpr auto second{PARAMETERS(stillimage)};
}stillimage{};

static inline constexpr struct PSNR final : TUNE{
    inline constexpr PSNR() = default;
    static inline constexpr auto second{PARAMETERS(psnr)};
}psnr{};

static inline constexpr struct SSIM final : TUNE {
    inline constexpr SSIM() = default;
    static inline constexpr auto second{PARAMETERS(ssim)};
}ssim{};

static inline constexpr struct FASTDECODE final : TUNE{
    inline constexpr FASTDECODE() = default;
    static inline constexpr auto second{PARAMETERS(fastdecode)};
} fastdecode{};

static inline constexpr struct ZEROLATENCY final : TUNE{
    inline constexpr ZEROLATENCY() = default;
    static inline constexpr auto second{PARAMETERS(zerolatency)};
}zerolatency{};

struct PROFILE {
    static inline constexpr auto first{PARAMETERS(profile)};
protected:
    inline constexpr PROFILE() = default;
};

static inline constexpr struct BASELINE final : PROFILE{
    inline constexpr BASELINE() = default;
    static inline constexpr auto second{PARAMETERS(baseline)};
}baseline{};

static inline constexpr struct EXTENDED final :PROFILE{
    inline constexpr EXTENDED() = default;
    static inline constexpr auto second{PARAMETERS(extended)};
}extended{};

static inline constexpr struct MAIN final : PROFILE{
    inline constexpr MAIN() = default;
    static inline constexpr auto second{PARAMETERS(main)};
}main_{};

static inline constexpr struct HIGH final :PROFILE{
    inline constexpr HIGH() = default;
    static inline constexpr auto second{PARAMETERS(high)};
}high{};

struct QP final {
    static inline constexpr auto m_name{PARAMETERS(qp)};
    explicit inline constexpr QP(const long long &v):m_value{v}{}
    inline constexpr operator auto() const{return m_value;}
    [[nodiscard]] inline constexpr auto value() const{return m_value;}
private:
    long long m_value{};
};

struct NAL_HRD {
    static inline constexpr auto first{PARAMETERS(nal-hrd)};
protected:
    inline constexpr NAL_HRD() = default;
};

static inline constexpr struct CBR final : NAL_HRD{
    static inline constexpr auto second{PARAMETERS(cbr)};
    inline constexpr CBR() = default;
}cbr{};

struct CRF final {
    static inline constexpr auto m_name{PARAMETERS(crf)};
    inline constexpr explicit CRF(const long long &v):m_value{v}{}
    inline constexpr operator auto (){return m_value;}
    [[nodiscard]] inline constexpr auto value() const{return m_value;}
private:
    long long m_value{};
};

#endif
