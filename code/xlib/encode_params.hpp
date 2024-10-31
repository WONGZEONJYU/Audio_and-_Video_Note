#ifndef ENCODE_PARAMS_HPP_
#define ENCODE_PARAMS_HPP_

#define PARAMETERS(x) #x

struct PRESET {
    static inline constexpr auto first{PARAMETERS(preset)};
protected:
    inline constexpr PRESET() = default;
};

static inline constexpr struct ULTRAFAST final : PRESET {
    inline constexpr ULTRAFAST() = default;
    static inline constexpr auto second{PARAMETERS(ultrafast)};
}ultrafast_{};

static inline constexpr struct SUPERFAST final : PRESET {
    inline constexpr SUPERFAST() = default;
    static inline constexpr auto second{PARAMETERS(superfast)};
}superfast_{};

static inline constexpr struct VERYFAST final : PRESET{
    inline constexpr VERYFAST() = default;
    static inline constexpr auto second{PARAMETERS(veryfast)};
}veryfast_{};

static inline constexpr struct FASTER final : PRESET{
    inline constexpr FASTER() = default;
    static inline constexpr auto second{PARAMETERS(faster)};
}faster_{};

static inline constexpr struct FAST final : PRESET{
    inline constexpr FAST() = default;
    static inline constexpr auto second{PARAMETERS(fast)};
}fast_{};

static inline constexpr struct MEDIUM final : PRESET{
    inline constexpr MEDIUM() = default;
    static inline constexpr auto second{PARAMETERS(medium)};
}medium_{};

static inline constexpr struct SLOW final : PRESET{
    inline constexpr SLOW() = default;
    static inline constexpr auto second{PARAMETERS(slow)};
}slow_{};

static inline constexpr struct SLOWER final : PRESET{
    inline constexpr SLOWER() = default;
    static inline constexpr auto second{PARAMETERS(slower)};
}slower_{};

static inline constexpr struct VERYSLOW final : PRESET{
    inline constexpr VERYSLOW() = default;
    static inline constexpr auto second{PARAMETERS(veryslow)};
}veryslow_{};

static inline constexpr struct PLACEBO final : PRESET{
    inline constexpr PLACEBO() = default;
    static inline constexpr auto second{PARAMETERS(placebo)};
}placebo_{};

struct TUNE {
    static inline constexpr auto first{PARAMETERS(tune)};
protected:
    inline constexpr TUNE() = default;
};

static inline constexpr struct FILM final : TUNE{
    inline constexpr FILM() = default;
    static inline constexpr auto second{PARAMETERS(film)};
}film_{};

static inline constexpr struct ANIMATION final : TUNE{
    inline constexpr ANIMATION() = default;
    static inline constexpr auto second{PARAMETERS(animation)};
}animation_{};

static inline constexpr struct GRAIN final : TUNE{
    inline constexpr GRAIN() = default;
    static inline constexpr auto second{PARAMETERS(grain)};
}grain_{};

static inline constexpr struct STILLIMAGE final : TUNE {
    inline constexpr STILLIMAGE() = default;
    static inline constexpr auto second{PARAMETERS(stillimage)};
}stillimage_{};

static inline constexpr struct PSNR final : TUNE{
    inline constexpr PSNR() = default;
    static inline constexpr auto second{PARAMETERS(psnr)};
}psnr_{};

static inline constexpr struct SSIM final : TUNE {
    inline constexpr SSIM() = default;
    static inline constexpr auto second{PARAMETERS(ssim)};
}ssim_{};

static inline constexpr struct FASTDECODE final : TUNE{
    inline constexpr FASTDECODE() = default;
    static inline constexpr auto second{PARAMETERS(fastdecode)};
} fastdecode_{};

static inline constexpr struct ZEROLATENCY final : TUNE{
    inline constexpr ZEROLATENCY() = default;
    static inline constexpr auto second{PARAMETERS(zerolatency)};
}zerolatency_{};

struct PROFILE {
    static inline constexpr auto first{PARAMETERS(profile)};
protected:
    inline constexpr PROFILE() = default;
};

static inline constexpr struct BASELINE final : PROFILE{
    inline constexpr BASELINE() = default;
    static inline constexpr auto second{PARAMETERS(baseline)};
}baseline_{};

static inline constexpr struct EXTENDED final :PROFILE{
    inline constexpr EXTENDED() = default;
    static inline constexpr auto second{PARAMETERS(extended)};
}extended_{};

static inline constexpr struct MAIN final : PROFILE{
    inline constexpr MAIN() = default;
    static inline constexpr auto second{PARAMETERS(main)};
}main_{};

static inline constexpr struct HIGH final :PROFILE{
    inline constexpr HIGH() = default;
    static inline constexpr auto second{PARAMETERS(high)};
}high_{};

struct NAL_HRD {
    static inline constexpr auto first{PARAMETERS(nal-hrd)};
protected:
    inline constexpr NAL_HRD() = default;
};

static inline constexpr struct CBR final : NAL_HRD{
    static inline constexpr auto second{PARAMETERS(cbr)};
    inline constexpr CBR() = default;
}cbr_{};

struct QP_CRF_Base {
    [[nodiscard]] inline constexpr auto value() const{return m_value_;}
    [[nodiscard]] inline constexpr explicit operator auto() const{return m_value_;}
protected:
    inline constexpr explicit QP_CRF_Base(const long long &v):m_value_(v){}
private:
    long long m_value_{};
};

struct QP final : QP_CRF_Base {
    static inline constexpr auto m_name{PARAMETERS(qp)};
    inline constexpr explicit QP(const long long &v):QP_CRF_Base(v){}
};

struct CRF final : QP_CRF_Base {
    static inline constexpr auto m_name{PARAMETERS(crf)};
    inline constexpr explicit CRF(const long long &v):QP_CRF_Base(v){}
};

#endif
