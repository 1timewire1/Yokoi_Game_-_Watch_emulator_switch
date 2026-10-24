#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

// Symmetric save-state reader/writer for the LCDonald ports: each game lists
// its fields once (LCDonald_program::visit_state()) and the same call both
// saves and loads them, so the two directions can't drift apart. Little-endian
// fixed-width values; containers are length-prefixed. Any short read/write
// latches ok() to false, after which every further call is a no-op.
class LCDonaldStateIO {
public:
    LCDonaldStateIO(FILE* file, bool writing) : file_(file), writing_(writing) {}

    bool ok() const { return ok_; }
    bool writing() const { return writing_; }

    void io(int& v) { int32_t x = (int32_t)v; raw(&x, sizeof(x)); v = (int)x; }
    void io(bool& v) { uint8_t x = v ? 1 : 0; raw(&x, sizeof(x)); v = (x != 0); }
    void io(float& v) { raw(&v, sizeof(v)); }
    void io(uint64_t& v) { raw(&v, sizeof(v)); }

    void io(std::string& v) {
        uint32_t n = (uint32_t)v.size();
        raw(&n, sizeof(n));
        if (!ok_ || n > kMaxLen) { ok_ = false; return; }
        if (!writing_) v.resize(n);
        if (n) raw(&v[0], n);
    }

    void io(std::vector<int>& v) {
        uint32_t n = length(v.size());
        if (!writing_) v.assign(n, 0);
        for (uint32_t i = 0; ok_ && i < n; i++) io(v[i]);
    }

    void io(std::vector<std::string>& v) {
        uint32_t n = length(v.size());
        if (!writing_) v.assign(n, std::string());
        for (uint32_t i = 0; ok_ && i < n; i++) io(v[i]);
    }

    void io(std::vector<std::vector<std::string>>& v) {
        uint32_t n = length(v.size());
        if (!writing_) v.assign(n, std::vector<std::string>());
        for (uint32_t i = 0; ok_ && i < n; i++) io(v[i]);
    }

    void io(std::unordered_map<std::string, float>& m) {
        uint32_t n = length(m.size());
        if (writing_) {
            // Sorted by key, so the same contents always produce the same
            // bytes (unordered_map iteration order depends on insert history).
            std::vector<std::string> keys;
            keys.reserve(m.size());
            for (const auto& kv : m) keys.push_back(kv.first);
            std::sort(keys.begin(), keys.end());
            for (std::string& k : keys) {
                float f = m[k];
                io(k);
                io(f);
            }
        } else {
            m.clear();
            for (uint32_t i = 0; ok_ && i < n; i++) {
                std::string k;
                float f = 0.0f;
                io(k);
                io(f);
                if (ok_) m[k] = f;
            }
        }
    }

private:
    static constexpr uint32_t kMaxLen = 1u << 16;  // sanity bound against a corrupt file

    uint32_t length(size_t size) {
        uint32_t n = (uint32_t)size;
        raw(&n, sizeof(n));
        if (n > kMaxLen) ok_ = false;
        return ok_ ? n : 0;
    }

    void raw(void* p, size_t n) {
        if (!ok_) return;
        const size_t done = writing_ ? fwrite(p, 1, n, file_) : fread(p, 1, n, file_);
        if (done != n) ok_ = false;
    }

    FILE* file_;
    bool writing_;
    bool ok_ = true;
};
