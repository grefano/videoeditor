#include "renderfile.hpp"

int main(){
  return 0;
  if (!glfwInit()){
    printf("%s\n", "falha inicializando glfw");
    return -1;
  }
  
  Glfw glfw;
  Imgui imgui(glfw.window_);
  
  gladLoadGL();
  
  Renderfile render;
  printf("created render\n");
  render.init({640, 360});
  
  
  
  printf("init\n");
  for(int i = 0; i < 60; i++){
    try{
      // pixels_yuv_create(render.pixels, i);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render.tex, 0);
      geterr("fbo tex 2d");
      glClearColor((sin((float)i / 2) + 1)/2, 1.0, 0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      render.render();
      printf("render\n");
    } catch (std::exception& e){
      render.end();
      printf("render error: %s\n", e.what());
      throw e;
    }
  }
  render.end();
  printf("end\n");
  return 0;
}