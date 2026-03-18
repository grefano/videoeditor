#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include "profile.hpp"
extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/samplefmt.h>
    #include <libswscale/swscale.h>
}
#include "mediapool.h"
struct VideoReaderState{
    AVRational time_base;
    uint8_t* frame_buffer;
    AVFormatContext* av_format_context;
    AVCodecContext* av_codec_context;
    AVFrame* av_frame;
    AVPacket* av_packet;
    SwsContext* sws_scaler_context;
    int video_stream_index;

};
class VideoReader{
    private:
    
    public:
    int w, h;
    int64_t pts = 0;
    VideoReaderState state;
    bool file_open(const char* filename);
    bool read_frame();
    bool seek_frame(double ts);
    bool jump_to_ts(float ts_sec, double* pt_seconds, int64_t* pts);
    double get_time_base();
    double get_timestamp();

    VideoReader(const char* filepath);
    ~VideoReader();

};

