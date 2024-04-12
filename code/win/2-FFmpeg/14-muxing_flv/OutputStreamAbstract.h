#ifndef INC_14_MUXING_FLV_OUTPUTSTREAMABSTRACT_H
#define INC_14_MUXING_FLV_OUTPUTSTREAMABSTRACT_H

struct AVFormatContext;
struct AVRational;
struct AVStream;
struct AVPacket;

class OutputStreamAbstract {
public:
    [[nodiscard]] virtual bool write_frame() = 0;
protected:
   static int write_media_file(AVFormatContext&,const AVRational&,AVStream&,AVPacket&);
};

#endif
