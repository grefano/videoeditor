#include "opengl.h"
#include "ffmpeg.h"
#include "implglfw.h"
#include "implimgui.h"
#include "videowriter.hpp"


void logi(){
  static int debugindex = 0;
  printf("%d\n", debugindex);
  debugindex++;
}


class Renderfile{
  private:
  videoWriterFfmpeg videowriter;

  public:
  bool is_rendering = false;
  float tl_time = 0;
  ImVec2 indim = {0,0};

  GLuint tex = 0;
  GLuint fbo = 0;
  std::vector<uint8_t> pixels;

  std::function<bool(float, GLuint&, GLuint&)> cb_update_tex;
  void init(ImVec2 dim);
  void init_tex(ImVec2 indim);
  void tex_to_pixels();
  void end();
  void update();
  void render(ImVec2);
  void add_tex_frame();
  ~Renderfile(){
    end();
  }
};


void Renderfile::init_tex(ImVec2 indim){
  glGenTextures(1, &this->tex);
  logi();
  glGenFramebuffers(1, &this->fbo);
  logi();


   printf("init tex %d fbo %d\n", tex, fbo);
  // glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  geterr("bind framebuffer");
  glBindTexture(GL_TEXTURE_2D, tex);
  geterr("bind texx");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, indim.x, indim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  geterr("tex image 2d"); 

  printf("inited tex\n");
}
void Renderfile::init(ImVec2 indim){
  printf("init\n");
  // this->dim = dim;
  is_rendering = true;

  pixels.clear();
  pixels.resize(indim.x * indim.y * 4);
  this->indim = indim;
 init_tex(indim);
  try{
    videowriter.init(indim, {640, 360}, "output.mp4");
  } catch (std::exception& e){
    printf("error init videowriter %s\n", e.what());
    videowriter.end();
    throw 1;
  }
  
}
void Renderfile::render(ImVec2 time_region){
  assert(time_region.y > time_region.x);
  assert(cb_update_tex);
  init({640, 360});
  for(float t = time_region.x; t < time_region.y; t+=1.0f/30.0f){
      printf("t %f\n", t);
      if (this->cb_update_tex(t, tex, fbo)){//render.update_tex(&tl, t, renderfile.tex, renderfile.fbo
        add_tex_frame();
      }

  }
  end();


}
void pixels_yuv_create(std::vector<uint8_t>& pixels, int frame){
  for(int i = 0; i < pixels.size(); i+=4){
    // pixels[i] = 100 + sin(i/100)*100;
    float a = (sin((float)frame + (float)i/20.0f) + 1)/2;
    pixels[i] = 255 * a;
    pixels[i+1] = 255 * (1-a);
    pixels[i+2] = 0;
    pixels[i+3] = 255;
  }
}
void Renderfile::tex_to_pixels(){
    assert(fbo != 0);
  assert(tex != 0);

  glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
  geterr();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex, 0);
  geterr();
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  geterr();
  
}

// add frame 
void Renderfile::add_tex_frame(){
  // GLuint& fbo = this->fbo;
  printf("--render\n");
// static double then = 0;
//     double now = glfwGetTime();
//     if (now - then < 1.0f/30.0f){
//         return;
//     }
    
 
  printf("tex %d fbo %d\n", tex, fbo);

  assert(indim.x > 0 && indim.y > 0);
  
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  printf("fbo status %i\n", glCheckFramebufferStatus(GL_FRAMEBUFFER) );
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
    throw std::runtime_error("fbo incompleto");
  }
  tex_to_image(tex, pixels.data(), indim.x, indim.y); // !!! pixels e tex podem ter dimensoes diferentes
  printf("pixel %" PRIu8 "\n", pixels[0]);
glBindFramebuffer(GL_FRAMEBUFFER, 0);

  videowriter.add_frame(pixels);
  
}

void Renderfile::end(){
  is_rendering = false;

  glDeleteTextures(1, &tex);
  glDeleteFramebuffers(1, &fbo);

  videowriter.end();

}