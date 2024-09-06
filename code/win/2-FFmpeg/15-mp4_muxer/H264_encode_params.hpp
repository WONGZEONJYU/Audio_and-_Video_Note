//
// Created by Administrator on 2024/4/26.
//

#ifndef INC_15_MP4_MUXER_H264_ENCODE_PARAMS_HPP
#define INC_15_MP4_MUXER_H264_ENCODE_PARAMS_HPP

#define PARAMETERS(x) #x

struct preset {
    static inline constexpr auto first{PARAMETERS(preset)};
protected:
    constexpr preset() = default;
};

struct preset_ultrafast_t final :  preset {
    constexpr preset_ultrafast_t() = default;
    static inline constexpr auto second{PARAMETERS(ultrafast)};
};

struct preset_superfast_t final :  preset {
    constexpr preset_superfast_t() = default;
    static inline constexpr auto second{PARAMETERS(superfast)};
};

struct preset_veryfast_t final :  preset{
    constexpr preset_veryfast_t() = default;
    static inline constexpr auto second{PARAMETERS(veryfast)};
};

struct preset_faster_t final : preset{
    constexpr preset_faster_t() = default;
    static inline constexpr auto second{PARAMETERS(faster)};
};

struct preset_fast_t final : preset{
    constexpr preset_fast_t() = default;
    static inline constexpr auto second{PARAMETERS(fast)};
};

struct preset_medium_t final :  preset{
    constexpr preset_medium_t() = default;
    static inline constexpr auto second{PARAMETERS(medium)};
};

struct preset_slow_t final :  preset{
    constexpr preset_slow_t() = default;
    static inline constexpr auto second{PARAMETERS(slow)};
};

struct preset_slower_t final :  preset{
    constexpr preset_slower_t() = default;
    static inline constexpr auto second{PARAMETERS(slower)};
};

struct preset_veryslow_t final :  preset{
    constexpr preset_veryslow_t() = default;
    static inline constexpr auto second{PARAMETERS(veryslow)};
};

struct preset_placebo_t final : preset{
    constexpr preset_placebo_t() = default;
    static inline constexpr auto second{PARAMETERS(placebo)};
};

static inline constexpr preset_ultrafast_t preset_ultrafast{};
static inline constexpr preset_superfast_t preset_superfast{};
static inline constexpr preset_veryfast_t preset_veryfast{};
static inline constexpr preset_faster_t preset_faster{};
static inline constexpr preset_fast_t preset_fast{};
static inline constexpr preset_medium_t preset_medium{};
static inline constexpr preset_slow_t preset_slow{};
static inline constexpr preset_slower_t preset_slower{};
static inline constexpr preset_veryslow_t preset_veryslow{};
static inline constexpr preset_placebo_t preset_placebo{};

struct tune {
    static inline constexpr auto first{PARAMETERS(tune)};
protected:
    constexpr tune() = default;
};

struct tune_film_t final : tune{
    constexpr tune_film_t() = default;
    static inline constexpr auto second{PARAMETERS(film)};
};

struct tune_animation_t final : tune{
    constexpr tune_animation_t() = default;
    static inline constexpr auto second{PARAMETERS(animation)};
};

struct tune_grain_t final : tune{
    constexpr tune_grain_t() = default;
    static inline constexpr auto second{PARAMETERS(grain)};
};

struct tune_stillimage_t final : tune{
    constexpr tune_stillimage_t() = default;
    static inline constexpr auto second{PARAMETERS(stillimage)};
};

struct tune_psnr_t final : tune{
    constexpr tune_psnr_t() = default;
    static inline constexpr auto second{PARAMETERS(psnr)};
};

struct tune_ssim_t final : tune{
    constexpr tune_ssim_t() = default;
    static inline constexpr auto second{PARAMETERS(ssim)};
};

struct tune_fastdecode_t final : tune{
    constexpr tune_fastdecode_t() = default;
    static inline constexpr auto second{PARAMETERS(fastdecode)};
};

struct tune_zerolatency_t final : tune{
    constexpr tune_zerolatency_t() = default;
    static inline constexpr auto second{PARAMETERS(zerolatency)};
};

static inline constexpr tune_film_t tune_film{};
static inline constexpr tune_animation_t tune_animation{};
static inline constexpr tune_grain_t tune_grain{};
static inline constexpr tune_stillimage_t tune_stillimage{};
static inline constexpr tune_psnr_t tune_psnr{};
static inline constexpr tune_ssim_t tune_ssim{};
static inline constexpr tune_fastdecode_t tune_fastdecode{};
static inline constexpr tune_zerolatency_t tune_zerolatency{};

struct profile {
    static inline constexpr auto first{PARAMETERS(profile)};
protected:
    constexpr profile() = default;
};

struct profile_baseline_t final :profile{
    constexpr profile_baseline_t() = default;
    static inline constexpr auto second{PARAMETERS(baseline)};
};

struct profile_extended_t final :profile{
    constexpr profile_extended_t() = default;
    static inline constexpr auto second{PARAMETERS(extended)};
};

struct profile_main_t final :profile{
    constexpr profile_main_t() = default;
    static inline constexpr auto second{PARAMETERS(main)};
};

struct profile_high_t final :profile{
    constexpr profile_high_t() = default;
    static inline constexpr auto second{PARAMETERS(high)};
};

static inline constexpr profile_baseline_t profile_baseline{};
static inline constexpr profile_extended_t profile_extended{};
static inline constexpr profile_main_t profile_main{};
static inline constexpr profile_high_t profile_high{};

#endif
