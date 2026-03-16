#pragma once
#include <GLFW/glfw3.h>
#include <stdio.h>

class Glfw{
    
    public:
    GLFWwindow* window_;
    Glfw();
    ~Glfw();
};

Glfw::Glfw(){
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    window_ = glfwCreateWindow(1280, 720, "editor", NULL, NULL);
    if (!window_){
        //printf("falha ao criar janela");
        glfwTerminate();
        throw "window não foi criada";
    }   
    glfwMakeContextCurrent(window_);

}

Glfw::~Glfw(){
    glfwDestroyWindow(window_);
    glfwTerminate();
}
