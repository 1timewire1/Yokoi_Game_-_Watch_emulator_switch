#include "Tamago.h"

#include <cstring>
#include <ctime>

#include "tamago_icons.h"

namespace {
// DRAM offset of each LCD row, top to bottom (the port's main.c lcd_order).
constexpr uint16_t kLcdRow[Tamago::kLcdH] = {
    0x0C0, 0x0CC, 0x0D8, 0x0E4, 0x0F0, 0x0FC, 0x108, 0x114,
    0x120, 0x12C, 0x138, 0x144, 0x150, 0x15C, 0x168, 0x174,
    0x0B4, 0x0A8, 0x09C, 0x090, 0x084, 0x078, 0x06C, 0x060,
    0x054, 0x048, 0x03C, 0x030, 0x024, 0x018, 0x00C};
constexpr uint8_t kOffInk = 14;   // an unlit dot still shows a little, as on the real LCD
}

Tamago::Tamago() : BrickCore("TAMAGO") {}

Tamago::~Tamago() {
    if (peer_) (follower_ ? peer_ : this)->link_unpair();
    if (s_heard_ == this) s_heard_ = nullptr;
    tamago_machine_release(&m_);
}

void Tamago::reset() {
    const std::vector<uint8_t>* b = rom_info().blob("rom");
    bios_ = b ? *b : std::vector<uint8_t>();
    figs_.clear();
    for (int i = 1; i <= 99; i++) {
        char name[8];
        snprintf(name, sizeof(name), "fig%02d", i);
        const std::vector<uint8_t>* f = rom_info().blob(name);
        if (!f) break;
        figs_.push_back(f);
    }
    figure_count_ = (int)figs_.size();
    memset(&m_, 0, sizeof(m_));
    tamago_machine_init(&m_, bios_.data(), bios_.size());
    tamago_machine_set_primary(&m_);   // its sound is the one rendered
    s_heard_ = this;
    friends_ = tamago_machine_is_friends_of(&m_);
    figure_ = 0;
    keys_ = 0x0F;
    frame_cycle_ = 0;
    memset(pcm_, 0, sizeof(pcm_));
    sound_level_ = 0.0f;
    ink_valid_ = false;
    tamago_machine_frame(&m_);
}

void Tamago::set_figure(int n, uint8_t contacts) {
    if (friends_ || n < 0 || n > figure_count_) n = 0;
    contacts &= 3;
    if (n && !contacts) n = 0;   // no pin touching: nothing inserted
    if (n == figure_ && (n == 0 || contacts == contacts_)) return;
    figure_ = n;
    contacts_ = contacts;
    if (n == 0) tamago_machine_set_figure(&m_, 0, nullptr, 0);
    else tamago_machine_set_figure(&m_, contacts, figs_[n - 1]->data(), figs_[n - 1]->size());
}

Tamago* Tamago::s_heard_ = nullptr;

void Tamago::end_frame() {
    // The sound of the frame that just ran, played back during the next one -
    // only the primary console has any (tamago_machine.c); heard this frame,
    // this one takes over from the next.
    if (s_heard_ == this) tamago_audio_render(pcm_, kSamples, kFrame);
    else memset(pcm_, 0, sizeof(pcm_));
    if (heard_ && s_heard_ != this) {
        s_heard_ = this;
        tamago_machine_set_primary(&m_);
    }
    heard_ = false;
    frame_cycle_ = 0;
    tamago_machine_frame(&m_);
    // The game asks for the time with hour 255 (a cold boot, a reset): after
    // a second of asking it gets the console's, as the port's gameclock.c.
    if (m_.wram[0x306] == 255) {
        if (++ask_frames_ == 60) {
            const time_t now = time(nullptr);
            const tm* t = localtime(&now);
            if (t) {
                set_date(t->tm_mon + 1, t->tm_mday);
                m_.wram[0x307] = (uint8_t)t->tm_min;
                m_.wram[friends_ ? 0x136 : 0x396] = (uint8_t)t->tm_sec;
                m_.wram[0x306] = (uint8_t)t->tm_hour;
            }
        }
    } else {
        ask_frames_ = 0;
    }
}

int Tamago::step_one() {
    if (frame_cycle_ >= kFrame) end_frame();
    m_.keys = keys_;
    int cycles = 1;
    if (!m_.cpu.halted && tamago_machine_step(&m_) == 0) cycles = (int)m_.last_cycles;
    if (cycles < 1) cycles = 1;
    frame_cycle_ += (uint32_t)cycles;
    clk_ += (uint64_t)cycles;
    const uint32_t i = frame_cycle_ * (uint32_t)kSamples / kFrame;
    sound_level_ = (float)pcm_[2 * (i < (uint32_t)kSamples ? i : kSamples - 1)] / 65536.0f;
    return cycles;
}

int Tamago::execute() {
    // Linked, the other console runs this one: only time passes here.
    if (follower_) return (int)(TAMAGO_CLOCK_HZ / kTickHz);
    const int cycles = step_one();
    if (peer_)
        while (peer_->clk_ < clk_) peer_->step_one();
    return cycles;
}

bool Tamago::link_pair(BrickCore* other) {
    if (!other || other->rom_info().core != "TAMAGO" || other == this) return false;
    Tamago* o = static_cast<Tamago*>(other);
    if (o->friends_ != friends_) return false;   // a Tama-Go and a Friends don't talk
    link_unpair();
    peer_ = o;
    o->peer_ = this;
    o->follower_ = true;
    o->clk_ = clk_;
    tamago_link_connect(&m_, &o->m_);
    tamago_machine_set_primary(&m_);   // the sound is this one's, until the other is played
    s_heard_ = this;
    return true;
}

void Tamago::link_unpair() {
    if (!peer_) return;
    tamago_link_disconnect(&m_);
    tamago_link_disconnect(&peer_->m_);
    peer_->peer_ = nullptr;
    peer_->follower_ = false;
    follower_ = false;
    peer_ = nullptr;
    tamago_machine_set_primary(&m_);
    s_heard_ = this;
}

void Tamago::port_input(int port, uint8_t mask, int level) {
    if (port != 0) return;
    if (level == 1) keys_ &= (uint8_t)~mask;
    else keys_ |= mask;
    keys_ &= 0x0F;
}

const uint8_t* Tamago::ink() {
    if (ink_valid_ && memcmp(ink_dram_, m_.dram, sizeof(ink_dram_)) == 0) return &ink_[0][0];
    memcpy(ink_dram_, m_.dram, sizeof(ink_dram_));
    ink_valid_ = true;
    memset(ink_, 0, sizeof(ink_));
    // Ten icons, five above and five below the matrix; two bits each in DRAM 4.. (the port's draw_icons).
    const uint8_t (*icons)[kIcon * kIcon] = friends_ ? kTamagoIconsFriends : kTamagoIconsGo;
    unsigned a = 4;
    int b = 0;   // the port's draw_icons: icon 0 is bits 0-1 of DRAM 4, then 6-7, 4-5... of the next bytes
    for (int n = 0; n < 10; n++) {
        const unsigned level = (m_.dram[a] >> b) & 3;
        if ((b -= 2) < 0) { b = 6; a++; }
        const unsigned shade = level ? level * 85 : kOffInk;
        const int cell = kPicW / 5;
        const int x0 = (n % 5) * cell + (cell - kIcon) / 2;
        const int y0 = n < 5 ? (kIconRow - kIcon) / 2 : kIconRow + kLcdH * kDot + (kIconRow - kIcon) / 2;
        for (int y = 0; y < kIcon; y++)
            for (int x = 0; x < kIcon; x++)
                ink_[y0 + y][x0 + x] = (uint8_t)(icons[n][y * kIcon + x] * shade / 255);
    }
    // The dot matrix: each dot kDot x kDot, a one-pixel gap to its right and below.
    for (int y = 0; y < kLcdH; y++) {
        const uint8_t* row = m_.dram + kLcdRow[y];
        for (int x = 0; x < kLcdW; x++) {
            const unsigned s = (row[x >> 2] >> (6 - (x & 3) * 2)) & 3u;
            const uint8_t v = s ? (uint8_t)(s * 85) : kOffInk;
            for (int dy = 0; dy < kDot - 1; dy++)
                memset(&ink_[kIconRow + y * kDot + dy][x * kDot], v, kDot - 1);
        }
    }
    return &ink_[0][0];
}

void Tamago::serialize(BrickStateIO& io) {
    std::vector<uint8_t> st(tamago_machine_state_size());
    if (io.writing()) tamago_machine_serialize(&m_, st.data());
    io.vec(st);
    if (!io.writing() && io.ok()) {
        if (!tamago_machine_deserialize(&m_, st.data(), st.size())) return;
        ink_valid_ = false;
        const int f = figure_;
        figure_ = -1;
        set_figure(f, contacts_);   // the figure isn't part of the machine's state
    }
    io.pod(keys_);
    io.pod(frame_cycle_);
}

// Work RAM ($0000-$05FF, mirrored up to $0FFF): the game's clock lives there
// (month $304, day $305, hour $306, minute $307, second $396 / Friends $136;
// the build script's clock.* options), as the port's gameclock.c.
bool Tamago::poke_ram(uint32_t addr, uint8_t v) {
    if (addr >= sizeof(m_.wram)) return false;
    m_.wram[addr] = v;
    return true;
}

int Tamago::peek_ram(uint32_t addr) const {
    return addr < sizeof(m_.wram) ? m_.wram[addr] : -1;
}

void Tamago::set_date(int month, int day) {
    poke_ram(0x304, (uint8_t)month);
    poke_ram(0x305, (uint8_t)day);
}

void Tamago::fast_forward(uint64_t ticks) {
    ff_ticks_ += ticks;
    const uint64_t half = kTickHz / 2;
    while (ff_ticks_ >= half) {
        tamago_machine_tick(&m_, kTickCycles);
        ff_ticks_ -= half;
    }
    frame_cycle_ = 0;
    ink_valid_ = false;
}
