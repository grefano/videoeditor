#include <iostream>
#include <fstream>
#include <future>
#include <memory>
#include <glad.h>



#include "ffmpeg.h"
#include "timeline.hpp"
#include "ui.h"
#include "implimgui.h"
#include "implglfw.h"
#include "mediapool.h"
#include "render.hpp"
#include "import.hpp"
// #include "renderoutput.hpp"
#include "renderfile.hpp"
/*
clipvideo tem o videoreader -> clipvideo nao deve herdar o clip, e sim ter como membro -> mas onde armazenar o clipvideo?
/ registro (clip-videoref-videostate) -> clipvideo não é um objeto, só lê o videoref com base
``` tl.add_clip(t0, t1); render{ get_clip_tex(&clip, ) }

problema: 
/ clipvideo herdar clip e ter videostate -> como separar  
/ clipvideo references clip e ter videostate -> 

*/

namespace TL_APPLICATION{
    Mediapool* mediapool;
    Timeline* tl;
    Clip* add_clip(size_t track, ImVec2 time_tl, const char* filepath){
        MediaSource* file1 = (*mediapool).add_file(filepath);
        log(file1->filepath);
        Clip* clip = (*tl).add_clip(track, time_tl.x, time_tl.y);
        float scale = 2;
        // auto comp = clip->add_component<Transform>();
        // comp->scale = {scale, scale};
        // comp->position = {.5,.5};
        clip->masterclip = new VideoClip(file1);
        std::get<VideoClip*>(clip->masterclip)->source = file1;
        return clip;
    }
    
}
using namespace TL_APPLICATION;

void render_dur(Renderfile& renderfile, double dur){
    
    static bool init = true;
    static double tstart = glfwGetTime();
    double tnow = glfwGetTime();
    printf("init %i start %f end %f\n", init, tstart, tnow);
    if (init){
        // renderfile.init();
        init = false;
    }
    if (tnow > tstart + dur){
        renderfile.end();
        assert(false);
    }

  
    printf("render add frame\n");
    renderfile.add_tex_frame();

    
    
}




// PreviewUI UIpreview2("preview2");
int main(){
    if (!glfwInit()){
        log("%s\n", "falha inicializando glfw");
        return -1;
    }
    
    Glfw glfw;
    Imgui imgui(glfw.window_);
    
    gladLoadGL();
    Instrumentor::Get().BeginSession("ffmpeg");
    InstrumentationTimer profile_before = {"before-loop"};

    Timeline tl;
    TL_APPLICATION::tl = &tl;
    glfwSetWindowUserPointer(glfw.window_, &tl);
    glfwSetKeyCallback(glfw.window_, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto tl = static_cast<Timeline*>(glfwGetWindowUserPointer(window));
        tl->key_callback(key, action);
    });
    Render render(ImVec2(640, 360));
    Renderfile renderfile;
    renderfile.cb_update_tex = [&render, &tl](float t, GLuint& tex, GLuint& fbo){ return render.update_tex(&tl, t, tex, fbo); };
    // renderfile.fbo = render.fbo;
    log("render");
    Mediapool mediapool;
    TL_APPLICATION::mediapool = &mediapool;
    Import import = {&mediapool};
    
    log("mediapool");
    
    /*
    botao importar
    ~ pegar filepath
    validar filepath
    validar arquivo
    decidir qual mediasource concreto vai ser usado
    source = create_mediasource(filepath)


    */
    Clip* clip = add_clip(0, {5, 20}, "teste.mp4");
    Transform* comp = clip->add_component<Transform>();
    comp->position = {0, 0};
    comp->scale = {2, 2};

    clip = add_clip(1, {-5, 10}, "video3.mp4");
    comp = clip->add_component<Transform>();
    comp->position = {0, 0};
    comp->scale = {1, 1};
    log("clip");


    TimelineUI UItl(&tl);
    PreviewUI UIpreview;
    ImportUI UIimport;
    UIimport.app.import = [&import](char* filepath){ import.import_filepath(filepath); };
    MediapoolUI UImediapool;
    RenderUI UIrender;
    UIrender.cb_start = [&renderfile](ImVec2 time){ renderfile.render(time); };

    // renderfile.init({640, 360});
    profile_before.Stop();
    double lasttime = glfwGetTime();
    double timeaccum = 0.0f;
    while (!glfwWindowShouldClose(glfw.window_)) {
        double now = glfwGetTime();
        double dt = now - lasttime;     
        timeaccum+=dt;   
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        // comp2->position.x -= dt*.001;
    log("update");
        
    tl.update(dt);

        // static double then = 0;
        // double now = glfwGetTime();
        printf("time diff %f\n", now - lasttime);
        if (timeaccum >= 1.0f/30.0f){
            render.update_tex(&tl, tl.playhead_time, tl.playhead_tex, render.fbo);
        
            timeaccum = 0;
        } 
        lasttime = now;
            // overlap_textures(0, tl.playhead_tex, renderfile.tex, renderfile.fbo, render.shd_overlap);
            // render_dur(renderfile, 5);
            
        // render.render();
        log("draw tl");
        // log("tex = %d\n", tl.playhead_tex);
        log("draw preview");
        // log("DIM %d %d", clip->, clip->h);
        ImGui::Image( render.clip_tex, ImVec2(640, 360));
        ImGui::Image( render.clip_result_tex, ImVec2(640, 360));

        ImGui::Image( renderfile.tex, ImVec2(640, 360));
        
        
        UIimport.draw();
        UImediapool.draw(&mediapool);
        UItl.draw();
        UIpreview.draw(&tl, tl.playhead_tex, render.preview_dimensions);
        UIrender.draw();


        ImGui::Render();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(glfw.window_); // restaura o contexto
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        
        

        glfwSwapBuffers(glfw.window_);
        glfwPollEvents();
        
    }
    renderfile.end();

    Instrumentor::Get().EndSession();
    return 0;
}