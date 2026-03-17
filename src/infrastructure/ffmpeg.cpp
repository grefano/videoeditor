#include "ffmpeg.h"

VideoReader::VideoReader(const char* filepath){
    ////printf("video reader constructed\n");
    // Initialize state so destructor can safely clean up even if file_open() fails.
    this->state = {};
    this->state.video_stream_index = -1;

    printf("open file\n");
    this->file_open(filepath);
}
VideoReader::~VideoReader(){
    VideoReaderState* state = &this->state;
    delete[] state->frame_buffer;
    sws_free_context(&state->sws_scaler_context);
    avformat_close_input(&state->av_format_context);
    avformat_free_context(state->av_format_context);
    av_frame_free(&state->av_frame);
    av_packet_free(&state->av_packet);
    avcodec_free_context(&state->av_codec_context);
}

bool VideoReader::jump_to_ts(float ts_sec, double* pt_seconds, int64_t* pts){
    // glfwSetTime(ts_sec);
    *pt_seconds = ts_sec;
    *pts = (int64_t)(*pt_seconds  * (double)this->state.time_base.den / (double)this->state.time_base.num);
    this->seek_frame(*pts);

    return true;
}

bool VideoReader::file_open(const char* filename){
    bool debug_print = true;
    this->state.av_format_context = avformat_alloc_context();
    if (!this->state.av_format_context){
        throw std::runtime_error("não criou avformatcontext");
        return false;
    }

    if (avformat_open_input(&this->state.av_format_context, filename, NULL, NULL) != 0){
        throw std::runtime_error("nao abriu video\n");
        return false;
    }
    const AVCodecParameters* av_codec_params = nullptr;
    const AVCodec* av_codec = nullptr;
    for(int i = 0; i < this->state.av_format_context->nb_streams; ++i){
        av_codec_params = this->state.av_format_context->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);
        if (!av_codec){
            continue;
        }
        if(av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            this->state.video_stream_index = i;
            this->state.time_base = this->state.av_format_context->streams[i]->time_base;
            break;
        }
    }

    if (this->state.video_stream_index == -1){
        throw std::runtime_error("nao achou uma video stream válida\n");
        return 0;
    }

    this->state.av_codec_context = avcodec_alloc_context3(av_codec);
    if(!this->state.av_codec_context){
        throw std::runtime_error("nao criou avcodeccontext\n");
        return 0;
    }

    if(avcodec_parameters_to_context(this->state.av_codec_context, av_codec_params) < 0){
        throw std::runtime_error("nao iniciou avcodeccontext\n");
        return 0;
    }

    if(avcodec_open2(this->state.av_codec_context, av_codec, NULL) < 0){
        throw std::runtime_error("nao abriu codec\n");
        return 0;
    }
    
    this->state.av_frame = av_frame_alloc();
    if(!this->state.av_frame){
        throw std::runtime_error("nao alocou avframe\n");
        return 0;
    }
    
    this->state.frame_buffer = new uint8_t[this->state.av_codec_context->width * this->state.av_codec_context->height * 4];
    this->state.av_packet = av_packet_alloc();
    if (!this->state.av_packet){
        throw std::runtime_error("nao alocou avpacket\n");
        return 0;
    }
// ////printf("width: %d, height: %d, pix_fmt codec_params: %d\n", 
//     av_codec_params->width, 
//     av_codec_params->height, 
//     av_codec_params->format);
    this->w = av_codec_params->width;
    this->h = av_codec_params->height;

    // this->state.sws_scaler_context = sws_getContext(
    // av_codec_params->width, av_codec_params->height, (AVPixelFormat)av_codec_params->format,
    // av_codec_params->width, av_codec_params->height, AV_PIX_FMT_RGB0,
    // SWS_BILINEAR, NULL, NULL, NULL);
    // // this->state.sws_scaler_context = sws_getContext(this->state.av_codec_context->width, this->state.av_codec_context->height, this->state.av_codec_context->pix_fmt, this->state.av_codec_context->width, this->state.av_codec_context->height, AV_PIX_FMT_RGB0, SWS_BILINEAR, NULL, NULL, NULL);
    // if (!this->state.sws_scaler_context){
    //     std::cout << "não iniciou o sws scaler" << std::endl;
    //     return 0;
    // }
    return 1;
}
bool VideoReader::read_frame(){
    ////printf("a\n");
    
    int res;
    while (av_read_frame(this->state.av_format_context, this->state.av_packet) >= 0){
    ////printf("b\n");
        if(this->state.av_packet->stream_index != this->state.video_stream_index){
            av_packet_unref(this->state.av_packet);
            continue;
        }    
        res = avcodec_send_packet(this->state.av_codec_context, this->state.av_packet);
        if (res < 0){
            std::cout << "falhou decode packet " << av_err2str(res) << std::endl;
            
            return 0;
        }
        res = avcodec_receive_frame(this->state.av_codec_context, this->state.av_frame);
        if (res == AVERROR(EAGAIN) || res == AVERROR_EOF){
            av_packet_unref(this->state.av_packet);
            continue;
        } else if (res < 0){
            std::cout << "falhou decode packet " << av_err2str(res) << std::endl;

        }
        av_packet_unref(this->state.av_packet);
    ////printf("c\n");

        break;
    }

    this->pts = this->state.av_frame->pts;
    //printf("read pts %d in sec %f\n", pts, (double)pts * get_time_base());
    

    if (!this->state.sws_scaler_context) {
        this->state.sws_scaler_context = sws_getContext(
            this->state.av_frame->width, this->state.av_frame->height, this->state.av_codec_context->pix_fmt,
            this->state.av_frame->width, this->state.av_frame->height, AV_PIX_FMT_RGB0,
            SWS_BILINEAR, NULL, NULL, NULL);
        if (!this->state.sws_scaler_context){
            std::cout << "não iniciou o sws scaler" << std::endl;
            return 0;
        }
    }

    uint8_t* dest[4] = {this->state.frame_buffer, NULL, NULL, NULL};
    int dest_linesize[4] = {this->state.av_frame->width * 4, 0, 0, 0};
    ////printf("before sws\n");
    int h = sws_scale(this->state.sws_scaler_context, this->state.av_frame->data, this->state.av_frame->linesize, 0, this->state.av_frame->height, dest, dest_linesize);
    ////printf("sws h %i", h);
    // *out_width = this->state.av_frame->width;
    // *out_height = this->state.av_frame->height;
    // *frame_buffer = data;
    // data = nullptr;
    
    //printf("reedeede\n");
    return 1;
}

double VideoReader::get_time_base(){
    return (double)this->state.time_base.num / (double)this->state.time_base.den;
}


// int64_t VideoReader::get_(){
//     return (int64_t)(ts * AV_TIME_BASE);
// }


bool VideoReader::seek_frame(double ts){
    AVStream* stream = this->state.av_format_context->streams[this->state.video_stream_index];
    int64_t timestamp = av_rescale_q((int64_t)(ts * AV_TIME_BASE), AV_TIME_BASE_Q, stream->time_base);
    
    int res = av_seek_frame(this->state.av_format_context, this->state.video_stream_index, timestamp, AVSEEK_FLAG_BACKWARD);
    if (res < 0){
        std::cout << "falhou seek: " << av_err2str(res) << std::endl;
        return false;
    }
    avcodec_flush_buffers(this->state.av_codec_context);

    // avança até o frame mais próximo do ts
    double pts_in_sec = 0.0;
    do {
        read_frame();
        pts_in_sec = (double)this->pts * get_time_base();
    } while (pts_in_sec < ts - get_time_base()); // tolerância de 1 frame

    return true;
}
