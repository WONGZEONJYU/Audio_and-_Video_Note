#ifndef XADUIO_PARAMETERS_HPP
#define XADUIO_PARAMETERS_HPP

#define AUDIO_U8        0x0008  /**< Unsigned 8-bit samples */
#define AUDIO_S8        0x8008  /**< Signed 8-bit samples */
#define AUDIO_U16LSB    0x0010  /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB    0x8010  /**< Signed 16-bit samples */
#define AUDIO_U16MSB    0x1010  /**< As above, but big-endian byte order */
#define AUDIO_S16MSB    0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB
/* @} */

/**
 *  \name int32 support
 */
 /* @{ */
#define AUDIO_S32LSB    0x8020  /**< 32-bit integer samples */
#define AUDIO_S32MSB    0x9020  /**< As above, but big-endian byte order */
#define AUDIO_S32       AUDIO_S32LSB
/* @} */

/**
 *  \name float32 support
 */
 /* @{ */
#define AUDIO_F32LSB    0x8120  /**< 32-bit floating point samples */
#define AUDIO_F32MSB    0x9120  /**< As above, but big-endian byte order */
#define AUDIO_F32       AUDIO_F32LSB
/* @} */

/**
 *  \name Native audio byte ordering
 */
 /* @{ */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS    AUDIO_U16LSB
#define AUDIO_S16SYS    AUDIO_S16LSB
#define AUDIO_S32SYS    AUDIO_S32LSB
#define AUDIO_F32SYS    AUDIO_F32LSB
#else
#define AUDIO_U16SYS    AUDIO_U16MSB
#define AUDIO_S16SYS    AUDIO_S16MSB
#define AUDIO_S32SYS    AUDIO_S32MSB
#define AUDIO_F32SYS    AUDIO_F32MSB
#endif

#define ENUM_AUDIO_FMT(name) enum class name##_##AudioFormats
#define SET_FMT_VAL(sub_enum,name,value) sub_enum##_##name = value
#define GET_FMT_VAL(enum_name) enum_name##_##AudioFormats

#define XAUDIO_MAP(sub_enum,arr_name) \
static constexpr std::pair<ENUM_AUDIO_FMT(XAudio),ENUM_AUDIO_FMT(sub_enum)> arr_name[]

#include <type_traits>
#include <array>


ENUM_AUDIO_FMT(Qt) : int {
    Unknown = 0,
    SET_FMT_VAL(Qt,UInt8,1),
    SET_FMT_VAL(Qt,Int16,2),
    SET_FMT_VAL(Qt,Int32,3),
    SET_FMT_VAL(Qt,Float,4),
};

ENUM_AUDIO_FMT(SDL) : int {
    Unknown = -1,
    SET_FMT_VAL(SDL,AUDIO_U8,AUDIO_U8),
    SET_FMT_VAL(SDL,AUDIO_S8,AUDIO_S8),

    SET_FMT_VAL(SDL,AUDIO_U16LSB,AUDIO_U16LSB),
    SET_FMT_VAL(SDL,AUDIO_S16LSB,AUDIO_S16LSB),
    SET_FMT_VAL(SDL,AUDIO_U16,AUDIO_U16),

    SET_FMT_VAL(SDL,AUDIO_U16MSB,AUDIO_U16MSB),
    SET_FMT_VAL(SDL,AUDIO_S16MSB,AUDIO_S16MSB),
    SET_FMT_VAL(SDL,AUDIO_S16,AUDIO_S16),

    SET_FMT_VAL(SDL,AUDIO_S32LSB,AUDIO_S32LSB),
    SET_FMT_VAL(SDL,AUDIO_S32MSB,AUDIO_S32MSB),
    SET_FMT_VAL(SDL,AUDIO_S32,AUDIO_S32),

    SET_FMT_VAL(SDL,AUDIO_F32LSB,AUDIO_F32LSB),
    SET_FMT_VAL(SDL,AUDIO_F32MSB,AUDIO_F32MSB),
    SET_FMT_VAL(SDL,AUDIO_F32,AUDIO_F32),

    SET_FMT_VAL(SDL,AUDIO_U16SYS,AUDIO_U16SYS),
    SET_FMT_VAL(SDL,AUDIO_S16SYS,AUDIO_S16SYS),
    SET_FMT_VAL(SDL,AUDIO_S32SYS,AUDIO_S32SYS),
    SET_FMT_VAL(SDL,AUDIO_F32SYS,AUDIO_F32SYS),
};

ENUM_AUDIO_FMT(XAudio) : int {
    Unknown = -1,
    SET_FMT_VAL(XAudio,S8_FMT,1),
    SET_FMT_VAL(XAudio,U8_FMT,2),
    SET_FMT_VAL(XAudio,S16_FMT,3),
    SET_FMT_VAL(XAudio,U16_FMT,4),
    SET_FMT_VAL(XAudio,S32_FMT,5),
    SET_FMT_VAL(XAudio,U32_FMT,6),
    SET_FMT_VAL(XAudio,S64_FMT,7),
    SET_FMT_VAL(XAudio,U64_FMT,8),
    SET_FMT_VAL(XAudio,FLOAT_FMT,9),
    SET_FMT_VAL(XAudio,DOUBLE_FMT,10),
};

struct XAudioSpec {
  int m_freq {44100};
  ENUM_AUDIO_FMT(XAudio) m_format {GET_FMT_VAL(XAudio)::XAudio_S16_FMT};
  int m_channels{2},m_samples{1024};
};

static inline ENUM_AUDIO_FMT(Qt) xaudio_fmt_to_qt_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) {

    XAUDIO_MAP(Qt,list){
        {GET_FMT_VAL(XAudio)::XAudio_U8_FMT,GET_FMT_VAL(Qt)::Qt_UInt8},
        {GET_FMT_VAL(XAudio)::XAudio_S16_FMT,GET_FMT_VAL(Qt)::Qt_Int16},
        {GET_FMT_VAL(XAudio)::XAudio_S32_FMT,GET_FMT_VAL(Qt)::Qt_Int32},
        {GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT,GET_FMT_VAL(Qt)::Qt_Float},
    };

    for (const auto &[first, second] : list) {
        if (first == f) {
            return second;
        }
    }

    return GET_FMT_VAL(Qt)::Unknown;
}

static inline ENUM_AUDIO_FMT(SDL) xaudio_fmt_to_sdl_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) {

    XAUDIO_MAP(SDL,list) {
        {GET_FMT_VAL(XAudio)::XAudio_S8_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_S8},
        {GET_FMT_VAL(XAudio)::XAudio_U8_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_U8},
        {GET_FMT_VAL(XAudio)::XAudio_S16_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_S16SYS},
        {GET_FMT_VAL(XAudio)::XAudio_U16_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_U16SYS},
        {GET_FMT_VAL(XAudio)::XAudio_S32_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_S32SYS},
        {GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_F32SYS},
    };

    for (const auto &[first,second] : list) {
        if (first == f) {
            return second;
        }
    }

    return GET_FMT_VAL(SDL)::Unknown;
}

#undef ENUM_AUDIO_FMT
#undef SET_FMT_VAL
#undef GET_FMT_VAL
#undef XAUDIO_MAP

#endif
