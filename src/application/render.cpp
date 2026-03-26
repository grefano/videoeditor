#include "render.hpp"
#include "log.hpp"
static bool debug = false;

void ClipReadWrite::get_clip_tex_result(std::list<std::unique_ptr<ComponentShader>>* components, GLuint raw_tex, GLuint result_tex, GLuint fbo){
  PROFILE_FUNCTION();

  for(auto& compptr : *components){
    log("it comp\n");
    log("COMP\n");
    compptr.get()->get_tex(raw_tex, result_tex, fbo);
  }
}

void ClipReadWrite::update_image(VideoReader* reader, float ts){
  PROFILE_FUNCTION();
  double pts_in_sec = (double)reader->pts * reader->get_time_base();
  double diff = (double)ts - pts_in_sec;
      log("pts in sec %f time base %f pts %d\n", pts_in_sec, reader->get_time_base(), reader->pts);
  if (diff < -1 || diff > 1){
      log("seek. diff=%f\n", diff);
      reader->seek_frame((double)ts);
      reader->read_frame();
      
  } else if ((double)ts > pts_in_sec){
      log("read\n");
      reader->read_frame();
  }
  log("w %f h %f", reader->w, reader->h);
  log("image to text\n");

  


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

void Render::update_tex(Timeline* tl, GLuint& outtex){
  PROFILE_FUNCTION();
  static double then = 0;
  double now = glfwGetTime();
  if (now - then < 1.0f/30.0f){
    return;
  }
  then = now;
  // walker -> lista de clipes na ordem certa -> chamar clip.get_image pra todos -> imagem final
  std::list<Clip*> clips;
  WalkerTimeline::walk(tl, &clips);
  // log("--clip walk size=%zu\n", clips.size());
  int i = 0;
  // GLuint& outtex = this->playhead_tex;
  for (Clip* clip : clips){
      log("clip t0 %f t1 %f\n", clip->tl_time0, clip->tl_time1);
      float rel_ts = tl->playhead_time - clip->tl_time0;
      if (tl->playhead_time < clip->tl_time0 || tl->playhead_time > clip->tl_time1){
        continue;
      }

      log("textures clip %d clip res %d playhead %d\n fbo %d\n", this->clip_tex, this->clip_result_tex, outtex, this->fbo);
   
      std::visit([this, clip, rel_ts](auto master){ this->get_tex(clip, master, rel_ts); }, clip->masterclip);

      if (i == 0) {
          overlap_textures(this->clip_result_tex, this->clip_result_tex, outtex, this->fbo, this->shd_overlap);
      } else {
          overlap_textures(this->clip_result_tex, outtex, this->temp_tex, this->fbo, this->shd_overlap);
          std::swap(outtex, this->temp_tex);
      }
      i++;
  };
}
void Render::get_tex(Clip* clip, VideoClip* masterclip, float rel_ts){
    PROFILE_FUNCTION();

  log("visit VIDEOCLIP\n");
  log("source path %s\n t0 %f t1 %f\n", masterclip->source->filepath, clip->tl_time0, clip->tl_time1);

  VideoReader* reader = &masterclip->reader;
  ClipReadWrite::update_image(reader, rel_ts);
  log("w %f h %f\n", reader->w, reader->h);
  log("textures clip %d clip res %d playhead %d\n fbo %d\n", this->clip_tex, this->clip_result_tex, this->playhead_tex, this->fbo);
  image_to_tex(this->clip_tex, reader->state.frame_buffer, reader->w, reader->h);
  ClipReadWrite::get_clip_tex_result(&clip->shader_components, this->clip_tex, this->clip_result_tex, this->fbo);

}
