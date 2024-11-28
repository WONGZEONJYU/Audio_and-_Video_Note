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

#define ENUM_AUDIO_FMT(name) enum name##_##AudioFormats
#define SET_FMT_VAL(sub_enum,name,value) sub_enum##_##name = value
#define GET_FMT_VAL(enum_name) enum_name##_##AudioFormats

#include "xhelper.hpp"

ENUM_AUDIO_FMT(XAudio) {
    SET_FMT_VAL(XAudio,Unknown,-1),
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
    SET_FMT_VAL(XAudio,U8P_FMT,11),
    SET_FMT_VAL(XAudio,S16P_FMT,12),
    SET_FMT_VAL(XAudio,S32P_FMT,13),
    SET_FMT_VAL(XAudio,FLOATP_FMT,14),
    SET_FMT_VAL(XAudio,DOUBLEP_FMT,15),
    SET_FMT_VAL(XAudio,S64P_FMT,16),
};

ENUM_AUDIO_FMT(Qt)  {
    SET_FMT_VAL(Qt,Unknown,0),
    SET_FMT_VAL(Qt,UInt8,1),
    SET_FMT_VAL(Qt,Int16,2),
    SET_FMT_VAL(Qt,Int32,3),
    SET_FMT_VAL(Qt,Float,4),
};

ENUM_AUDIO_FMT(SDL) {
    SET_FMT_VAL(SDL,Unknown,-1),
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

ENUM_AUDIO_FMT(FF) {
    SET_FMT_VAL(FF,Unknown,-1),
    SET_FMT_VAL(FF,FMT_U8,0),
    SET_FMT_VAL(FF,FMT_S16,1),
    SET_FMT_VAL(FF,FMT_S32,2),
    SET_FMT_VAL(FF,FMT_FLT,3),
    SET_FMT_VAL(FF,FMT_DBL,4),
    SET_FMT_VAL(FF,FMT_U8P,5),
    SET_FMT_VAL(FF,FMT_S16P,6),
    SET_FMT_VAL(FF,FMT_S32P,7),
    SET_FMT_VAL(FF,FMT_FLTP,8),
    SET_FMT_VAL(FF,FMT_DBLP,9),
    SET_FMT_VAL(FF,FMT_S64,10),
    SET_FMT_VAL(FF,FMT_S64P,11),
    SET_FMT_VAL(FF,FMT_NB,12),
};

struct XAudioSpec {
  int m_freq {44100};
  ENUM_AUDIO_FMT(XAudio) m_format {GET_FMT_VAL(XAudio)::XAudio_S16_FMT};
  int m_channels{2},m_samples{1024},format_size{2};
};

/**
 * 把自定义格式转换为QT支持的播放格式
 * @param f 自定义格式
 * @return GET_FMT_VAL(Qt)
 */
XLIB_API auto to_qt_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) ->ENUM_AUDIO_FMT(Qt) ;

/**
 * 把自定义格式转换为SDL支持的播放格式
 * @param f 自定义格式
 * @return GET_FMT_VAL(SDL)
 */
XLIB_API auto to_sdl_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) ->ENUM_AUDIO_FMT(SDL);

/**
 * 把SDL格式转换成自定义格式
 * @param f SDL格式
 * @return
 */
XLIB_API auto sdl_to_xaudio_format(const ENUM_AUDIO_FMT(SDL) &f) ->ENUM_AUDIO_FMT(XAudio);

/**
 *sdl_to_xaudio_format的重载版
 */
static inline auto sdl_to_xaudio_format(const int &f){
    return sdl_to_xaudio_format(static_cast<ENUM_AUDIO_FMT(SDL)>(f));
}

/**
 * 把自定义格式转换为ffmpeg支持的格式
 * @param f 自定义格式
 * @return GET_FMT_VAL(FF)
 */
XLIB_API auto to_ffmpeg_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) ->ENUM_AUDIO_FMT(FF);

/**
 * 把ffmpeg支持的格式转换为自定义格式
 * 平面格式一律转换为交错模式
 * @param f
 * @return GET_FMT_VAL(XAudio)
 */
XLIB_API auto ff_to_xaduio_format(const ENUM_AUDIO_FMT(FF) &f) ->ENUM_AUDIO_FMT(XAudio);

/**
 * ff_to_xaduio_format的重载版
 */
static inline auto ff_to_xaduio_format(const int &f) {
    return ff_to_xaduio_format(static_cast<ENUM_AUDIO_FMT(FF)>(f));
}

/**
 * ffmpeg平面格式转换为交错模式
 * @param f
 * @return  GET_FMT_VAL(FF)
 */
XLIB_API auto planer_to_interlace(const ENUM_AUDIO_FMT(FF) &f) ->ENUM_AUDIO_FMT(FF);

static inline auto planer_to_interlace(const int &f) {
    return planer_to_interlace(static_cast<ENUM_AUDIO_FMT(FF)>(f));
}

#endif
