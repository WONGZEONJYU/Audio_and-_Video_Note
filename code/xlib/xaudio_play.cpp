#include "xaudio_play.hpp"
#include "xavframe.hpp"
#include "xswrsample.hpp"

using namespace std;

template<typename T>
static inline bool plane_to_cross(const XAVFrame &frame,vector<uint8_t> &out) {

    if constexpr (!(std::is_same_v<T,uint8_t> || std::is_same_v<T,int8_t> ||
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

auto XAudio_Play::Open(const XCodecParameters &parameters)->bool {
    m_ff_audio_parameters_ = parameters;
    return {};
}

auto XAudio_Play::Open(const XCodecParameters_sp &parameters) ->bool {
    if (parameters) {
        m_ff_audio_parameters_ = *parameters;
    }
    return {};
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

#if 1
    using plane_to_cross_type = bool(*)(const XAVFrame &frame,vector<uint8_t> &out);

    static constexpr pair<ENUM_AUDIO_FMT(FF),plane_to_cross_type> list[]{
        {GET_FMT_VAL(FF)::FF_FMT_U8P,plane_to_cross<uint8_t>},
        {GET_FMT_VAL(FF)::FF_FMT_S16P,plane_to_cross<uint16_t>},
        {GET_FMT_VAL(FF)::FF_FMT_S32P,plane_to_cross<uint32_t>},
        {GET_FMT_VAL(FF)::FF_FMT_FLTP,plane_to_cross<float>},
        {GET_FMT_VAL(FF)::FF_FMT_DBLP,plane_to_cross<double>},
        {GET_FMT_VAL(FF)::FF_FMT_S64P,plane_to_cross<uint64_t>},
    };

    for (const auto &[fst,snd]:list) {
        if (fst == frame.format) {
            b = snd(frame,in);
            break;
        }
    }

    if (b) {
        push_helper(in);
    }else {
        Push(frame.data[0],frame.linesize[0]);
    }

#else
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
#endif
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

bool XAudio_Play::init_speed_ctr(const int &sample_rate,const int &channels){
    CHECK_FALSE_(m_init_speed_ctr_ = m_speed_ctr_.Open(sample_rate,channels),return {});
    return true;
}

template<typename T>
static inline int64_t Speed_Change_helper(const vector<uint8_t> &in, vector<uint8_t> &out,
    Audio_Playback_Speed &s,const XAudioSpec &spec_) {

    if constexpr (!(std::is_same_v<T,uint8_t> || std::is_same_v<T,int8_t> ||
        std::is_same_v<T,short> || std::is_same_v<T,uint16_t> ||
        std::is_same_v<T,int32_t> || std::is_same_v<T,uint32_t> ||
        std::is_same_v<T,int64_t> || std::is_same_v<T,uint64_t> ||
        std::is_same_v<T,float> || std::is_same_v<T,double> )) {
        static_assert(false,GET_STR(not support type));
    }

    int64_t out_size{-1};

    const auto in_samples_num{static_cast<int>(in.size() / (spec_.m_channels  * sizeof(T)))};

    CHECK_FALSE_(s.Send(reinterpret_cast<const T*>(in.data()),in_samples_num),return out_size);

    auto need_sample{s.sonicSamplesAvailable()};
    need_sample = need_sample < 0 ? 0 : need_sample;
    vector<uint8_t> temp_out(need_sample * spec_.m_channels * sizeof(T));

    out_size = s.Receive(reinterpret_cast<T*>(temp_out.data()),need_sample);

    if (out_size > 0) {
        out_size = static_cast<decltype(out_size)>(temp_out.size());
        out = std::move(temp_out);
    }

    return out_size;
}

int64_t XAudio_Play::Speed_Change(data_buffer_t &in, data_buffer_t &out) {

    int64_t out_size{-1};

    CHECK_FALSE_(!in.empty(),return out_size);

    if (1.0f != m_speed_) {
#if 0
        m_speed_ctr_.Set_Speed(m_speed_);
        const auto in_samples_num{static_cast<int>(in.size() / (m_spec_.m_channels  * sizeof(float)))};

        CHECK_FALSE_(m_speed_ctr_.Send(reinterpret_cast<float*>(in.data()),in_samples_num),return out_size);

        auto need_sample{m_speed_ctr_.sonicSamplesAvailable()};
        need_sample = need_sample < 0 ? 0 : need_sample;
        data_buffer_t temp_out(need_sample * m_spec_.m_channels * sizeof(float));

        out_size = m_speed_ctr_.Receive(reinterpret_cast<float *>(temp_out.data()),need_sample);

        if (out_size > 0) {
            out_size = static_cast<decltype(out_size)>(temp_out.size());
            out = std::move(temp_out);
        }
#endif
        using Speed_Change_type = int64_t(*)(const vector<uint8_t> &, vector<uint8_t> &,
                Audio_Playback_Speed &,const XAudioSpec &);

        static constexpr pair<ENUM_AUDIO_FMT(XAudio),Speed_Change_type> list[]{
            {GET_FMT_VAL(XAudio)::XAudio_S8_FMT,Speed_Change_helper<int8_t>},
            {GET_FMT_VAL(XAudio)::XAudio_U8_FMT,Speed_Change_helper<uint8_t>},
            {GET_FMT_VAL(XAudio)::XAudio_S16_FMT,Speed_Change_helper<int16_t>},
            {GET_FMT_VAL(XAudio)::XAudio_U16_FMT,Speed_Change_helper<uint16_t>},
            {GET_FMT_VAL(XAudio)::XAudio_S32_FMT,Speed_Change_helper<int32_t>},
            {GET_FMT_VAL(XAudio)::XAudio_U32_FMT,Speed_Change_helper<uint32_t>},
            {GET_FMT_VAL(XAudio)::XAudio_S64_FMT,Speed_Change_helper<int64_t>},
            {GET_FMT_VAL(XAudio)::XAudio_U64_FMT,Speed_Change_helper<uint64_t>},
            {GET_FMT_VAL(XAudio)::XAudio_FLOAT_FMT,Speed_Change_helper<float>},
            {GET_FMT_VAL(XAudio)::XAudio_DOUBLE_FMT,Speed_Change_helper<double>},
        };

        for (const auto &[first,
                          second] : list) {
            if (first == m_spec_.m_format){
                out_size = second(in,out,m_speed_ctr_,m_spec_);
                break;
            }
        }
    }else {
        out = std::move(in);
        out_size = static_cast<decltype(out_size)>(out.size());
    }

    return out_size;
}
