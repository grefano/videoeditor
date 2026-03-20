#pragma once
#include <glad.h>
#include <stdio.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iterator>
#include <iostream>
#include "profile.hpp"

void geterr(const char* info="");
void tex_to_image(GLuint tex, uint8_t* buffer, int w, int h);
GLuint create_texture();
void image_to_tex(GLuint tex, uint8_t* buffer, int w, int h);
void RenderQuad();
GLuint createShader(const char* vs, const char* fs);
// GLuint overlap_textures(GLuint tex_below, GLuint tex_above, GLuint shader);
void overlap_textures(const GLuint tex_below, const GLuint tex_above, const GLuint result_tex, const GLuint fbo, const GLuint shader);

std::string readFileToString(const std::string& filename);