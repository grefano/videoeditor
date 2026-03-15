#include "render.hpp"


bool debug = false;




void ClipReadWrite::get_clip_tex_result(std::list<std::unique_ptr<ComponentShader>>* components, GLuint raw_tex, GLuint result_tex, GLuint fbo){
  for(auto& compptr : *components){
    if (debug){
      printf("it comp\n");
    }
    printf("COMP\n");
    compptr.get()->get_tex(raw_tex, result_tex, fbo);
  }

}

void ClipReadWrite::update_image(VideoReader* reader, float ts){
  double pts_in_sec = (double)reader->pts * reader->get_time_base();
  double diff = (double)ts - pts_in_sec;
  if (debug)
      printf("pts in sec %f time base %f pts %d\n", pts_in_sec, reader->get_time_base(), reader->pts);
  if (diff < -1 || diff > 1){
      if (debug)
          printf("seek. diff=%f\n", diff);
      reader->seek_frame((double)ts);
      reader->read_frame();
      
  } else if ((double)ts > pts_in_sec){
      if (debug)
          printf("read\n");
      reader->read_frame();
  }
  if (debug){
      printf("w %f h %f", reader->w, reader->h);
      printf("image to text\n");

  }
  


}


class Timeline;

void Render::init_shader(){
  glGenFramebuffers(1, &this->fbo);
  glGenTextures(1, &this->clip_tex);
  glGenTextures(1, &this->clip_result_tex);
  glGenTextures(1, &this->playhead_tex);
  glGenTextures(1, &this->temp_tex);
  this->shd_overlap= createShader(vs, fs);

    
}
Render::Render(ImVec2 preview_dimensions) : preview_dimensions(preview_dimensions){
  this->init_shader();

}
Render::~Render(){
  glDeleteFramebuffers(1, &this->fbo);
  glDeleteTextures(1, &this->playhead_tex);
  glDeleteTextures(1, &this->clip_result_tex);
  glDeleteTextures(1, &this->clip_tex);
  glDeleteProgram(this->shd_overlap);
}
void Render::update_preview_tex(Timeline* tl){
  static double then = 0;
  double now = glfwGetTime();
  if (now - then < 1.0f/30.0f){
    return;
  }
  then = now;
  // walker -> lista de clipes na ordem certa -> chamar clip.get_image pra todos -> imagem final
  std::list<Clip*> clips;
  WalkerTimeline::walk(tl, &clips);
  // printf("--clip walk size=%zu\n", clips.size());
  int i = 0;
  
  for (Clip* clip : clips){
      printf("clip t0 %f t1 %f\n", clip->tl_time0, clip->tl_time1);
      float rel_ts = tl->playhead_time - clip->tl_time0;
      if (tl->playhead_time < clip->tl_time0 || tl->playhead_time > clip->tl_time1){
        continue;
      }

      printf("textures clip %d clip res %d playhead %d\n fbo %d\n", this->clip_tex, this->clip_result_tex, this->playhead_tex, this->fbo);
      
      clip->masterclip->accept(&this->clipVisitor,clip, this, rel_ts);

      // dispatch(clip, clip->masterclip, rel_ts);

      if (i == 0) {
          overlap_textures(this->clip_result_tex, this->clip_result_tex, this->playhead_tex, this->fbo, this->shd_overlap);
      } else {
          overlap_textures(this->clip_result_tex, this->playhead_tex, this->temp_tex, this->fbo, this->shd_overlap);
          std::swap(this->playhead_tex, this->temp_tex);
      }
      i++;
  };

}
void RenderClipVisitor::visit(VideoClip& masterclip, Clip* clip, Render* render, float rel_ts){
  printf("visit VIDEOCLIP\n");
  printf("source path %s\n t0 %f t1 %f\n", masterclip.source->filepath, clip->tl_time0, clip->tl_time1);
  VideoReader* reader = &masterclip.reader;
  ClipReadWrite::update_image(reader, rel_ts);
  printf("w %f h %f\n", reader->w, reader->h);
  printf("textures clip %d clip res %d playhead %d\n fbo %d\n", render->clip_tex, render->clip_result_tex, render->playhead_tex, render->fbo);
  image_to_tex(render->clip_tex, reader->state.frame_buffer, reader->w, reader->h);
  ClipReadWrite::get_clip_tex_result(&clip->shader_components, render->clip_tex, render->clip_result_tex, render->fbo);

}
// void Render::dispatch(Clip* clip, VideoClip* masterclip, float rel_ts){

//   VideoReader* reader = &masterclip->reader;
//   ClipReadWrite::update_image(reader, rel_ts);

//   printf("textures clip %d clip res %d playhead %d\n fbo %d\n", this->clip_tex, this->clip_result_tex, this->playhead_tex, this->fbo);
//   image_to_tex(this->clip_tex, reader->state.frame_buffer, reader->w, reader->h);
//   ClipReadWrite::get_clip_tex_result(&clip->shader_components, this->clip_tex, this->clip_result_tex, this->fbo);

// }
