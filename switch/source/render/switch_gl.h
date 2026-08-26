#pragma once

// Trimmed port of android/app/src/main/cpp/render/yokoi_gl.h: same shader
// and vertex layout, minus tex_ui (no in-app menu UI yet) and minus the
// per-EGL-context bookkeeping (Switch only ever has one GL context).

// Must be glad, not <GLES3/gl3.h> directly: there's no linkable libGLESv3 on
// Switch homebrew (unlike Android), gl* symbols are resolved at runtime via
// eglGetProcAddress through glad's loaded function-pointer table.
#include <glad/glad.h>

#include <cstddef>

struct RenderVertex {
    float x;
    float y;
    float u;
    float v;
};

struct GlResources {
    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    size_t vbo_capacity_bytes = 0;
    GLint uTex = -1;
    GLint uMul = -1;
    GLint uAlphaOnly = -1;
    GLint uScale = -1;
    GLint uOffset = -1;
    GLint uRotCosSin = -1;
    GLuint tex_white = 0;

    GLuint tex_segments = 0;
    GLuint tex_background = 0;
    GLuint tex_console = 0;
    int tex_segments_w = 0;
    int tex_segments_h = 0;
    int tex_background_w = 0;
    int tex_background_h = 0;
    int tex_console_w = 0;
    int tex_console_h = 0;

    int viewport_w = 0;
    int viewport_h = 0;
};

GLuint switch_gl_compile_shader(GLenum type, const char* src);
GLuint switch_gl_link_program(GLuint vs, GLuint fs);

// Initializes shader program + VAO/VBO + fallback white texture. Call once,
// after the EGL context is current.
void switch_gl_init_resources(GlResources& r);
