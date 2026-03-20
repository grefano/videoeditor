#pragma once

#include "opengl.h"
#include <list>
#include <stdio.h>
#include <memory>
#include <imgui.h>


extern const char* vs;
extern const char* fs;

extern const char* vs_transform;
extern const char* fs_transform;


extern const char* vs_default;
extern const char* fs_default;
struct ComponentShader{
    GLuint shader = 0;
    void bind_shader(const char* vs, const char* fs);
    void get_tex(GLuint tex, GLuint result_tex, GLuint fbo);
    virtual void set_uniform(GLuint shader) = 0;

    ComponentShader(const char* vs, const char* fs){
        bind_shader(vs, fs);
    }
    ~ComponentShader(){
        glDeleteProgram(this->shader);
    }
};
struct Default : public ComponentShader{
    Default() : ComponentShader(vs_default, fs_default){};
    void set_uniform(GLuint shader) override;
};
struct Transform : public ComponentShader{
    ImVec2 position;
    ImVec2 scale;
    Transform() : ComponentShader(vs_transform, fs_transform){};
    void set_uniform(GLuint shader) override;
};

struct Overlap : public ComponentShader{
    ImVec2 position;
    ImVec2 scale;
    Overlap() : ComponentShader(vs, fs){};
    void set_uniform(GLuint shader) override;


};
