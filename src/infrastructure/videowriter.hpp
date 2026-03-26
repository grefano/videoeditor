#pragma once
#include "ffmpeg.h"
#include "implimgui.h"
struct OutputVideoState{
  SwsContext* sws = nullptr;
  const AVCodec* codec = nullptr;
  AVCodecContext* codec_ctx = nullptr;
  AVFormatContext* fmt_ctx = nullptr;
  AVStream* stream = nullptr;
  AVFrame* frame = nullptr;

  

};

struct videoWriterFfmpeg{
  OutputVideoState state;
  int frame_index = 0;
  void flush_packets() {
      AVPacket* packet = av_packet_alloc();
      while (avcodec_receive_packet(state.codec_ctx, packet) == 0) {
          packet->stream_index = state.stream->index;
          av_packet_rescale_ts(packet, state.codec_ctx->time_base, state.stream->time_base);
          if (int res = av_interleaved_write_frame(state.fmt_ctx, packet) < 0){
            printf("err %d\n", res);
              throw std::runtime_error("write frame");

          }
          av_packet_unref(packet);
      }
      av_packet_free(&packet);
  }

  //@param dim output dimensions
  void init(ImVec2 indim, ImVec2 outdim, const char* name){
    auto& frame = state.frame;
    auto& sws = state.sws;
    auto& codec = state.codec;
    auto& codec_ctx = state.codec_ctx;
    auto& fmt_ctx = state.fmt_ctx;
    auto& stream = state.stream;
    this->frame_index = 0;

    frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = outdim.x;
    frame->height = outdim.y;
    av_frame_get_buffer(frame, 32); // ???
    
    sws = sws_getContext(
      indim.x, indim.y, AV_PIX_FMT_RGBA,
      outdim.x, outdim.y, AV_PIX_FMT_YUV420P,
      SWS_BILINEAR, nullptr, nullptr, nullptr
    
    );

    // AVFormatContext* fmt_ctx = nullptr;
    avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, name);

    AVRational fps = {30, 1};
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->width = outdim.x;
    codec_ctx->height = outdim.y;
    codec_ctx->time_base = {1, 90000};//{fps.den, fps.num};
    codec_ctx->framerate = fps;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 0;
    if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avcodec_open2(codec_ctx, codec, NULL);

    stream = avformat_new_stream(fmt_ctx, codec);
    stream->time_base = codec_ctx->time_base;
    if (avcodec_parameters_from_context(stream->codecpar, codec_ctx) < 0){ // ? pq stream precisa ter "copia" dos parametros
      throw std::runtime_error("avcodec parameters from context");
    }

    if (avio_open(&fmt_ctx->pb, name, AVIO_FLAG_WRITE) < 0){
      throw std::runtime_error("avio open");
    }

    if (avformat_write_header(fmt_ctx, nullptr) < 0){
      throw std::runtime_error("avformat write header");
    }

    printf("codec time_base: %d/%d\n", codec_ctx->time_base.num, codec_ctx->time_base.den);
    printf("stream time_base: %d/%d\n", stream->time_base.num, stream->time_base.den);
  }
  void add_frame(std::vector<uint8_t>& pixels){
    printf("frame index %d\n", frame_index);
    assert(pixels.data() != nullptr);

    auto& frame = state.frame;
    auto& sws = state.sws;
    auto& codec = state.codec;
    auto& codec_ctx = state.codec_ctx;
    auto& fmt_ctx = state.fmt_ctx;
    auto& stream = state.stream;


    uint8_t* inData[1] = { pixels.data() };
    ImVec2 outdim = {state.codec_ctx->width, state.codec_ctx->height};
    int inLinesize[1] = { 4 *  outdim.x};

    av_frame_make_writable(frame);
    printf("s %d d %d l %d 0 %d y %d d %d l %d",  sws, inData, inLinesize, 0, outdim.y, frame->data, frame->linesize);
    sws_scale( sws, inData, inLinesize, 0, outdim.y, frame->data, frame->linesize);


    printf("timebase %d %d\n", codec_ctx->time_base.num, codec_ctx->time_base.den); // 50 * 1
    frame->pts = av_rescale_q(frame_index, (AVRational){codec_ctx->framerate.den, codec_ctx->framerate.num}, codec_ctx->time_base);///this->frame_index * codec_ctx->time_base.den / codec_ctx->time_base.num; // !!! unidade provavelmente errada
    this->frame_index++;
    int res = avcodec_send_frame(codec_ctx, frame);
    printf("send frame %i\n", res);
    if (res < 0){
      printf("drnfg %d ", res);
      throw std::runtime_error("send frame");
      
    }
    flush_packets();


  }
  void end(){
    auto& frame = state.frame;
    auto& sws = state.sws;
    auto& codec = state.codec;
    auto& codec_ctx = state.codec_ctx;
    auto& fmt_ctx = state.fmt_ctx;
    auto& stream = state.stream;
    stream->duration = frame_index;


    avcodec_send_frame(codec_ctx, nullptr);

    AVPacket* pkt = av_packet_alloc();
    
    while (avcodec_receive_packet(codec_ctx, pkt) == 0) {
      pkt->stream_index = stream->index;
      printf("pkt dts %" PRIu64 " pts %" PRIu64 "\n", pkt->dts, pkt->pts);
      av_interleaved_write_frame(fmt_ctx, pkt);
      av_packet_unref(pkt);
    }

    av_packet_free(&pkt);

    av_write_trailer(fmt_ctx);
    avio_close(fmt_ctx->pb);
    av_frame_free(&frame);
  }
}; 