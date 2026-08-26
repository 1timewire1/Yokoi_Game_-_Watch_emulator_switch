#include "yokoi_gl.h"

#include <android/log.h>

#include <cstdint>

namespace {
constexpr const char* kLogTag = "Yokoi";
}

GLuint yokoi_gl_compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        GLsizei len = 0;
        glGetShaderInfoLog(shader, (GLsizei)sizeof(log), &len, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "shader compile failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint yokoi_gl_link_program(GLuint vs, GLuint fs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        GLsizei len = 0;
        glGetProgramInfoLog(program, (GLsizei)sizeof(log), &len, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void yokoi_gl_init_resources(GlResources& r) {
    // If the GL context was lost (rotation/background), object IDs from the old context
    // are not valid anymore. Recreate everything for this context.
    r.program = 0;
    r.vao = 0;
    r.vbo = 0;
    r.vbo_capacity_bytes = 0;
    r.uTex = -1;
    r.uMul = -1;
    r.uAlphaOnly = -1;
    r.uScale = -1;
    r.uOffset = -1;
    r.tex_white = 0;

    r.tex_segments = 0;
    r.tex_background = 0;
    r.tex_console = 0;
    r.tex_segments_w = r.tex_segments_h = 0;
    r.tex_background_w = r.tex_background_h = 0;
    r.tex_console_w = r.tex_console_h = 0;

    const char* vs_src =
        "#version 300 es\n"
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aUv;\n"
        "out vec2 vUv;\n"
        "uniform vec2 uScale;\n"
        "uniform vec2 uOffset;\n"
        "void main() {\n"
        "  vUv = aUv;\n"
        "  gl_Position = vec4(aPos.xy * uScale + uOffset, 0.0, 1.0);\n"
        "}\n";

    const char* fs_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 vUv;\n"
        "uniform sampler2D uTex;\n"
        "uniform vec4 uMul;\n"
        "uniform float uAlphaOnly;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  vec4 t = texture(uTex, vUv);\n"
        "  if (uAlphaOnly > 0.5) {\n"
        "    float m = max(t.a, max(t.r, max(t.g, t.b)));\n"
        "    fragColor = vec4(uMul.rgb, m * uMul.a);\n"
        "  } else {\n"
        "    fragColor = t * uMul;\n"
        "  }\n"
        "}\n";

    GLuint vs = yokoi_gl_compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = yokoi_gl_compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!vs || !fs) {
        return;
    }

    r.program = yokoi_gl_link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!r.program) {
        return;
    }

    r.uTex = glGetUniformLocation(r.program, "uTex");
    r.uMul = glGetUniformLocation(r.program, "uMul");
    r.uAlphaOnly = glGetUniformLocation(r.program, "uAlphaOnly");
    r.uScale = glGetUniformLocation(r.program, "uScale");
    r.uOffset = glGetUniformLocation(r.program, "uOffset");

    glGenVertexArrays(1, &r.vao);
    glGenBuffers(1, &r.vbo);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)(sizeof(float) * 2));
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Fallback 1x1 white texture.
    glGenTextures(1, &r.tex_white);
    glBindTexture(GL_TEXTURE_2D, r.tex_white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    const uint8_t white_px[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_px);
}
