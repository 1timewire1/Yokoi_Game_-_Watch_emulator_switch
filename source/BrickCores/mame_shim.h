// Minimal stand-ins for the parts of MAME's emu.h that the uPD7810 core
// (MAME src/devices/cpu/upd7810, BSD-3-Clause, Juergen Buchmueller) uses,
// so its opcode functions and tables compile unchanged outside MAME.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>

typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;
typedef int8_t s8; typedef int16_t s16; typedef int32_t s32; typedef int64_t s64;
typedef uint32_t offs_t;

union PAIR {
    struct { u8 l, h, h2, h3; } b;
    struct { u16 l, h; } w;
    struct { s8 l, h, h2, h3; } sb;
    struct { s16 l, h; } sw;
    u32 d;
    s32 sd;
};

template <typename T> constexpr int BIT(T x, int n) { return (x >> n) & 1; }

enum { CLEAR_LINE = 0, ASSERT_LINE = 1 };
enum { INPUT_LINE_NMI = 32 };

inline void logerror(const char*, ...) {}
inline void standard_irq_callback(int, int) {}
[[noreturn]] inline void fatalerror(const char* m, ...) { fprintf(stderr, "%s", m); abort(); }

struct devcb_read8 {
    std::function<u8(offs_t, u8)> f;
    bool isunset() const { return !f; }
    u8 operator()(offs_t o = 0, u8 m = 0xff) { return f ? f(o, m) : 0xff; }
    template <int N> struct arr { devcb_read8 a[N]; devcb_read8& operator[](int i) { return a[i]; } };
    template <int N> using array = arr<N>;
};
struct devcb_write8 {
    std::function<void(u8)> f;
    bool isunset() const { return !f; }
    void operator()(u8 d) { if (f) f(d); }
    void operator()(offs_t, u8 d, u8 = 0xff) { if (f) f(d); }
};
struct devcb_read_line {
    std::function<int()> f;
    bool isunset() const { return !f; }
    int operator()() { return f ? f() : 0; }
};
struct devcb_write_line {
    std::function<void(int)> f;
    bool isunset() const { return !f; }
    void operator()(int s) { if (f) f(s); }
};

// The CPU's 16-bit address space: the machine supplies read/write.
struct upd_bus {
    std::function<u8(offs_t)> read;
    std::function<void(offs_t, u8)> write;
    u8 read_byte(offs_t a) { return read(a & 0xffff); }
    void write_byte(offs_t a, u8 v) { write(a & 0xffff, v); }
};
struct upd_view { void select(int) {} void disable() {} };
