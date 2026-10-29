#include "BrickRom.h"

#include <cstdlib>
#include <cstring>

namespace {

struct Reader {
    const uint8_t* p;
    size_t left;
    bool ok = true;

    bool take(void* out, size_t n) {
        if (!ok || n > left) return ok = false;
        if (out) std::memcpy(out, p, n);
        p += n;
        left -= n;
        return true;
    }
    uint32_t u32() {
        uint8_t b[4] = {};
        take(b, 4);
        return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    }
    uint8_t u8() {
        uint8_t b = 0;
        take(&b, 1);
        return b;
    }
    std::string str(size_t n) {
        std::string s(n, '\0');
        take(n ? &s[0] : nullptr, n);
        return s;
    }
};

}  // namespace

const std::string& BrickRom::Options::str(const std::string& key) const {
    static const std::string kEmpty;
    auto it = values_.find(key);
    return it == values_.end() ? kEmpty : it->second;
}

long BrickRom::Options::num(const std::string& key, long fallback) const {
    auto it = values_.find(key);
    if (it == values_.end() || it->second.empty()) return fallback;
    char* end = nullptr;
    const long v = strtol(it->second.c_str(), &end, 0);
    return (end && *end == '\0') ? v : fallback;
}

std::vector<long> BrickRom::Options::list(const std::string& key) const {
    std::vector<long> out;
    const std::string& s = str(key);
    size_t start = 0;
    while (start < s.size()) {
        size_t comma = s.find(',', start);
        if (comma == std::string::npos) comma = s.size();
        out.push_back(strtol(s.substr(start, comma - start).c_str(), nullptr, 0));
        start = comma + 1;
    }
    return out;
}

std::map<std::string, std::string> BrickRom::Options::with_prefix(const std::string& prefix) const {
    std::map<std::string, std::string> out;
    for (auto it = values_.lower_bound(prefix); it != values_.end() && it->first.compare(0, prefix.size(), prefix) == 0; ++it) {
        out[it->first.substr(prefix.size())] = it->second;
    }
    return out;
}

void BrickRom::Options::parse(const char* text, size_t size) {
    values_.clear();
    size_t start = 0;
    while (start < size) {
        size_t nl = start;
        while (nl < size && text[nl] != '\n') nl++;
        const std::string line(text + start, nl - start);
        const size_t eq = line.find('=');
        if (eq != std::string::npos) values_[line.substr(0, eq)] = line.substr(eq + 1);
        start = nl + 1;
    }
}

const std::vector<uint8_t>* BrickRom::blob(const std::string& name) const {
    auto it = blobs.find(name);
    return it == blobs.end() ? nullptr : &it->second;
}

bool BrickRom::is_container(const uint8_t* data, size_t size) {
    return data && size >= 4 && std::memcmp(data, "BRK1", 4) == 0;
}

bool BrickRom::parse(const uint8_t* data, size_t size) {
    if (!is_container(data, size)) return false;
    Reader r{data + 4, size - 4};
    core = r.str(r.u8());
    clock_hz = r.u32();
    const uint32_t opt_len = r.u32();
    if (!r.ok || opt_len > r.left) return false;
    options.parse((const char*)r.p, opt_len);
    r.take(nullptr, opt_len);
    const uint32_t count = r.u32();
    blobs.clear();
    for (uint32_t i = 0; r.ok && i < count; i++) {
        const std::string name = r.str(r.u8());
        const uint32_t n = r.u32();
        if (!r.ok || n > r.left) return false;
        blobs[name].assign(r.p, r.p + n);
        r.take(nullptr, n);
    }
    return r.ok && !core.empty() && clock_hz > 0 && blob("rom") != nullptr;
}
