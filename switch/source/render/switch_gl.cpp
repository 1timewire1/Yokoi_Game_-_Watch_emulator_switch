#include "switch_gl.h"

#include <cstdint>
#include <cstdio>

#include "std/debug_log.h"

GLuint switch_gl_compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        GLsizei len = 0;
        glGetShaderInfoLog(shader, (GLsizei)sizeof(log), &len, log);
        // printf alone goes nowhere useful here - no console (it'd fight EGL
        // for the NWindow) and no nxlink by default - so this also logs to file.
        YOKOI_LOG("shader compile failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint switch_gl_link_program(GLuint vs, GLuint fs) {
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
        YOKOI_LOG("program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

void switch_gl_init_resources(GlResources& r) {
    const char* vs_src =
        "#version 300 es\n"
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aUv;\n"
        "out vec2 vUv;\n"
        "uniform vec2 uScale;\n"
        "uniform vec2 uOffset;\n"
        "uniform vec2 uRotCosSin;\n" // (cos, sin) of a screen-rotation angle around NDC origin; (1,0) = no rotation.
        "void main() {\n"
        "  vUv = aUv;\n"
        "  vec2 p = aPos.xy * uScale + uOffset;\n"
        "  vec2 rotated = vec2(p.x * uRotCosSin.x - p.y * uRotCosSin.y,\n"
        "                       p.x * uRotCosSin.y + p.y * uRotCosSin.x);\n"
        "  gl_Position = vec4(rotated, 0.0, 1.0);\n"
        "}\n";

    const char* fs_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 vUv;\n"
        "uniform sampler2D uTex;\n"
        "uniform vec4 uMul;\n"
        "uniform float uAlphaOnly;\n"
        "uniform float uMultiplyDarken;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  vec4 t = texture(uTex, vUv);\n"
        "  if (uMultiplyDarken > 0.5) {\n"
        // Multiply-blend mode (paired with glBlendFunc(GL_DST_COLOR,
        // GL_ZERO) at the call site): outside the segment's shape (alpha
        // 0) this must multiply the destination by 1.0 (no-op), and
        // inside it (alpha 1) by uMul's tint - darkening whatever's
        // already drawn there (background colour + label) in place
        // instead of replacing it, matching how MAME's own artwork
        // compositing (and the real LCD ink over a printed backdrop)
        // looks. Always opaque so the multiply blend applies uniformly
        // across the whole quad, not just the "on" pixels.
        "    fragColor = vec4(mix(vec3(1.0), uMul.rgb, t.a), 1.0);\n"
        "  } else if (uAlphaOnly > 0.5) {\n"
        "    float m = max(t.a, max(t.r, max(t.g, t.b)));\n"
        "    fragColor = vec4(uMul.rgb, m * uMul.a);\n"
        "  } else {\n"
        "    fragColor = t * uMul;\n"
        "  }\n"
        "}\n";

    GLuint vs = switch_gl_compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = switch_gl_compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!vs || !fs) {
        YOKOI_LOG("switch_gl_init_resources: shader compile failed (vs=%u fs=%u), aborting init", (unsigned)vs, (unsigned)fs);
        return;
    }

    r.program = switch_gl_link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!r.program) {
        YOKOI_LOG("switch_gl_init_resources: program link failed, aborting init");
        return;
    }

    r.uTex = glGetUniformLocation(r.program, "uTex");
    r.uMul = glGetUniformLocation(r.program, "uMul");
    r.uAlphaOnly = glGetUniformLocation(r.program, "uAlphaOnly");
    r.uMultiplyDarken = glGetUniformLocation(r.program, "uMultiplyDarken");
    r.uScale = glGetUniformLocation(r.program, "uScale");
    r.uOffset = glGetUniformLocation(r.program, "uOffset");
    r.uRotCosSin = glGetUniformLocation(r.program, "uRotCosSin");

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

    glGenTextures(1, &r.tex_white);
    glBindTexture(GL_TEXTURE_2D, r.tex_white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    const uint8_t white_px[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_px);

    const GLenum err = glGetError();
    YOKOI_LOG("switch_gl_init_resources: done, program=%u vao=%u vbo=%u tex_white=%u uTex=%d uMul=%d uAlphaOnly=%d uScale=%d uOffset=%d glGetError=0x%04x",
              (unsigned)r.program, (unsigned)r.vao, (unsigned)r.vbo, (unsigned)r.tex_white,
              r.uTex, r.uMul, r.uAlphaOnly, r.uScale, r.uOffset, (unsigned)err);
}
