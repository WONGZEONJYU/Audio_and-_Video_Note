#ifndef INC_14_MUXING_FLV_OUTPUTSTREAMABSTRACT_H
#define INC_14_MUXING_FLV_OUTPUTSTREAMABSTRACT_H

class OutputStreamAbstract {
public:
    virtual void write_frame() = 0;
};

#endif
