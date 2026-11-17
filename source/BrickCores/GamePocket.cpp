#include "GamePocket.h"

#include <cstring>

#include "upd7810.h"

// --- HD44102 (MAME src/devices/video/hd44102.cpp) --------------------------

void GamePocket::HD44102::control_w(uint8_t data) {
    switch (data) {
        case 0x38: status |= 0x20; break;   // display off
        case 0x39: status &= ~0x20; break;  // display on
        case 0x3a: status &= ~0x40; break;  // count down
        case 0x3b: status |= 0x40; break;   // count up
        default: {
            const int nx = (data & 0xc0) >> 6, ny = data & 0x3f;
            if (ny == 0x3e) page = (uint8_t)nx;  // display start page
            else if (ny <= 49) { x = nx; y = ny; }
            break;
        }
    }
}

void GamePocket::HD44102::count() {
    if (status & 0x40) { if (++y > 49) y = 0; }
    else { if (--y < 0) y = 49; }
}

void GamePocket::HD44102::data_w(uint8_t data) {
    ram[x][y] = data;
    count();
}

bool GamePocket::HD44102::dot(int sx, int sy) const {
    // MAME's render(): row z of the buffer (z = page*8 + row, wrapping)
    // shows RAM bit z of column sx - written to buffer row z, not row y.
    if (status & 0x20) return false;
    const int z = sy;  // the buffer row asked for
    return (ram[z >> 3][sx] >> (z & 7)) & 1;
}

// --- machine (MAME src/mame/epoch/gamepock.cpp) -----------------------------

GamePocket::GamePocket() : BrickCore("UPD78C06"), cpu_(new upd7810_device()) {
    upd7810_device& c = *cpu_;
    c.m_program.read = [this](offs_t a) { return read(a); };
    c.m_program.write = [this](offs_t a, u8 v) { write(a, v); };
    c.m_io.read = [](offs_t) -> u8 { return 0xff; };
    c.m_io.write = [](offs_t, u8) {};
    c.m_pa_out_cb.f = [this](u8 d) { control_w(d); };
    c.m_pb_out_cb.f = [this](u8 d) { lcd_data_ = d; };
    c.m_pc_in_cb.f = [this](offs_t, u8) -> u8 { return input_r(); };
    c.m_to_func.f = [this](int state) { set_speaker(state != 0); };
}

GamePocket::~GamePocket() = default;

uint8_t GamePocket::read(uint32_t a) const {
    if (a < 0x1000) return a < bios_.size() ? bios_[a] : 0xff;  // internal ROM
    if (a >= 0xff80) return iram_[a & 0x7f];                    // internal RAM
    if (a >= 0x4000 && a < 0xc000) {
        if (cart_.empty()) return 0xff;
        return cart_[(a - 0x4000) % cart_.size()];
    }
    if (a >= 0xc000 && a < 0xd000) return ram_[a & 0x7ff];      // 2 KB, mirrored
    return 0xff;                                                // unmapped: high
}

void GamePocket::write(uint32_t a, uint8_t v) {
    if (a >= 0xff80) iram_[a & 0x7f] = v;
    else if (a >= 0xc000 && a < 0xd000) ram_[a & 0x7ff] = v;
}

void GamePocket::control_w(uint8_t data) {
    // 76------ input select, --543--- LCD CS, -----2-- LCD D/I, ------1- LCD M
    // Written to the LCD on the falling edge of M.
    if (~data & control_ & 2) {
        for (int i = 0; i < 3; i++) {
            if ((data >> (i + 3)) & 1) {
                if ((data >> 2) & 1) lcd_[i].data_w(lcd_data_);
                else lcd_[i].control_w(lcd_data_);
            }
        }
    }
    control_ = data;
}

uint8_t GamePocket::input_r() const {
    uint8_t data = 0xff;
    for (int i = 0; i < 2; i++) {
        if ((control_ >> (i ^ 7)) & 1) data &= inputs_[i];
    }
    return data;
}

void GamePocket::render() {
    for (int i = 0; i < 3; i++) {
        for (int sx = 0; sx < 50; sx++) {
            for (int sy = 0; sy < 32; sy++) {
                int dx = sx, dy = sy;
                switch (i) {
                    case 0: dx = 49 - sx; break;                          // top-left, reversed
                    case 1: dy += 32; break;                              // bottom-left
                    default: dx = (sx % 25) + 50; dy += (sx / 25) * 32;   // right half
                }
                if (dx < kW && dy < kH) fb_[dy][dx] = lcd_[i].dot(sx, sy);
            }
        }
    }
}

void GamePocket::reset() {
    const std::vector<uint8_t>* b = rom_info().blob("rom");
    bios_ = b ? *b : std::vector<uint8_t>();
    const std::vector<uint8_t>* c = rom_info().blob("cart");
    cart_ = c ? *c : std::vector<uint8_t>();
    memset(ram_, 0, sizeof(ram_));
    memset(iram_, 0, sizeof(iram_));
    for (HD44102& l : lcd_) l = HD44102();
    control_ = lcd_data_ = 0;
    frame_len_ = clock_hz() / 60 ? clock_hz() / 60 : 1;
    frame_cycles_ = 0;
    memset(fb_, 0, sizeof(fb_));
    cpu_->reset_cpu();
}

int GamePocket::execute() {
    const int cycles = cpu_->run(1);
    frame_cycles_ += (uint32_t)cycles;
    if (frame_cycles_ >= frame_len_) {
        frame_cycles_ -= frame_len_;
        render();
    }
    return cycles > 0 ? cycles : 1;
}

uint8_t GamePocket::display_byte(int index) const {
    // Row-major bits, for the base class's helpers (test pattern, ...).
    if (index < 0 || index >= (kW * kH + 7) / 8) return 0;
    uint8_t v = 0;
    for (int b = 0; b < 8; b++) {
        const int p = index * 8 + b;
        if (p < kW * kH && fb_[p / kW][p % kW]) v |= (uint8_t)(1 << b);
    }
    return v;
}

void GamePocket::port_input(int port, uint8_t mask, int level) {
    if (port < 0 || port > 1) return;
    if (level == 1) inputs_[port] &= (uint8_t)~mask;  // active low
    else inputs_[port] |= mask;
}

bool GamePocket::poke_ram(uint32_t addr, uint8_t value) {
    if (addr >= 0xff80) { iram_[addr & 0x7f] = value; return true; }
    if (addr >= 0xc000 && addr < 0xd000) { ram_[addr & 0x7ff] = value; return true; }
    return false;
}

int GamePocket::peek_ram(uint32_t addr) const {
    if (addr >= 0xff80) return iram_[addr & 0x7f];
    if (addr >= 0xc000 && addr < 0xd000) return ram_[addr & 0x7ff];
    return -1;
}

void GamePocket::serialize(BrickStateIO& io) {
    upd7810_device& c = *cpu_;
#include "GamePocket_state.inc"
    io.bytes(ram_, sizeof(ram_));
    io.bytes(iram_, sizeof(iram_));
    for (HD44102& l : lcd_) {
        io.bytes(l.ram, sizeof(l.ram));
        io.pod(l.status);
        io.pod(l.output);
        io.pod(l.page);
        io.pod(l.x);
        io.pod(l.y);
    }
    io.pod(control_);
    io.pod(lcd_data_);
    io.pod(frame_cycles_);
    if (!io.writing()) render();
}
