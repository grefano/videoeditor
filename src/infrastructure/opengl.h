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

GLuint create_texture();
void image_to_tex(GLuint tex, uint8_t* buffer, int w, int h);
void RenderQuad();
GLuint createShader(const char* vs, const char* fs);
// GLuint overlap_textures(GLuint tex_below, GLuint tex_above, GLuint shader);
void overlap_textures(GLuint tex_below, GLuint tex_above, GLuint result_tex, GLuint fbo, GLuint shader);

extern const char* vs;
extern const char* fs;

extern const char* vs_transform;
extern const char* fs_transform;


extern const char* vs_default;
extern const char* fs_default;