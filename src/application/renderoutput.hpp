#pragma once
#include "ffmpeg.h"
#include "opengl.h"
#include "imgui.h"
/*
class Renderfile{
  public:
  ImVec2 dim = {0,0};
  int frame_index = 0;

  GLuint tex = 0;
  GLuint fbo = 0;
  std::vector<uint8_t> pixels;

  SwsContext* sws = nullptr;
  const AVCodec* codec = nullptr;
  AVCodecContext* codec_ctx = nullptr;
  AVFormatContext* fmt_ctx = nullptr;
  AVStream* stream = nullptr;
  AVFrame* frame = nullptr;

  void init();
  void end();
  void render();
};

void Renderfile::init(){
  this->dim = {640, 360};
  
  glGenTextures(1, &this->tex);
  glGenFramebuffers(1, &this->fbo);
  
  pixels.clear();
  pixels.resize(dim.x * dim.y * 4);
  this->frame_index = 0;
  
  printf("tex %d fbo %d\n", tex, fbo);
  // glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dim.x, dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  this->frame = av_frame_alloc();
  frame->format = AV_PIX_FMT_YUV420P;
  frame->width = dim.x;
  frame->height = dim.y;
  av_frame_get_buffer(frame, 32);
  
  this->sws = sws_getContext(
    dim.x, dim.y, AV_PIX_FMT_RGBA,
    dim.x, dim.y, AV_PIX_FMT_YUV420P,
    SWS_BILINEAR, nullptr, nullptr, nullptr
  
  );

  // AVFormatContext* fmt_ctx = nullptr;
  avformat_alloc_output_context2(&this->fmt_ctx, nullptr, nullptr, "output.mp4");


  this->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  this->codec_ctx = avcodec_alloc_context3(codec);
  codec_ctx->width = dim.x;
  codec_ctx->height = dim.y;
  codec_ctx->time_base = {1, 10};
  codec_ctx->framerate = {10, 1};
  codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  codec_ctx->gop_size = 10;
  codec_ctx->max_b_frames = 0;
  if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
      codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  avcodec_open2(codec_ctx, codec, NULL);

  this->stream = avformat_new_stream(fmt_ctx, codec);
  stream->time_base = codec_ctx->time_base;
  if (avcodec_parameters_from_context(stream->codecpar, codec_ctx) < 0){
    throw std::runtime_error("avcodec parameters from context");
  }

  if (avio_open(&fmt_ctx->pb, "output.mp4", AVIO_FLAG_WRITE) < 0){
    throw std::runtime_error("avio open");
  }

  if (avformat_write_header(fmt_ctx, nullptr) < 0){
    throw std::runtime_error("avformat write header");
  }
}
void Renderfile::render(){
  // GLuint& fbo = this->fbo;
  assert(fbo != 0);
  assert(tex != 0);
static double then = 0;
    double now = glfwGetTime();
    if (now - then < 1.0f/30.0f){
        return;
    }
    
  printf("tex %d fbo %d\n", tex, fbo);

  ImVec2& dim = this->dim;
  auto& frame = this->frame;
  auto& pixels = this->pixels;
  auto& sws = this->sws;
  auto& codec = this->codec;
  auto& codec_ctx = this->codec_ctx;
  auto& fmt_ctx = this->fmt_ctx;

  assert(dim.x > 0 && dim.y > 0);
  // ImVec2 dim = {640, 360};
  // std::vector<uint8_t> pixels(dim.x * dim.y * 4);
  
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  
  
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
    throw std::runtime_error("fbo incompleto");
  }
  glReadPixels(0,0,dim.x,dim.y,GL_RGBA,GL_UNSIGNED_BYTE, pixels.data());
  assert(pixels.data() != nullptr);
  // AVFrame* frame = av_frame_alloc();
  // frame->format = AV_PIX_FMT_YUV420P;
  // frame->width = dim.x;
  // frame->height = dim.y;
  // frame->pts = 0;
  // av_frame_get_buffer(frame, 32);

  // SwsContext* sws = sws_getContext(
  //   dim.x, dim.y, AV_PIX_FMT_RGBA,
  //   dim.x, dim.y, AV_PIX_FMT_YUV420P,
  //   SWS_BILINEAR, nullptr, nullptr, nullptr
  
  // );

  uint8_t* inData[1] = { pixels.data() };
  int inLinesize[1] = { 4 * dim.x };

  av_frame_make_writable(frame);
  printf("s %d d %d l %d 0 %d y %d d %d l %d",  sws, inData, inLinesize, 0, dim.y, frame->data, frame->linesize);
  sws_scale( sws, inData, inLinesize, 0, dim.y, frame->data, frame->linesize);

  // AVFormatContext* fmt_ctx = nullptr;
  // avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, "output.mp4");

  // const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  // AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
  // codec_ctx->width = dim.x;
  // codec_ctx->height = dim.y;
  // codec_ctx->time_base = {1, 60};
  // codec_ctx->framerate = {60, 1};
  // codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  // avcodec_open2(codec_ctx, codec, NULL);
  

  frame->pts = frame_index++;
  int res = res = avcodec_send_frame(codec_ctx, frame);
  printf("send frame %i\n", res);
  if (res < 0){
    printf(" %d ", res);
    throw std::runtime_error("send frame");
  
  }

  AVPacket* packet = av_packet_alloc();


  while(avcodec_receive_packet(codec_ctx, packet) == 0){
  printf("got packet\n");

    packet->stream_index = stream->index;
    if(av_interleaved_write_frame(fmt_ctx, packet)){
      throw std::runtime_error("write frame");
    }
    av_packet_unref(packet);
  }

  av_packet_free(&packet);

}

void Renderfile::end(){
  glDeleteTextures(1, &tex);

  avcodec_send_frame(codec_ctx, nullptr);

  AVPacket* pkt = av_packet_alloc();

  while (avcodec_receive_packet(codec_ctx, pkt) == 0) {
    pkt->stream_index = stream->index;
      av_interleaved_write_frame(fmt_ctx, pkt);
      av_packet_unref(pkt);
  }

  av_packet_free(&pkt);

  av_write_trailer(fmt_ctx);
  avio_close(fmt_ctx->pb);
  av_frame_free(&frame);

}
  */