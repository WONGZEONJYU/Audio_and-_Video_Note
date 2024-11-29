#include "xaudio_play.hpp"
#include "xavframe.hpp"
#include "xswrsample.hpp"

using namespace std;

template<typename T>
static inline bool plane_to_interleaved(const XAVFrame &frame,vector<uint8_t> &out) {

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
    if (const auto &[num, den]{parameters.x_time_base()}; num > 0) {
        m_time_base_ = static_cast<double>(den) / static_cast<double>(num);
    }
    return {};
}

auto XAudio_Play::Open(const XCodecParameters_sp &parameters) -> bool {
    if (parameters) {
        XAudio_Play::Open(*parameters);
    }
    return {};
}

void XAudio_Play::push_helper(data_buffer_t &in,const int64_t &pts) {
    unique_lock locker(m_mux_);
    if (data_buffer_t out; Speed_Change(in,out) > 0) {
        TRY_CATCH(CHECK_EXC(m_datum_.emplace_back(std::move(out),0,pts)));
    }
}

void XAudio_Play::Push(const uint8_t *data, const size_t &size,const int64_t &pts) {
    vector in_buf(data, data + size);
    push_helper(in_buf,pts);
}

void XAudio_Play::Push(const XAVFrame &frame) {

    IS_NULLPTR(frame.data[0], return);
    if (frame.width || frame.height) {
        return;
    }

    if (1 == frame.ch_layout.nb_channels) {
        const auto fmt_size{av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame.format))};
        Push(frame.data[0],fmt_size * frame.nb_samples,frame.pts);
        return;
    }

    using plane_to_cross_type = bool(*)(const XAVFrame &frame,vector<uint8_t> &out);
    static constexpr pair<ENUM_AUDIO_FMT(FF),plane_to_cross_type> list[]{
        {GET_FMT_VAL(FF)::FF_FMT_U8P,plane_to_interleaved<uint8_t>},
        {GET_FMT_VAL(FF)::FF_FMT_S16P,plane_to_interleaved<uint16_t>},
        {GET_FMT_VAL(FF)::FF_FMT_S32P,plane_to_interleaved<uint32_t>},
        {GET_FMT_VAL(FF)::FF_FMT_FLTP,plane_to_interleaved<float>},
        {GET_FMT_VAL(FF)::FF_FMT_DBLP,plane_to_interleaved<double>},
        {GET_FMT_VAL(FF)::FF_FMT_S64P,plane_to_interleaved<uint64_t>},
    };

    data_buffer_t in;
    bool b{};
    for (const auto &[fst,snd]:list) {
        if (fst == frame.format) {
            b = snd(frame,in);
            break;
        }
    }

    if (b) {
        push_helper(in,frame.pts);
    }else {
        Push(frame.data[0],frame.linesize[0],frame.pts);
    }
}

void XAudio_Play::AudioCallback(void * const userdata,
                                uint8_t * stream,const int length) {
    const auto this_{static_cast<XAudio_Play *>(userdata)};
    this_->Callback(stream,length);
}

bool XAudio_Play::init_swr(const XSwrParam &p) {
    m_swr_ = new_XSwrSample(p);
    return m_swr_.operator bool();
}

bool XAudio_Play::init_speed_ctr(const int &sample_rate,const int &channels){
    CHECK_FALSE_(m_init_speed_ctr_ = m_speed_ctr_.Open(sample_rate,channels),return {});
    return m_init_speed_ctr_;
}

template<typename T>
static inline int64_t Speed_Change_helper(const vector<uint8_t> &in, vector<uint8_t> &out,
    Audio_Playback_Speed &s) {

    if constexpr (!(std::is_same_v<T,uint8_t> || std::is_same_v<T,int8_t> ||
        std::is_same_v<T,short> || std::is_same_v<T,uint16_t> ||
        std::is_same_v<T,int32_t> || std::is_same_v<T,uint32_t> ||
        std::is_same_v<T,int64_t> || std::is_same_v<T,uint64_t> ||
        std::is_same_v<T,float> || std::is_same_v<T,double> )) {
        static_assert(false,GET_STR(not support type));
    }

    int64_t out_size{-1};

    const auto channels{s.get_channels()};

    const auto in_samples_num{static_cast<int>(in.size() / (channels * sizeof(T)))};

    CHECK_FALSE_(s.Send(reinterpret_cast<const T*>(in.data()),in_samples_num),return out_size);

    auto need_sample{s.sonicSamplesAvailable()};
    need_sample = need_sample < 0 ? 0 : need_sample;
    //cerr << "need_sample = " << need_sample << "\n";
    out.resize(need_sample * channels * sizeof(T));

    out_size = s.Receive(reinterpret_cast<T*>(out.data()),need_sample);

    if (out_size > 0) {
        out_size = static_cast<decltype(out_size)>(out.size());
    }

    return out_size;
}

int64_t XAudio_Play::Speed_Change(data_buffer_t &in,data_buffer_t &out) {

    int64_t out_size{-1};

    CHECK_FALSE_(!in.empty(),return out_size);

    if (1.0f != m_speed_) {

        using Speed_Change_type = int64_t(*)(const vector<uint8_t> &,vector<uint8_t> &,
                Audio_Playback_Speed &);

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
                out_size = second(in,out,m_speed_ctr_);
                break;
            }
        }
    }else {
        out = std::move(in);
        out_size = static_cast<decltype(out_size)>(out.size());
    }

    return out_size;
}

void XAudio_Play::set_speed(const double &s) {
    if(m_init_speed_ctr_){
        m_speed_ = s < 0.1 ? 0.1f : static_cast<float>(s);
        m_speed_ctr_.Set_Speed(m_speed_);
    }
}
