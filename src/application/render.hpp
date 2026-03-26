#pragma once
#include "timelinewalker.hpp"
#include "profile.hpp"
#include <variant>

namespace ClipReadWrite{
    void get_clip_tex_result(std::list<std::unique_ptr<ComponentShader>>* components, GLuint raw_tex, GLuint result_tex, GLuint fbo);
    void update_image(VideoReader* reader, float ts);
}

class Timeline;
class Render{
  public:

  GLuint temp_tex = 0;
  GLuint clip_tex = 0;
  GLuint clip_result_tex = 0;
  GLuint shd_overlap = 0;
  GLuint fbo = 0;
  ImVec2 preview_dimensions;
  // GLuint playhead_tex = 0;

  void init_shader();
  bool update_tex(Timeline*, float, GLuint&,GLuint&);
  void render(GLuint fbo);
  Render(ImVec2 preview_dimensions);
  ~Render();
  void get_tex(Clip* clip, VideoClip* masterclip, float rel_ts);
  
  private:
  uint8_t* playhead_frame = nullptr;
 
};

