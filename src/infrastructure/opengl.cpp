#include "opengl.h"
GLuint create_texture(){
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    return tex;
}
  void geterr(const char* info){
    GLenum err = glGetError();
    // logi();
    if (err){
      printf("gl error %d em %s\n", err, info);
      throw 0;
    }
  }
void image_to_tex(GLuint tex, uint8_t* buffer, int w, int h){
  PROFILE_FUNCTION();

    // glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

}
void tex_to_image(GLuint tex, uint8_t* buffer, int w, int h){
    printf("--tex to image\n");
    glBindTexture(GL_TEXTURE_2D, tex);
    geterr("bind tex");
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    geterr("read pixels");
}
void RenderQuad()
{
    //printf("render quad\n");
    static GLuint quadVAO = 0;
    static GLuint quadVBO;

    if (quadVAO == 0)
    {
        float quadVertices[] = {

            // pos   // tex
            -1.0f,  1.0f, 0.0f,1.0f,
            -1.0f, -1.0f, 0.0f,0.0f,
             1.0f, -1.0f, 1.0f,0.0f,

            -1.0f,  1.0f, 0.0f,1.0f,
             1.0f, -1.0f, 1.0f,0.0f,
             1.0f,  1.0f, 1.0f,1.0f
        };

        glGenVertexArrays(1,&quadVAO);
        glGenBuffers(1,&quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER,quadVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(quadVertices),quadVertices,GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}
GLuint createShader(const char* vs, const char* fs)
{
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex,1,&vs,nullptr);
    glCompileShader(vertex);
    //printf("awdawd\n");
    GLint success;
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        char info[512];
        glGetShaderInfoLog(vertex,512,NULL,info);
        printf("error %s\n", info);
    }
    //printf("create fragment\n");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment,1,&fs,nullptr);
    glCompileShader(fragment);

    //printf("program\n");
    GLuint program = glCreateProgram();
    glAttachShader(program,vertex);
    glAttachShader(program,fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

void overlap_textures(const GLuint tex_below, const GLuint tex_above, const GLuint result_tex, const GLuint fbo, const GLuint shader)
{   
    printf("overlap\n");
    // static GLuint fbo = 0;


    // static GLuint resultTexture;
    // glGenTextures(1, &resultTexture);
    glBindTexture(GL_TEXTURE_2D, result_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 360, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        result_tex,
        0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("FBO ERROR\n");
    }

    glViewport(0,0,640,360);

    glUseProgram(shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_below);
    glUniform1i(glGetUniformLocation(shader,"tex1"),0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_above);
    glUniform1i(glGetUniformLocation(shader,"tex2"),1);

    glClear(GL_COLOR_BUFFER_BIT);

    glUniform2f(glGetUniformLocation(shader, "pos"),
        0.25f + sin(glfwGetTime())*.2,
        0.25f);

    glUniform2f(glGetUniformLocation(shader, "scale"),0.5f,0.5f);

    RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER,0);

}


std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        throw false;
    }
    // Read the entire file content into the string using iterators
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;

}

