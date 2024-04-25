//
// Created by Administrator on 2024/4/20.
//

#ifndef INC_15_MP4_MUXER_AUDIOOUTPUTSTREAM_HPP
#define INC_15_MP4_MUXER_AUDIOOUTPUTSTREAM_HPP

#include "OutputStreamAbstract.h"
#include "Audio_Resample.h"
#include "AudioEncoder.h"

struct Audio_encoder_params;
class Muxer;

class AudioOutputStream final : public OutputStreamAbstract{

    explicit AudioOutputStream() = default;
    void Construct(const std::shared_ptr<Muxer>&,
                   const Audio_encoder_params&,
                   const Audio_Resample_Params &) noexcept(false);

public:
    using AudioOutputStream_sp_type = std::shared_ptr<AudioOutputStream>;

    static AudioOutputStream_sp_type create(const std::shared_ptr<Muxer> &,
                                            const Audio_encoder_params &,
                                            const Audio_Resample_Params &) noexcept(false);

    [[nodiscard]] auto Frame_size() const noexcept(true){
        return m_stream->codecpar->frame_size;
    }

//    [[nodiscard]] auto nb_Frames() const noexcept(true){
//        return m_stream->nb_frames;
//    }

    void encoder(const ShareAVFrame_sp_type &,const long long &pts,
                 const AVRational& ,vector_type& ) const noexcept(false);

private:
    AudioEncoder_sp_type m_encoder;
    Audio_Resample_type m_audioResample;
};

using AudioOutputStream_sp_type = typename AudioOutputStream::AudioOutputStream_sp_type;

AudioOutputStream_sp_type new_AudioOutputStream(const std::shared_ptr<Muxer>&,
                                                const Audio_encoder_params&,
                                                const Audio_Resample_Params &) noexcept(false);
#endif
