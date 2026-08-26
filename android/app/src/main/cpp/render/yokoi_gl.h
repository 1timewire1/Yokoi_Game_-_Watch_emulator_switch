#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <cstddef>

struct RenderVertex {
    float x;
    float y;
    float u;
    float v;
};

struct GlResources {
    EGLContext ctx = EGL_NO_CONTEXT;
    int width = 0;
    int height = 0;

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    size_t vbo_capacity_bytes = 0;
    GLint uTex = -1;
    GLint uMul = -1;
    GLint uAlphaOnly = -1;
    GLint uScale = -1;
    GLint uOffset = -1;
    GLuint tex_white = 0;

    GLuint tex_segments = 0;
    GLuint tex_background = 0;
    GLuint tex_console = 0;
    GLuint tex_ui = 0;
    int tex_segments_w = 0;
    int tex_segments_h = 0;
    int tex_background_w = 0;
    int tex_background_h = 0;
    int tex_console_w = 0;
    int tex_console_h = 0;
    int tex_ui_w = 0;
    int tex_ui_h = 0;
};

GLuint yokoi_gl_compile_shader(GLenum type, const char* src);
GLuint yokoi_gl_link_program(GLuint vs, GLuint fs);

// Initializes GPU resources for a given GL context.
// Safe to call on a freshly-created GlResources (ctx should already be set).
void yokoi_gl_init_resources(GlResources& r);
