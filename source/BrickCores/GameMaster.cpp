#include "GameMaster.h"

#include <cstring>

#include "upd7810.h"

// --- SED1520 -----------------------------------------------------------------

void GameMaster::SED1520::control_write(uint8_t d) {
    if ((d & 0xfe) == 0xae) lcd_on = d & 1;               // display on/off
    else if ((d & 0xe0) == 0xc0) start_line = d & 0x1f;   // start line
    else if ((d & 0xfc) == 0xb8) page = d & 3;            // page address
    else if ((d & 0x80) == 0x00) column = d & 0x7f;       // column address
    else if ((d & 0xfe) == 0xa0) adc = d & 1;
    else if ((d & 0xfe) == 0xa4) static_drive = d & 1;
    else if ((d & 0xfe) == 0xa8) duty = d & 1;
    else if (d == 0xe0) { modify_write = 1; old_column = column; }
    else if (d == 0xee) { modify_write = 0; column = old_column; }
    else if (d == 0xe2) { start_line = column = 0; page = 3; }
}

void GameMaster::SED1520::data_write(uint8_t d) {
    data = d;
    if (column < 80) {  // no RAM write past the last column
        ddr[(page * 80 + column) % 320] = d;
        column++;
    }
}

uint8_t GameMaster::SED1520::data_read() {
    const uint8_t out = data;  // reads return the latch, then refill it
    if (column < 80) {
        data = ddr[(page * 80 + column) % 320];
        if (!modify_write) column++;
    } else {
        data = 0;
    }
    return out;
}

// --- machine -------------------------------------------------------------------

GameMaster::GameMaster() : BrickCore("UPD78C11"), cpu_(new upd7810_device()) {
    upd7810_device& c = *cpu_;
    c.configure_78c11();
    c.m_program.read = [this](offs_t a) { return read(a); };
    c.m_program.write = [this](offs_t a, u8 v) { write(a, v); };
    c.m_io.read = [](offs_t) -> u8 { return 0xff; };
    c.m_io.write = [](offs_t, u8) {};
    c.m_pa_in_cb.f = [this](offs_t, u8) -> u8 { return joy_; };
    c.m_pc_out_cb.f = [this](u8 d) {
        chipsel_ = d & 7;            // d0 RAM, d1/d2 the two LCD drivers
        set_speaker((d >> 4) & 1);   // d4 speaker
    };
}

GameMaster::~GameMaster() = default;

uint8_t GameMaster::read(uint32_t a) {
    if (a < 0x1000) return a < bios_.size() ? bios_[a] : 0xff;   // internal ROM
    if (a >= 0xff00) return iram_[a & 0xff];                      // internal RAM
    if (a >= 0x8000) return cart_.empty() ? 0xff : cart_[(a - 0x8000) % cart_.size()];
    if (a >= 0x4000) {  // 0x4000-0x7fff: RAM and/or the LCD drivers, by chip select
        uint8_t d = 0;
        if (chipsel_ & 1) d |= ram_[a & 0x7ff];
        for (int i = 0; i < 2; i++)
            if ((chipsel_ >> (i + 1)) & 1) d |= (a & 1) ? lcd_[i].data_read() : lcd_[i].status_read();
        return d;
    }
    return 0xff;
}

void GameMaster::write(uint32_t a, uint8_t v) {
    if (a >= 0xff00) { iram_[a & 0xff] = v; return; }
    if (a >= 0x4000 && a < 0x8000) {
        if (chipsel_ & 1) ram_[a & 0x7ff] = v;
        for (int i = 0; i < 2; i++)
            if ((chipsel_ >> (i + 1)) & 1) {
                if (a & 1) lcd_[i].data_write(v);
                else lcd_[i].control_write(v);
            }
    }
}

void GameMaster::render() {
    // LCD 0 the top 32 rows, LCD 1 the bottom 32: column c%80, row (c/80)*8+bit.
    for (int n = 0; n < 2; n++) {
        const SED1520& l = lcd_[n];
        for (int c = 0; c < 320; c++) {
            const int x = c % 80;
            if (x >= kW) continue;
            for (int b = 0; b < 8; b++) {
                const int y = n * 32 + (c / 80) * 8 + b;
                fb_[y][x] = l.lcd_on && !l.static_drive ? ((l.ddr[c] >> b) & 1) : (l.static_drive ? fb_[y][x] : false);
            }
        }
    }
}

void GameMaster::reset() {
    const std::vector<uint8_t>* b = rom_info().blob("rom");
    bios_ = b ? *b : std::vector<uint8_t>();
    const std::vector<uint8_t>* c = rom_info().blob("cart");
    cart_ = c ? *c : std::vector<uint8_t>();
    memset(ram_, 0, sizeof(ram_));
    memset(iram_, 0, sizeof(iram_));
    lcd_[0] = SED1520();
    lcd_[1] = SED1520();
    chipsel_ = 0;
    frame_len_ = clock_hz() / 60 ? clock_hz() / 60 : 1;
    frame_cycles_ = 0;
    memset(fb_, 0, sizeof(fb_));
    cpu_->reset_cpu();
}

int GameMaster::execute() {
    const int cycles = cpu_->run(1);
    frame_cycles_ += (uint32_t)cycles;
    if (frame_cycles_ >= frame_len_) {
        frame_cycles_ -= frame_len_;
        render();
    }
    return cycles > 0 ? cycles : 1;
}

uint8_t GameMaster::display_byte(int index) const {
    if (index < 0 || index >= (kW * kH + 7) / 8) return 0;
    uint8_t v = 0;
    for (int b = 0; b < 8; b++) {
        const int p = index * 8 + b;
        if (p < kW * kH && fb_[p / kW][p % kW]) v |= (uint8_t)(1 << b);
    }
    return v;
}

void GameMaster::port_input(int port, uint8_t mask, int level) {
    if (port != 0) return;
    if (level == 1) joy_ &= (uint8_t)~mask;  // active low
    else joy_ |= mask;
}

bool GameMaster::poke_ram(uint32_t addr, uint8_t value) {
    if (addr >= 0xff00) { iram_[addr & 0xff] = value; return true; }
    if (addr >= 0x4000 && addr < 0x8000) { ram_[addr & 0x7ff] = value; return true; }
    return false;
}

int GameMaster::peek_ram(uint32_t addr) const {
    if (addr >= 0xff00) return iram_[addr & 0xff];
    if (addr >= 0x4000 && addr < 0x8000) return ram_[addr & 0x7ff];
    return -1;
}

void GameMaster::serialize(BrickStateIO& io) {
    upd7810_device& c = *cpu_;
#include "GamePocket_state.inc"
    io.bytes(ram_, sizeof(ram_));
    io.bytes(iram_, sizeof(iram_));
    for (SED1520& l : lcd_) io.bytes(&l, sizeof(l));
    io.pod(chipsel_);
    io.pod(frame_cycles_);
    if (!io.writing()) render();
}
