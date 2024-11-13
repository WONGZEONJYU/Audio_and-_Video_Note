#include "xaudio_play.hpp"
#include "xavframe.hpp"
#include "xcodec_parameters.hpp"

using namespace std;

template<typename T>
static inline bool plane_to_cross(const XAVFrame &frame,vector<uint8_t> &out) {

    if (!frame.data[0] || !frame.data[1] || frame.linesize[0] != frame.linesize[1]) {
        return {};
    }

    if (out.capacity() < frame.linesize[0]) {
        out.clear();
        out.resize(frame.linesize[0]);
    }

    const auto out_data{reinterpret_cast<T*>(out.data())};

    const auto Left_{reinterpret_cast<T*>(frame.data[0])},
            Right_{reinterpret_cast<T*>(frame.data[1])};

    for (decltype(frame.nb_samples) i{};i < frame.nb_samples;++i) {
        const auto index{i * 2};
        out_data[index] = Left_[i];
        out_data[index + 1] = Right_[i];
    }
    return true;
}

bool XAudio_Play::Open(const XCodecParameters &parameters) {

    XAudioSpec spec;
    auto &[freq,format,channels,samples]{spec};
    freq = parameters.Sample_rate();
    channels = parameters.Ch_layout()->nb_channels;

    const auto frame_size{ parameters.Audio_nbSamples()};
    samples = frame_size > 0 ? frame_size : samples;

    switch (parameters.Audio_sample_format()) {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P: {
            format = AUDIO_S8;
            break;
        }
        case AV_SAMPLE_FMT_S16:         ///< signed 16 bits
        case AV_SAMPLE_FMT_S16P: {      ///< signed 16 bits, planar
            format = AUDIO_S16;
            break;
        }
        case AV_SAMPLE_FMT_S32:         ///< signed 32 bits
        case AV_SAMPLE_FMT_S32P: {      ///< signed 32 bits, planar
            format = AUDIO_S32;
            break;
        }
        case AV_SAMPLE_FMT_FLT:         ///< float
            case AV_SAMPLE_FMT_FLTP: {      ///< float, planar
            format = AUDIO_F32;
            break;
        }
        default:
            break;
    }

    return Open(spec);
}

bool XAudio_Play::Open(const XAudioSpec &spec_) {
    std::unique_lock locker(m_mux_);
    is_init_son_ = m_son.Open(spec_.m_freq,spec_.m_channels);
    if (is_init_son_) {
        m_spec_ = spec_;
    }
    return is_init_son_;
}

void XAudio_Play::push_helper(data_buffer_t &in) {
    std::unique_lock locker(m_mux_);
    if (data_buffer_t out; Speed_Change(in,out) > 0) {
        auto &[m_data,m_offset] {m_datum_.emplace_back()};
        m_data = std::move(out);
    }
}

void XAudio_Play::Push(const uint8_t *data, const size_t &size) {
    std::vector in_buf(data, data + size);
    push_helper(in_buf);
}

void XAudio_Play::Push(const XAVFrame &frame) {

    if (!frame.data[0]) {
        return;
    }

    data_buffer_t in;
    bool b{};
    switch (frame.format) {
        case AV_SAMPLE_FMT_U8P: {
            b = plane_to_cross<uint8_t>(frame,in);
            break;
        }

        case AV_SAMPLE_FMT_S16P: {
            b = plane_to_cross<uint16_t>(frame,in);
            break;
        }

        case AV_SAMPLE_FMT_S32P: {
            b = plane_to_cross<uint32_t>(frame,in);
            break;
        }

        case AV_SAMPLE_FMT_FLTP: {
            b = plane_to_cross<float>(frame,in);
            break;
        }

        case AV_SAMPLE_FMT_DBLP: {
            b = plane_to_cross<double>(frame,in);
            break;
        }
        case AV_SAMPLE_FMT_S64P: {
            b = plane_to_cross<uint64_t>(frame,in);
            break;
        }

        default: {
            Push(frame.data[0],frame.linesize[0]);
            return;
        }
    }

    if (b) {
        push_helper(in);
    }
}

void XAudio_Play::AudioCallback(void * const userdata,
                                uint8_t * stream,const int length) {
    const auto this_{static_cast<XAudio_Play *>(userdata)};
    this_->Callback(stream, length);
}

int64_t XAudio_Play::Speed_Change(data_buffer_t &in, data_buffer_t &out) {

    int64_t out_size{-1};

    if (in.empty()) {
        return out_size;
    }

    if (is_init_son_ && 1.0f != m_speed_) {

        m_son.sonicSetSpeed(m_speed_);

        const auto in_samples_num{static_cast<int>(in.size() / (static_cast<int>(m_spec_.m_channels)  * sizeof(int16_t)) )};

        CHECK_FALSE_(m_son.sonicWriteShortToStream(reinterpret_cast<int16_t*>(in.data()),in_samples_num),return out_size);

        auto need_sample{m_son.sonicSamplesAvailable()};
        need_sample = need_sample < 0 ? 0 : need_sample;
        data_buffer_t temp_out(need_sample * m_spec_.m_channels * sizeof(int16_t));

        out_size  = m_son.sonicReadShortFromStream(reinterpret_cast<int16_t *>(temp_out.data()),need_sample);

        if (out_size > 0) {
            out = std::move(temp_out);
        }

    }else {
        out = std::move(in);
        out_size = static_cast<int64_t>(out.size());
    }

    return out_size;
}
