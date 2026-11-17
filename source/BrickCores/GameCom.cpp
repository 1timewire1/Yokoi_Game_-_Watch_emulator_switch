#include "GameCom.h"

#include <cstring>

#include "sm8500.h"

// SM8521 register addresses used here (MAME gamecom.h).
enum : uint16_t {
    R_IE0 = 0x10, R_IE1 = 0x11, R_P0 = 0x14, R_P1 = 0x15, R_P2 = 0x16, R_P3 = 0x17, R_PS0 = 0x1E, R_PS1 = 0x1F,
    R_MMU0 = 0x24, R_MMU1, R_MMU2, R_MMU3, R_MMU4,
    R_LCDC = 0x30, R_LCH = 0x31, R_LCV = 0x32,
    R_DMC = 0x34, R_DMX1, R_DMY1, R_DMDX, R_DMDY, R_DMX2, R_DMY2, R_DMPL, R_DMBR, R_DMVP,
    R_SGC = 0x40, R_SG0L = 0x42, R_SG1L = 0x44, R_SG0TH = 0x46, R_SG0TL, R_SG1TH, R_SG1TL, R_SG2L, R_SG2TH = 0x4C, R_SG2TL,
    R_SGDA = 0x4E, R_TM0C = 0x50, R_TM0D, R_TM1C, R_TM1D, R_CLKT, R_SG0W0 = 0x60, R_SG1W0 = 0x70,
};

static const int kTimerLimit[8] = {2, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
static constexpr uint32_t kCartFlag = 0x80000000u;

GameCom::GameCom() : BrickCore("SM8521"), cpu_(new sm8500_cpu()) {
    cpu_->read = [this](uint16_t a) { return read(a); };
    cpu_->write = [this](uint16_t a, uint8_t v) { write(a, v); };
    cpu_->dma_cb = [this](int) { handle_dma(); };
    cpu_->timer_cb = [this](int c) { update_timers(c); };
}

GameCom::~GameCom() = default;

const uint8_t* GameCom::bank_ptr(uint32_t off) const {
    if (off & kCartFlag) {
        off &= ~kCartFlag;
        return off < cart_.size() ? &cart_[off] : nullptr;
    }
    return off < kernel_.size() ? &kernel_[off] : nullptr;
}

uint8_t GameCom::read(uint16_t a) {
    if (a < 0x400) return ram_[a];
    if (a < 0x1000) return 0;
    if (a < 0x2000) return (a - 0x1000) < irom_.size() ? irom_[a - 0x1000] : 0xff;
    if (a < 0xA000) {
        const int b = (a >> 13);  // 1..4
        const uint32_t off = bank_[b];
        const uint8_t* p = bank_ptr((off & ~kCartFlag) + (a & 0x1fff) | (off & kCartFlag));
        return p ? *p : 0xff;
    }
    if (a < 0xE000) return 0;  // VRAM is write only
    return nvram_[a - 0xE000];
}

void GameCom::write(uint16_t a, uint8_t v) {
    if (a < 0x400) {
        if (a >= 0x14 && a <= 0x17) { pio_w(a, v); return; }
        if (a >= 0x20 && a <= 0x7f) { internal_w(a, v); return; }
        ram_[a] = v;
        return;
    }
    if (a >= 0xA000 && a < 0xE000) { vram_[a - 0xA000] = v; return; }
    if (a >= 0xE000) nvram_[a - 0xE000] = v;
}

void GameCom::set_mmu(int mmu, uint8_t data) {
    if (data < 0x20) bank_[mmu] = (uint32_t)data << 13;
    else if (cart_sel_ && !cart_.empty()) bank_[mmu] = ((uint32_t)data << 13) | kCartFlag;
}

void GameCom::handle_stylus_press(int column) {
    uint16_t data = 0;
    if (stylus_x_ >= 0 && stylus_y_ >= 0) {
        const int col = stylus_x_ >= 192 ? 12 : stylus_x_ / 16;
        if (col == column) data = (uint16_t)(1 << (stylus_y_ / 16));
    }
    if (data) {
        const uint16_t sy = data ^ 0x3ff;
        ram_[R_P0] = (uint8_t)sy;
        ram_[R_P1] = (uint8_t)((ram_[R_P1] & 0xFC) | (sy >> 8));
    } else {
        ram_[R_P0] = 0xFF;
        ram_[R_P1] |= 3;
    }
}

void GameCom::handle_input_press(uint16_t mux) {
    static const uint16_t kCols[13] = {0xFFFB, 0xFFF7, 0xFFEF, 0xFFDF, 0xFFBF, 0xFF7F, 0xFEFF,
                                       0xFDFF, 0xFBFF, 0xF7FF, 0xEFFF, 0xDFFF, 0xBFFF};
    for (int c = 0; c < 13; c++)
        if (mux == kCols[c]) { handle_stylus_press(c); return; }
    if (mux == 0x7FFF) {
        ram_[R_P0] = in_[0];
        ram_[R_P1] = (uint8_t)((ram_[R_P1] & 0xFC) | (in_[1] & 3));
    } else if (mux == 0xFFFF) {
        uint8_t in2 = in_[2];
        if (stylus_x_ >= 0) in2 &= (uint8_t)~0x04;  // stylus press
        ram_[R_P0] = (uint8_t)((ram_[R_P0] & 0xFC) | (in2 & 3));
        ram_[R_P1] = 0xFF;
    }
}

void GameCom::pio_w(uint16_t off, uint8_t data) {
    ram_[off] = data;
    if (off == R_P1 || off == R_P2) handle_input_press((uint16_t)(ram_[R_P1] | (ram_[R_P2] << 8)));
    else if (off == R_P3) cart_sel_ = (data & 0xc0) == 0x40;
}

void GameCom::internal_w(uint16_t off, uint8_t data) {
    switch (off) {
    case R_MMU1: case R_MMU2: case R_MMU3: case R_MMU4: set_mmu(off - R_MMU0, data); break;
    case R_DMBR: data &= 0x7f; break;
    case R_TM0D: timer_[0].upcounter_max = data; data = 0; break;
    case R_TM0C:
        timer_[0].enabled = (data >> 7) & 1;
        timer_[0].prescale_max = (uint32_t)(kTimerLimit[data & 7] >> 1);
        timer_[0].prescale_count = 0;
        ram_[R_TM0D] = 0;
        break;
    case R_TM1D: timer_[1].upcounter_max = data; data = 0; break;
    case R_TM1C:
        timer_[1].enabled = (data >> 7) & 1;
        timer_[1].prescale_max = (uint32_t)(kTimerLimit[data & 7] >> 1);
        timer_[1].prescale_count = 0;
        ram_[R_TM1D] = 0;
        break;
    case R_CLKT:
        if (data & 0x80) {
            clock_period_ = clock_hz() * ((data & 0x40) ? 60u : 1u);
            clock_acc_ = 0;
        } else {
            clock_period_ = 0;
            data &= 0xC0;
        }
        break;
    case R_LCDC: case R_LCH: case R_LCV: {
        ram_[off] = data;
        const int htime = ((ram_[R_LCH] & 0x1f) + 1) * 200 / 4;
        const int vtime = ram_[R_LCV] & 0x1f;
        line_len_ = (uint32_t)(htime + 200);                // one line, in pixel clocks (= CPU cycles)
        frame_lines_ = (uint32_t)(vtime + 160);
        break;
    }
    case R_SGC: snd_.sgc = data; break;
    case R_SG0L: snd_.sg0l = data; break;
    case R_SG1L: snd_.sg1l = data; break;
    case R_SG0TH: snd_.sg0t = (uint16_t)((snd_.sg0t & 0xFF) | (data << 8)); break;
    case R_SG0TL: snd_.sg0t = (uint16_t)((snd_.sg0t & 0xFF00) | data); break;
    case R_SG1TH: snd_.sg1t = (uint16_t)((snd_.sg1t & 0xFF) | (data << 8)); break;
    case R_SG1TL: snd_.sg1t = (uint16_t)((snd_.sg1t & 0xFF00) | data); break;
    case R_SG2L: snd_.sg2l = data; break;
    case R_SG2TH: snd_.sg2t = (uint16_t)((snd_.sg2t & 0xFF) | (data << 8)); break;
    case R_SG2TL: snd_.sg2t = (uint16_t)((snd_.sg2t & 0xFF00) | data); break;
    case R_SGDA:
        snd_.sgda = data;
        if ((snd_.sgc & 0x8f) == 0x88) dac_ = data;
        break;
    default:
        if (off >= R_SG0W0 && off < R_SG0W0 + 16) snd_.sg0w[off - R_SG0W0] = data;
        else if (off >= R_SG1W0 && off < R_SG1W0 + 16) snd_.sg1w[off - R_SG1W0] = data;
        break;
    }
    ram_[off] = data;
}

// MAME gamecom_handle_dma(), one block per call (the CPU calls it while halted).
void GameCom::handle_dma() {
    const uint8_t dmc = ram_[R_DMC];
    if (!(dmc & 0x80)) return;
    const bool overwrite = dmc & 1;
    const int mode = dmc & 6;
    const int adjust_x = (dmc & 8) ? -1 : 1;
    const bool decrement_y = dmc & 0x10;
    const uint8_t bw = ram_[R_DMDX], bh = ram_[R_DMDY];
    const uint8_t sx = ram_[R_DMX1], sy = ram_[R_DMY1], dx = ram_[R_DMX2], dy = ram_[R_DMY2];
    const uint8_t pal = ram_[R_DMPL];
    int src_w = (ram_[R_LCH] & 0x20) ? 50 : 40, dst_w = src_w;
    uint16_t src_mask = 0x1FFF, dst_mask = 0x1FFF;
    const uint8_t* src = &vram_[(ram_[R_DMVP] & 1) ? 0x2000 : 0];
    uint8_t* dst = &vram_[(ram_[R_DMVP] & 2) ? 0x2000 : 0];
    switch (mode) {
    case 2:  // ROM -> VRAM
        src_w = 64;
        src_mask = 0x3FFF;
        if (ram_[R_DMBR] < 16) {
            const uint32_t o = (uint32_t)ram_[R_DMBR] << 14;
            if (o + 0x4000 <= kernel_.size()) src = &kernel_[o];
        } else if (cart_sel_ && !cart_.empty()) {
            const uint32_t o = (uint32_t)ram_[R_DMBR] << 14;
            if (o + 0x4000 <= cart_.size()) src = &cart_[o];
        }
        break;
    case 4: src_w = 64; src = nvram_; break;   // extended RAM -> VRAM
    case 6: dst_w = 64; dst = nvram_; break;   // VRAM -> extended RAM
    default: break;
    }
    uint16_t src_cur = (uint16_t)(src_w * sy + (sx >> 2)), dst_cur = (uint16_t)(dst_w * dy + (dx >> 2));
    uint16_t src_line = src_cur, dst_line = dst_cur;
    uint8_t sxc = sx & 3, dxc = dx & 3;
    for (int y = 0; y <= bh; y++) {
        for (int x = 0; x <= bw; x++) {
            const uint16_t sa = src_cur & src_mask, da = dst_cur & dst_mask;
            const uint8_t dadj = (uint8_t)((dxc ^ 3) << 1), sadj = (uint8_t)((sxc ^ 3) << 1);
            const uint8_t px = (src[sa] >> sadj) & 3;
            if (overwrite || px) {
                const uint8_t other = dst[da] & (uint8_t)~(3 << dadj);
                dst[da] = (uint8_t)(other | (((pal >> (px << 1)) & 3) << dadj));
            }
            sxc = (uint8_t)(sxc + adjust_x);
            if (sxc & 4) { src_cur = (uint16_t)(src_cur + adjust_x); sxc &= 3; }
            dxc++;
            if (dxc & 4) { dst_cur++; dxc &= 3; }
        }
        sxc = sx & 3;
        dxc = dx & 3;
        src_line = (uint16_t)(decrement_y ? src_line - src_w : src_line + src_w);
        src_cur = src_line;
        dst_line = (uint16_t)(dst_line + dst_w);
        dst_cur = dst_line;
    }
    ram_[R_DMC] &= 0x7f;
    cpu_->set_input_line(sm8500_cpu::DMA_INT, true);
}

void GameCom::update_timers(int cycles) {
    for (int t = 0; t < 2; t++) {
        Timer& tm = timer_[t];
        if (!tm.enabled || tm.prescale_max == 0) continue;
        const uint16_t d = t ? R_TM1D : R_TM0D;
        tm.prescale_count += (uint32_t)cycles;
        while (tm.prescale_count >= tm.prescale_max) {
            tm.prescale_count -= tm.prescale_max;
            ram_[d]++;
            if (ram_[d] >= tm.upcounter_max) {
                ram_[d] = 0;
                if (t == 0) {
                    if ((ram_[R_IE0] & 0x40) && (ram_[R_PS1] & 1)) cpu_->set_input_line(sm8500_cpu::TIM0_INT, true);
                } else {
                    if ((ram_[R_IE1] & 0x40) && (ram_[R_PS1] & 1) && ((ram_[R_PS0] & 7) < 4))
                        cpu_->set_input_line(sm8500_cpu::TIM1_INT, true);
                }
            }
        }
    }
}

void GameCom::draw_column(int x) {
    const uint8_t lcdc = ram_[R_LCDC];
    if (!(lcdc & 0x80)) {
        for (int y = 0; y < kH; y++) shade_[y][x] = 4;  // LCD off: MAME pen 0 (black)
        return;
    }
    int pal[4];
    switch (lcdc & 0x30) {
    case 0x00: pal[0] = 4; pal[1] = 3; pal[2] = 2; pal[3] = 0; break;
    case 0x30: pal[0] = 4; pal[1] = 2; pal[2] = 1; pal[3] = 0; break;
    default:   pal[0] = 4; pal[1] = 3; pal[2] = 1; pal[3] = 0; break;
    }
    const uint8_t* line = &vram_[((lcdc & 0x40) ? 0x2000 : 0) + 40 * x];
    for (int i = 0; i < 40; i++) {
        const uint8_t p = line[i];
        for (int k = 0; k < 4; k++) shade_[i * 4 + k][x] = (uint8_t)(4 - pal[(p >> (6 - 2 * k)) & 3]);
    }
}

void GameCom::sound_tick(int cycles) {
    // Waveform channels: one nibble every 2*SGnT cycles (MAME: 2764800/SGnT Hz),
    // 32 nibbles, restarted while enabled.
    for (int c = 0; c < 2; c++) {
        const uint16_t t = c ? snd_.sg1t : snd_.sg0t;
        const uint8_t* w = c ? snd_.sg1w : snd_.sg0w;
        const uint8_t en = c ? 0x82 : 0x81;
        snd_acc_[c] += (uint32_t)cycles;
        const uint32_t period = t ? 2u * t : 0x10000u;
        while (snd_acc_[c] >= period) {
            snd_acc_[c] -= period;
            if (snd_cnt_[c] > 0x1f && t > 0 && (snd_.sgc & en) == en) snd_cnt_[c] = 0;
            if (snd_cnt_[c] < 0x20) {
                snd_out_[c] = (uint8_t)((w[snd_cnt_[c] >> 1] >> ((snd_cnt_[c] & 1) * 4)) & 0xf);
                snd_cnt_[c]++;
            }
        }
    }
    // MAME's mix: the 8-bit DAC (holds its last value) at 0.5, each 4-bit
    // channel at 0.25 times SGnL/31.
    sound_level_ = 0.5f * (float)dac_ / 255.0f
                 + 0.25f * (float)snd_out_[0] / 15.0f * (float)(snd_.sg0l & 0x1f) / 31.0f
                 + 0.25f * (float)snd_out_[1] / 15.0f * (float)(snd_.sg1l & 0x1f) / 31.0f;
    // Optional noise channel (SG2), a guess: a random bit every 2*SG2T cycles.
    if (noise_on_ && (snd_.sgc & 0x84) == 0x84 && snd_.sg2t) {
        noise_acc_ += (uint32_t)cycles;
        while (noise_acc_ >= 2u * snd_.sg2t) {
            noise_acc_ -= 2u * snd_.sg2t;
            noise_lfsr_ = (noise_lfsr_ >> 1) ^ (-(noise_lfsr_ & 1u) & 0x80200003u);
            noise_out_ = noise_lfsr_ & 1;
        }
        sound_level_ += 0.25f * (float)noise_out_ * (float)(snd_.sg2l & 0x1f) / 31.0f;
    }
    if (basic_sound_) {
        // The one-bit buzzer: the DAC when it's in use, else the louder waveform.
        bool level;
        if ((snd_.sgc & 0x8f) == 0x88) {
            level = dac_ >= 0x80;
        } else {
            const int a = (snd_.sgc & 0x81) == 0x81 && snd_.sg0l ? snd_out_[0] : -1;
            const int b = (snd_.sgc & 0x82) == 0x82 && snd_.sg1l ? snd_out_[1] : -1;
            level = (a >= 8) || (b >= 8);
        }
        set_speaker(level);
    } else {
        set_speaker(sound_level_ > 0.5f);
    }
}

void GameCom::reset() {
    const std::vector<uint8_t>* b = rom_info().blob("rom");
    irom_ = b ? *b : std::vector<uint8_t>();
    const std::vector<uint8_t>* k = rom_info().blob("kernel");
    kernel_ = k ? *k : std::vector<uint8_t>();
    kernel_.resize(0x40000, 0xff);
    cart_.clear();
    const std::vector<uint8_t>* c = rom_info().blob("cart");
    if (c && !c->empty()) {
        // As MAME's common_load(): 2 MB, mirrored up; 1.75 MB carts at 256 KB.
        const size_t size = c->size();
        const size_t load = size == 0x1c0000 ? 0x40000 : 0;
        cart_.assign(0x200000, 0xff);
        memcpy(&cart_[load], c->data(), std::min(size, cart_.size() - load));
        if (size < 0x010000) memcpy(&cart_[0x008000], &cart_[0], 0x008000);
        if (size < 0x020000) memcpy(&cart_[0x010000], &cart_[0], 0x010000);
        if (size < 0x040000) memcpy(&cart_[0x020000], &cart_[0], 0x020000);
        if (size < 0x080000) memcpy(&cart_[0x040000], &cart_[0], 0x040000);
        if (size < 0x100000) memcpy(&cart_[0x080000], &cart_[0], 0x080000);
        if (size < 0x1c0000) memcpy(&cart_[0x100000], &cart_[0], 0x100000);
    }
    memset(ram_, 0, sizeof(ram_));
    memset(vram_, 0, sizeof(vram_));
    memset(nvram_, 0, sizeof(nvram_));
    memset(bank_, 0, sizeof(bank_));
    cart_sel_ = false;
    timer_[0] = timer_[1] = Timer();
    snd_ = Sound();
    snd_acc_[0] = snd_acc_[1] = 0;
    snd_cnt_[0] = snd_cnt_[1] = 0x40;
    snd_out_[0] = snd_out_[1] = 0;
    dac_ = 0x80;
    clock_acc_ = clock_period_ = 0;
    ram_[R_LCH] = 0x07;
    ram_[R_LCV] = 0x27;
    ram_[R_LCDC] = 0xb0;
    line_len_ = 600;
    frame_lines_ = 167;
    line_acc_ = 0;
    line_ = 0;
    autoboot_frame_ = autoboot_mapped_ = 0;
    autoboot_done_ = false;
    memset(shade_, 0, sizeof(shade_));
    cpu_->reset();
}

int GameCom::execute() {
    const int cycles = cpu_->run(1);
    // LCD: frame_lines_ lines of line_len_ pixel clocks; the picture is
    // taken at the end of the visible part, then the vblank interrupt.
    line_acc_ += (uint32_t)cycles;
    while (line_acc_ >= line_len_) {
        line_acc_ -= line_len_;
        line_++;
        if (line_ == 160) {
            for (int x = 0; x < kW; x++) draw_column(x);
            cpu_->set_input_line(sm8500_cpu::LCDC_INT, true);
            autoboot_frame();
        }
        if (line_ >= frame_lines_) line_ = 0;
    }
    if (clock_period_) {
        clock_acc_ += (uint32_t)cycles;
        if (clock_acc_ >= clock_period_) {
            clock_acc_ -= clock_period_;
            ram_[R_CLKT] = (uint8_t)((ram_[R_CLKT] & 0xC0) | ((ram_[R_CLKT] + 1) & 0x3f));
            cpu_->set_input_line(sm8500_cpu::CK_INT, true);
        }
    }
    sound_tick(cycles);
    return cycles > 0 ? cycles : 1;
}

void GameCom::autoboot_frame() {
    if (autoboot_done_ || cart_.empty()) return;
    autoboot_frame_++;
    autoboot_mapped_ = cart_mapped() ? autoboot_mapped_ + 1 : 0;
    if (autoboot_mapped_ >= 60) {  // the cartridge's code has been running for a second
        autoboot_done_ = true;
        stylus_x_ = stylus_y_ = -1;
        return;
    }
    // From the second second on: tap (35, 40) - the CARTRIDGE icon, and
    // anywhere on the intro to skip it - 6 frames down, 24 up.
    const bool down = autoboot_frame_ >= 60 && (autoboot_frame_ % 30) < 6;
    stylus_x_ = down ? 35 : -1;
    stylus_y_ = down ? 40 : -1;
}

void GameCom::set_stylus(int x, int y, bool down) {
    if (down && x >= 0 && x < kW && y >= 0 && y < kH) { stylus_x_ = x; stylus_y_ = y; }
    else stylus_x_ = stylus_y_ = -1;
}

int GameCom::port_id(const std::string& name) const {
    if (name == "IN0") return 0;
    if (name == "IN1") return 1;
    if (name == "IN2") return 2;
    return -1;
}

void GameCom::port_input(int port, uint8_t mask, int level) {
    if (port < 0 || port > 2) return;
    if (level == 1) in_[port] &= (uint8_t)~mask;  // active low
    else in_[port] |= mask;
}

bool GameCom::poke_ram(uint32_t addr, uint8_t value) {
    if (addr < 0x400) { ram_[addr] = value; return true; }
    if (addr >= 0xE000 && addr < 0x10000) { nvram_[addr - 0xE000] = value; return true; }
    return false;
}

int GameCom::peek_ram(uint32_t addr) const {
    if (addr < 0x400) return ram_[addr];
    if (addr >= 0xE000 && addr < 0x10000) return nvram_[addr - 0xE000];
    return -1;
}

void GameCom::serialize(BrickStateIO& io) {
    sm8500_cpu& c = *cpu_;
    io.pod(c.m_PC); io.pod(c.m_IE0); io.pod(c.m_IE1); io.pod(c.m_IR0); io.pod(c.m_IR1); io.pod(c.m_SYS);
    io.pod(c.m_CKC); io.pod(c.m_clock_changed); io.pod(c.m_SP); io.pod(c.m_PS0); io.pod(c.m_PS1);
    io.pod(c.m_IFLAGS); io.pod(c.m_CheckInterrupts); io.pod(c.m_halted); io.pod(c.m_oldpc);
    io.bytes(c.m_register_ram, sizeof(c.m_register_ram));
    io.bytes(ram_, sizeof(ram_));
    io.bytes(vram_, sizeof(vram_));
    io.bytes(nvram_, sizeof(nvram_));
    io.bytes(bank_, sizeof(bank_));
    io.pod(cart_sel_);
    io.bytes(timer_, sizeof(timer_));
    io.bytes(&snd_, sizeof(snd_));
    io.bytes(snd_acc_, sizeof(snd_acc_));
    io.bytes(snd_cnt_, sizeof(snd_cnt_));
    io.bytes(snd_out_, sizeof(snd_out_));
    io.pod(dac_);
    io.pod(clock_acc_); io.pod(clock_period_);
    io.pod(autoboot_frame_); io.pod(autoboot_mapped_); io.pod(autoboot_done_);
    io.pod(line_acc_); io.pod(line_len_); io.pod(frame_lines_); io.pod(line_);
    if (!io.writing()) for (int x = 0; x < kW; x++) draw_column(x);
}
