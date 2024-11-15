#include "xaudio_play.hpp"
#include "xavframe.hpp"
#include "xswrsample.hpp"

using namespace std;

template<typename T>
static inline bool plane_to_cross(const XAVFrame &frame,vector<uint8_t> &out) {

    if constexpr (!(std::is_same_v<T,uint8_t> ||
        std::is_same_v<T,short> || std::is_same_v<T,uint16_t> ||
        std::is_same_v<T,int32_t> || std::is_same_v<T,uint32_t> ||
        std::is_same_v<T,int64_t> || std::is_same_v<T,uint64_t> ||
        std::is_same_v<T,float> || std::is_same_v<T,double> )) {
        static_assert(false,GET_STR(not support type));
    }

    IS_NULLPTR(frame.data[0],return {});
    IS_NULLPTR(frame.data[1],return {});
    CHECK_FALSE_(frame.linesize[0] > 0,return {});

    if (out.capacity() < frame.linesize[0]) {
        out.clear();
        TRY_CATCH(CHECK_EXC(out.resize(frame.linesize[0])),return {});
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

void XAudio_Play::push_helper(data_buffer_t &in) {
    std::unique_lock locker(m_mux_);
    if (data_buffer_t out; Speed_Change(in,out) > 0) {
        TRY_CATCH(CHECK_EXC(m_datum_.emplace_back(std::move(out),0)));
    }
}

void XAudio_Play::Push(const uint8_t *data, const size_t &size) {
    std::vector in_buf(data, data + size);
    push_helper(in_buf);
}

void XAudio_Play::Push(const XAVFrame &frame) {

    IS_NULLPTR(frame.data[0], return);

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

bool XAudio_Play::init_swr(const XSwrParam &p) {
    m_swr_ = new_XSwrSample(p);
    return m_swr_.operator bool();
}

int64_t XAudio_Play::Speed_Change(data_buffer_t &in, data_buffer_t &out) {

    int64_t out_size{-1};

    CHECK_FALSE_(!in.empty(),return out_size);

    if (1.0f != m_speed_) {

        m_speed_ctr_.Set_Speed(m_speed_);
        const auto in_samples_num{static_cast<int>(in.size() / (m_spec_.m_channels  * sizeof(int16_t)))};

        CHECK_FALSE_(m_speed_ctr_.Send(reinterpret_cast<int16_t*>(in.data()),in_samples_num),return out_size);

        auto need_sample{m_speed_ctr_.sonicSamplesAvailable()};
        need_sample = need_sample < 0 ? 0 : need_sample;
        data_buffer_t temp_out(need_sample * m_spec_.m_channels * sizeof(int16_t));

        out_size = m_speed_ctr_.Receive(reinterpret_cast<int16_t *>(temp_out.data()),need_sample);

        if (out_size > 0) {
            out_size = static_cast<decltype(out_size)>(temp_out.size());
            out = std::move(temp_out);
        }
    }else {
        out = std::move(in);
        out_size = static_cast<decltype(out_size)>(out.size());
    }

    return out_size;
}
