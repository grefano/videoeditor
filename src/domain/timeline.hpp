#pragma once
#include "opengl.h"
#include "ffmpeg.h"
#include "shadercomponent.hpp"
#include <list>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <imgui.h>
class Render;
struct VideoClip;
struct Clip;
struct ClipVisitor{
    virtual void visit(VideoClip& masterclip, Clip* clip, Render* render, float rel_ts) = 0;
};
struct MasterClip{ // liga o clipe ao mediasource e ao filereader, decide como renderização acontece
    MediaSource* source;
    virtual void accept(ClipVisitor* visitor, Clip* clip, Render* render, float rel_ts) = 0;
};

struct VideoClip : public MasterClip{
    VideoReader reader;
    VideoClip(MediaSource* source) : reader(source->filepath){
        this->source = source;
    }
    void accept(ClipVisitor* visitor, Clip* clip, Render* render, float rel_ts);

};

struct Clip{
    float tl_time0;
    float tl_time1;
    MasterClip* masterclip;
    std::list<std::unique_ptr<ComponentShader>> shader_components;
    Clip(float t0, float t1){
        this->tl_time0 = t0;
        this->tl_time1 = t1;
    }
    ~Clip(){
        delete masterclip;
    }

    template <typename T>
    T* add_component();
};

template <typename T>
T* Clip::add_component(){
    printf("\n -- add component\n");
    static_assert(std::is_base_of_v<ComponentShader, T>);

    
    std::unique_ptr<T> element = std::make_unique<T>();
    T* ptr = element.get();
    shader_components.push_back(std::move(element));

    printf("\n -- \n");

    return ptr;
}


struct Track{
    int id;
    std::list<std::unique_ptr<Clip>> clips;
};
struct Timeline;

struct Timeline{
    float gui_playhead = 0.0f;
    float playhead_time = 0.0f;
    bool isPlaying = false;
    std::list<Track> tracks_;
    // Clip* add_clip_video(size_t track, const char* filename, float time0, float time1);
    Clip* add_clip(size_t track, float time0, float time1);
    void update(double dt);
    void key_callback(int key, int action);
};

