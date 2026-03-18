#pragma once
#include "timelinewalker.hpp"
#include "profile.hpp"

namespace ClipReadWrite{
    void get_clip_tex_result(std::list<std::unique_ptr<ComponentShader>>* components, GLuint raw_tex, GLuint result_tex, GLuint fbo);
    void update_image(VideoReader* reader, float ts);
}

class Timeline;
struct RenderClipVisitor : public ClipVisitor{
  void visit(VideoClip& masterclip, Clip* clip, Render* render, float rel_ts) override;
};
class Render{
  uint8_t* playhead_frame = nullptr;
  RenderClipVisitor clipVisitor;
  public:
  GLuint temp_tex = 0;
  GLuint clip_tex = 0;
  GLuint clip_result_tex = 0;
  GLuint shd_overlap = 0;
  GLuint fbo = 0;
  ImVec2 preview_dimensions;
  GLuint playhead_tex = 0;

  void init_shader();
  void update_preview_tex(Timeline*);
  void render();
  Render(ImVec2 preview_dimensions);
  ~Render();
  void dispatch(Clip* clip, VideoClip* masterclip, float rel_ts);
};
