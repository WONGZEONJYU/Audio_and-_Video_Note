#ifndef INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H
#define INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H

#include "OutputStreamAbstract.h"

#include <memory>

class AudioOutputStream final : public OutputStreamAbstract{

    bool construct() noexcept;
    explicit AudioOutputStream();

    void init_codec_parms();
    bool add_stream();
    bool open();
    bool swr_init() noexcept;

public:
    static std::shared_ptr<OutputStreamAbstract> create();
    ~AudioOutputStream();
    bool write_frame() override;

private:

};


#endif //INC_14_MUXING_FLV_AUDIOOUTPUTSTREAM_H
