#include "shadercomponent.hpp"

std::string fs_source = readFileToString("shaders/overlay.frag.glsl");
const char* fs = fs_source.c_str();
std::string vs_source = readFileToString("shaders/overlay.vertex.glsl");
const char* vs = vs_source.c_str();


std::string fs_transform_source = readFileToString("shaders/transform.frag.glsl");
const char* fs_transform = fs_transform_source.c_str();
std::string vs_transform_source = readFileToString("shaders/transform.vertex.glsl");
const char* vs_transform = vs_transform_source.c_str();



std::string fs_default_source = readFileToString("shaders/default.frag.glsl");
const char* fs_default = fs_default_source.c_str();
std::string vs_default_source = readFileToString("shaders/default.vertex.glsl");
const char* vs_default = vs_default_source.c_str();

void ComponentShader::bind_shader(const char* vs, const char* fs){
    this->shader = createShader(vs, fs);
}

void ComponentShader::get_tex(GLuint tex, GLuint result_tex, GLuint fbo){
    int w, h;
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

    // glGenTextures(1, &this->tex);
    glBindTexture(GL_TEXTURE_2D, result_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        result_tex,
        0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("FBO ERROR\n");
    }

    glViewport(0,0,w,h);
    
    assert(w > 0 && h > 0);
    glUseProgram(this->shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(shader,"tex"),0);

    glClear(GL_COLOR_BUFFER_BIT);

    this->set_uniform(shader);

    RenderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}
void Default::set_uniform(GLuint shader){
    
}

void Transform::set_uniform(GLuint shader){
    glUniform2f(glGetUniformLocation(this->shader, "offset"), position.x, position.y);
    glUniform2f(glGetUniformLocation(this->shader, "scale"), scale.x, scale.y);
    
}


void Overlap::set_uniform(GLuint shader){
    glUniform2f(glGetUniformLocation(shader, "offset"), position.x, position.y);
    glUniform2f(glGetUniformLocation(shader, "scale"), scale.x, scale.y);
    
}

