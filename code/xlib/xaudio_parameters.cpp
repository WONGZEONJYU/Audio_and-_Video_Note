#include "xaduio_parameters.hpp"

#define XAUDIO_MAP(sub_enum,arr_name) \
static constexpr std::pair<ENUM_AUDIO_FMT(XAudio),ENUM_AUDIO_FMT(sub_enum)> arr_name[]

auto to_qt_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) ->ENUM_AUDIO_FMT(Qt) {

    XAUDIO_MAP(Qt,list){
        {GET_FMT_VAL(XAudio)::XAudio_U8_FMT,GET_FMT_VAL(Qt)::Qt_UInt8},
        {GET_FMT_VAL(XAudio)::XAudio_S16_FMT,GET_FMT_VAL(Qt)::Qt_Int16},
        {GET_FMT_VAL(XAudio)::XAudio_S32_FMT,GET_FMT_VAL(Qt)::Qt_Int32},
        {GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT,GET_FMT_VAL(Qt)::Qt_Float},
    };

    for (const auto &[first,second] : list) {
        if (first == f) {
            return second;
        }
    }

    return GET_FMT_VAL(Qt)::Qt_Unknown;
}

auto to_sdl_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) ->ENUM_AUDIO_FMT(SDL) {
    XAUDIO_MAP(SDL,list) {
        {GET_FMT_VAL(XAudio)::XAudio_S8_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_S8},
        {GET_FMT_VAL(XAudio)::XAudio_U8_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_U8},
        {GET_FMT_VAL(XAudio)::XAudio_S16_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_S16SYS},
        {GET_FMT_VAL(XAudio)::XAudio_U16_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_U16SYS},
        {GET_FMT_VAL(XAudio)::XAudio_S32_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_S32SYS},
        {GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT,GET_FMT_VAL(SDL)::SDL_AUDIO_F32SYS},
    };

    for (const auto &[first,
            second] : list) {
        if (first == f) {
            return second;
        }
    }

    return GET_FMT_VAL(SDL)::SDL_Unknown;
}

auto sdl_to_xaudio_format(const ENUM_AUDIO_FMT(SDL) &f) ->ENUM_AUDIO_FMT(XAudio) {

    static constexpr std::pair<ENUM_AUDIO_FMT(SDL),ENUM_AUDIO_FMT(XAudio)> list[] {
        {GET_FMT_VAL(SDL)::SDL_AUDIO_S8,GET_FMT_VAL(XAudio)::XAudio_S8_FMT},
        {GET_FMT_VAL(SDL)::SDL_AUDIO_U8,GET_FMT_VAL(XAudio)::XAudio_U8_FMT},
        {GET_FMT_VAL(SDL)::SDL_AUDIO_S16SYS,GET_FMT_VAL(XAudio)::XAudio_S16_FMT},
        {GET_FMT_VAL(SDL)::SDL_AUDIO_U16SYS,GET_FMT_VAL(XAudio)::XAudio_U16_FMT},
        {GET_FMT_VAL(SDL)::SDL_AUDIO_S32SYS,GET_FMT_VAL(XAudio)::XAudio_S32_FMT},
        {GET_FMT_VAL(SDL)::SDL_AUDIO_F32SYS,GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT},
    };

    for (const auto &[first,
            second] : list) {
        if (first == f) {
            return second;
        }
    }

    return GET_FMT_VAL(XAudio)::XAudio_Unknown;
}

auto to_ffmpeg_audio_format(const ENUM_AUDIO_FMT(XAudio) &f) ->ENUM_AUDIO_FMT(FF) {

    XAUDIO_MAP(FF,list) {
        {GET_FMT_VAL(XAudio)::XAudio_U8_FMT,GET_FMT_VAL(FF)::FF_FMT_U8},
        {GET_FMT_VAL(XAudio)::XAudio_S16_FMT,GET_FMT_VAL(FF)::FF_FMT_S16},
        {GET_FMT_VAL(XAudio)::XAudio_S32_FMT,GET_FMT_VAL(FF)::FF_FMT_S32},
        {GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT,GET_FMT_VAL(FF)::FF_FMT_FLT},
        {GET_FMT_VAL(XAudio)::XAudio_DOUBLE_FMT,GET_FMT_VAL(FF)::FF_FMT_DBL},
        {GET_FMT_VAL(XAudio)::XAudio_S64_FMT,GET_FMT_VAL(FF)::FF_FMT_S64},
    };

    for (const auto &[first,
            second] : list) {
        if (first == f) {
            return second;
        }
    }

    return GET_FMT_VAL(FF)::FF_Unknown;
}

auto ff_to_xaduio_format(const ENUM_AUDIO_FMT(FF) &f) -> ENUM_AUDIO_FMT(XAudio) {

    static constexpr std::pair<ENUM_AUDIO_FMT(FF),ENUM_AUDIO_FMT(XAudio)> list[] {
        {GET_FMT_VAL(FF)::FF_FMT_U8,GET_FMT_VAL(XAudio)::XAudio_U8_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_S16,GET_FMT_VAL(XAudio)::XAudio_S16_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_S32,GET_FMT_VAL(XAudio)::XAudio_S32_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_FLT,GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_DBL,GET_FMT_VAL(XAudio)::XAudio_DOUBLE_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_S64,GET_FMT_VAL(XAudio)::XAudio_S64_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_U8P,GET_FMT_VAL(XAudio)::XAudio_U8_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_S16P,GET_FMT_VAL(XAudio)::XAudio_S16_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_S32P,GET_FMT_VAL(XAudio)::XAudio_S32_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_S64P,GET_FMT_VAL(XAudio)::XAudio_S64_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_FLTP,GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT},
        {GET_FMT_VAL(FF)::FF_FMT_DBLP,GET_FMT_VAL(XAudio)::XAudio_DOUBLE_FMT},
    };

    for (const auto &[first,
            second] : list) {
        if (first == f) {
            return second;
        }
    }
    return GET_FMT_VAL(XAudio)::XAudio_Unknown;
}

auto planer_to_interlace(const ENUM_AUDIO_FMT(FF) &f) ->ENUM_AUDIO_FMT(FF) {

    static constexpr std::pair<ENUM_AUDIO_FMT(FF),ENUM_AUDIO_FMT(FF)> list[] {
        {GET_FMT_VAL(FF)::FF_FMT_U8P,GET_FMT_VAL(FF)::FF_FMT_U8},
        {GET_FMT_VAL(FF)::FF_FMT_S16P,GET_FMT_VAL(FF)::FF_FMT_S16},
        {GET_FMT_VAL(FF)::FF_FMT_S32P,GET_FMT_VAL(FF)::FF_FMT_S32},
        {GET_FMT_VAL(FF)::FF_FMT_S64P,GET_FMT_VAL(FF)::FF_FMT_S64},
        {GET_FMT_VAL(FF)::FF_FMT_FLTP,GET_FMT_VAL(FF)::FF_FMT_FLT},
        {GET_FMT_VAL(FF)::FF_FMT_DBLP,GET_FMT_VAL(FF)::FF_FMT_DBL},
    };

    for (const auto &[first,
            second] : list) {
        if (first == f) {
            return second;
        }
    }
    return GET_FMT_VAL(FF)::FF_Unknown;
}
