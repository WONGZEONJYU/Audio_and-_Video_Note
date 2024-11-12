#include "xaudio_play.hpp"

void XAudio_Play::Push(const uint8_t *data, const size_t &size) {
    std::vector in_buf(data, data + size);
    std::unique_lock locker(m_mux_);

    if (decltype(in_buf) out_buf; Speed_Change(in_buf,out_buf) > 0) {
        auto &[m_data,m_offset] {m_datum_.emplace_back()};
        m_data = std::move(out_buf);
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

    if (m_speed_ != 1.0f ) {

        m_son.sonicSetSpeed(m_speed_);

        const auto in_samples_num{static_cast<int>(in.size() / (static_cast<int>(m_spec_.m_channels)  * sizeof(int16_t)) )};

        CHECK_FALSE_(m_son.sonicWriteShortToStream(reinterpret_cast<int16_t*>(in.data()),in_samples_num),return out_size);

        const auto need_sample{m_son.sonicSamplesAvailable()};

        std::vector<uint8_t> temp_out(need_sample * m_spec_.m_channels * sizeof(int16_t));

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
