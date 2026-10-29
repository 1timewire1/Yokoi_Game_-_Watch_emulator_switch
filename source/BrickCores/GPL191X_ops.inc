// Generated from BrickEmuPy cores/GPL191X.py (opcode handlers) by transpile.py.
void GPL191X::adc(int64_t operand) {
    int64_t A, new_value;
    A = A_;
    new_value = A + operand + CF_;
    if ((DF_) && ((A & 0x0F) + (operand & 0x0F) + CF_ > 9)) {
        new_value += 6;
    }
    VF_ = ((~(A ^ operand) & (A ^ new_value)) >> 7) & 0x1;
    NF_ = (new_value >> 7) & 0x1;
    if ((DF_) && (new_value > 0x99)) {
        new_value += 0x60;
    }
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 255;
    A_ = new_value & 0xFF;
}

void GPL191X::sbc(int64_t operand) {
    int64_t A, new_value;
    A = A_;
    new_value = A - operand - (!CF_);
    if (DF_) {
        if ((A & 0x0F) - (operand & 0x0F) - (!CF_) < 0) {
            new_value -= 6;
        }
        if (new_value < 0) {
            new_value -= 0x60;
        }
    }
    VF_ = (((A ^ operand) & (A ^ new_value)) >> 7) & 0x1;
    NF_ = (new_value >> 7) & 0x1;
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value >= 0;
    A_ = new_value & 0xFF;
}

int GPL191X::op_brk() {
    BF_ = 1;
    irq();
    return (int)(7);
}

int GPL191X::op_ora_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    A_ |= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(6);
}

int GPL191X::op_ora_zp() {
    A_ |= rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(3);
}

int GPL191X::op_asl_zp() {
    int64_t new_value, zp;
    zp = rd(PC_);
    new_value = rd(zp) << 1;
    wr(zp, new_value & 0xFF);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(5);
}

int GPL191X::op_php() {
    wr(SP_ | kStack, get_ps());
    SP_ = (SP_ - 1) & 0xFF;
    return (int)(3);
}

int GPL191X::op_ora_imm() {
    A_ |= rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(2);
}

int GPL191X::op_asl_a() {
    int64_t new_value;
    new_value = A_ << 1;
    A_ = new_value & 0xFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value >> 8;
    return (int)(2);
}

int GPL191X::op_ora_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    A_ |= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_asl_abs() {
    int64_t addr, new_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    new_value = rd(addr) << 1;
    wr(addr, new_value & 0xFF);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(6);
}

int GPL191X::op_bpl() {
    int64_t prev_PC, rel;
    if (!NF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_ora_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ |= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(5 + ((base ^ addr) > 255));
}

int GPL191X::op_ora_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    A_ |= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(3);
}

int GPL191X::op_asl_zp_x() {
    int64_t addr, new_value;
    addr = (rd(PC_) + X_) & 0xFF;
    new_value = rd(addr) << 1;
    wr(addr, new_value & 0xFF);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(6);
}

int GPL191X::op_ora_abs_y() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ |= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_and_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    A_ &= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(6);
}

int GPL191X::op_ora_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    A_ |= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_asl_abs_x() {
    int64_t addr, base, new_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    new_value = rd(addr) << 1;
    wr(addr, new_value & 0xFF);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(6 + ((base ^ addr) > 255));
}

int GPL191X::op_clc() {
    CF_ = 0;
    return (int)(2);
}

int GPL191X::op_jsr_abs() {
    int64_t pc;
    pc = (PC_ + 1) & 0xFFFF;
    wr(SP_ | kStack, pc >> 8);
    SP_ = (SP_ - 1) & 0xFF;
    wr(SP_ | kStack, pc & 0xFF);
    SP_ = (SP_ - 1) & 0xFF;
    PC_ = rd(PC_) | (rd(pc) << 8);
    return (int)(6);
}

int GPL191X::op_and_abs_y() {
    int64_t addr;
    addr = ((rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8)) + Y_) & 0xFFFF;
    A_ &= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_and_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    A_ &= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_rol_abs_x() {
    int64_t addr, base, new_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    new_value = (rd(addr) << 1) | CF_;
    wr(addr, new_value & 0xFF);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(6 + ((base ^ addr) > 255));
}

int GPL191X::op_bit_zp() {
    int64_t m;
    m = rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = m >> 7;
    VF_ = ((m & 0x40) > 0);
    ZF_ = !(A_ & m);
    return (int)(3);
}

int GPL191X::op_and_zp() {
    A_ &= rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(3);
}

int GPL191X::op_rol_zp() {
    int64_t new_value, opcode;
    opcode = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    new_value = (rd(opcode) << 1) | CF_;
    wr(opcode, new_value & 0xFF);
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(5);
}

int GPL191X::op_plp() {
    SP_ = (SP_ + 1) & 0xFF;
    set_ps(rd(SP_ | kStack));
    return (int)(4);
}

int GPL191X::op_and_imm() {
    A_ &= rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(2);
}

int GPL191X::op_rol_a() {
    int64_t new_value;
    new_value = (A_ << 1) | CF_;
    A_ = new_value & 0xFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(2);
}

int GPL191X::op_bit_abs() {
    int64_t addr, m;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    m = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = m >> 7;
    VF_ = ((m & 0x40) > 0);
    ZF_ = !(A_ & m);
    return (int)(4);
}

int GPL191X::op_and_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    A_ &= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_rol_abs() {
    int64_t addr, new_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    new_value = (rd(addr) << 1) | CF_;
    wr(addr, new_value & 0xFF);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(5);
}

int GPL191X::op_bmi() {
    int64_t prev_PC, rel;
    if (NF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_and_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ &= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(5 + ((base ^ addr) > 255));
}

int GPL191X::op_and_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    A_ &= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(6);
}

int GPL191X::op_rol_zp_x() {
    int64_t addr, new_value;
    addr = (rd(PC_) + X_) & 0xFF;
    new_value = (rd(addr) << 1) | CF_;
    wr(addr, new_value & 0xFF);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = ((new_value & 0x80) > 0);
    ZF_ = !(new_value & 0xFF);
    CF_ = new_value > 0xFF;
    return (int)(6);
}

int GPL191X::op_sec() {
    CF_ = 1;
    return (int)(2);
}

int GPL191X::op_rti() {
    SP_ = (SP_ + 1) & 0xFF;
    set_ps(rd(SP_ | kStack));
    SP_ = (SP_ + 1) & 0xFF;
    PC_ = rd(SP_ | kStack);
    SP_ = (SP_ + 1) & 0xFF;
    PC_ |= rd(SP_ | kStack) << 8;
    return (int)(6);
}

int GPL191X::op_eor_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    A_ ^= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_eor_zp() {
    A_ ^= rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(3);
}

int GPL191X::op_lsr_zp() {
    int64_t addr, prev_value;
    addr = rd(PC_);
    prev_value = rd(addr);
    wr(addr, prev_value >> 1);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = 0;
    ZF_ = !(prev_value & 0xFE);
    CF_ = prev_value & 0x01;
    return (int)(5);
}

int GPL191X::op_pha() {
    wr(SP_ | kStack, A_);
    SP_ = (SP_ - 1) & 0xFF;
    return (int)(3);
}

int GPL191X::op_eor_imm() {
    A_ ^= rd(PC_) & 0xFF;
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(2);
}

int GPL191X::op_lsr_a() {
    int64_t prev_value;
    prev_value = A_;
    A_ = (prev_value >> 1);
    NF_ = 0;
    ZF_ = !(prev_value & 0xFE);
    CF_ = prev_value & 0x01;
    return (int)(2);
}

int GPL191X::op_jmp_abs() {
    PC_ = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    return (int)(3);
}

int GPL191X::op_eor_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    A_ ^= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_lsr_abs() {
    int64_t addr, prev_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    prev_value = rd(addr);
    wr(addr, prev_value >> 1);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = 0;
    ZF_ = !(prev_value & 0xFE);
    CF_ = prev_value & 0x01;
    return (int)(6);
}

int GPL191X::op_bvc() {
    int64_t prev_PC, rel;
    if (!VF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_eor_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ ^= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(5 + ((base ^ addr) > 255));
}

int GPL191X::op_eor_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    A_ ^= rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_lsr_zp_x() {
    int64_t addr, prev_value;
    addr = (rd(PC_) + X_) & 0xFF;
    prev_value = rd(addr);
    wr(addr, prev_value >> 1);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = 0;
    ZF_ = !(prev_value & 0xFE);
    CF_ = prev_value & 0x01;
    return (int)(6);
}

int GPL191X::op_cli() {
    IF_ = 0;
    return (int)(2);
}

int GPL191X::op_eor_abs_y() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ ^= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_eor_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    A_ ^= rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_lsr_abs_x() {
    int64_t addr, base, prev_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    prev_value = rd(addr);
    wr(addr, prev_value >> 1);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = 0;
    ZF_ = !(prev_value & 0xFE);
    CF_ = prev_value & 0x01;
    return (int)(6 + ((base ^ addr) > 255));
}

int GPL191X::op_rts() {
    SP_ = (SP_ + 1) & 0xFF;
    PC_ = rd(SP_ | kStack);
    SP_ = (SP_ + 1) & 0xFF;
    PC_ |= rd(SP_ | kStack) << 8;
    PC_ += 1;
    return (int)(6);
}

int GPL191X::op_adc_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    adc(rd(addr));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(6);
}

int GPL191X::op_adc_zp() {
    adc(rd(rd(PC_)));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(3);
}

int GPL191X::op_ror_zp() {
    int64_t opcode, prev_value;
    opcode = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    prev_value = rd(opcode);
    wr(opcode, (prev_value >> 1) | (CF_ << 7));
    NF_ = CF_;
    ZF_ = !((prev_value & 0xFE) | CF_);
    CF_ = prev_value & 0x01;
    return (int)(5);
}

int GPL191X::op_pla() {
    SP_ = (SP_ + 1) & 0xFF;
    A_ = rd(SP_ | kStack);
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_adc_imm() {
    adc(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_ror_a() {
    int64_t prev_value;
    prev_value = A_;
    A_ = (prev_value >> 1) | (CF_ << 7);
    NF_ = CF_;
    ZF_ = !((prev_value & 0xFE) | CF_);
    CF_ = prev_value & 0x01;
    return (int)(2);
}

int GPL191X::op_jmp_ind() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    PC_ = rd(addr);
    PC_ |= rd((addr + 1) & 0xFFFF) << 8;
    return (int)(6);
}

int GPL191X::op_adc_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    adc(rd(addr));
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_ror_abs() {
    int64_t addr, prev_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    prev_value = rd(addr);
    wr(addr, (prev_value >> 1) | (CF_ << 7));
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = CF_;
    ZF_ = !((prev_value & 0xFE) | CF_);
    CF_ = prev_value & 0x01;
    return (int)(6);
}

int GPL191X::op_bvs() {
    int64_t prev_PC, rel;
    if (VF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_adc_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    adc(rd(addr));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(5 + ((base ^ addr) > 255));
}

int GPL191X::op_adc_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    adc(rd(addr));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(5);
}

int GPL191X::op_ror_zp_x() {
    int64_t addr, prev_value;
    addr = (rd(PC_) + X_) & 0xFF;
    prev_value = rd(addr);
    wr(addr, (prev_value >> 1) | (CF_ << 7));
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = CF_;
    ZF_ = !((prev_value & 0xFE) | CF_);
    CF_ = prev_value & 0x01;
    return (int)(6);
}

int GPL191X::op_sei() {
    IF_ = 1;
    return (int)(2);
}

int GPL191X::op_adc_abs_y() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    adc(rd(addr));
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_adc_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    adc(rd(addr));
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_ror_abs_x() {
    int64_t addr, base, prev_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    prev_value = rd(addr);
    wr(addr, (prev_value >> 1) | (CF_ << 7));
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = CF_;
    ZF_ = !((prev_value & 0xFE) | CF_);
    CF_ = prev_value & 0x01;
    return (int)(6 + ((base ^ addr) > 255));
}

int GPL191X::op_sta_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    wr(addr, A_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(6);
}

int GPL191X::op_sty_zp() {
    wr(rd(PC_), Y_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(3);
}

int GPL191X::op_sta_zp() {
    wr(rd(PC_), A_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(3);
}

int GPL191X::op_stx_zp() {
    wr(rd(PC_), X_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(3);
}

int GPL191X::op_dey() {
    Y_ = (Y_ - 1) & 0xFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(2);
}

int GPL191X::op_txa() {
    A_ = X_;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(2);
}

int GPL191X::op_sty_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    wr(addr, Y_);
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_sta_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    wr(addr, A_);
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_stx_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    wr(addr, X_);
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_bcc() {
    int64_t prev_PC, rel;
    if (!CF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_sta_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    wr(addr, A_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(6);
}

int GPL191X::op_sty_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    wr(addr, Y_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_sta_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    wr(addr, A_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_stx_zp_y() {
    int64_t addr;
    addr = (rd(PC_) + Y_) & 0xFF;
    wr(addr, X_);
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_tya() {
    A_ = Y_;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(2);
}

int GPL191X::op_sta_abs_y() {
    int64_t addr;
    addr = ((rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8)) + Y_) & 0xFFFF;
    wr(addr, A_);
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(5);
}

int GPL191X::op_txs() {
    SP_ = X_;
    return (int)(2);
}

int GPL191X::op_sta_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    wr(addr, A_);
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(5);
}

int GPL191X::op_ldy_imm() {
    Y_ = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(2);
}

int GPL191X::op_lda_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    A_ = rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(6);
}

int GPL191X::op_ldx_imm() {
    X_ = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(2);
}

int GPL191X::op_ldy_zp() {
    Y_ = rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(3);
}

int GPL191X::op_lda_zp() {
    A_ = rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(3);
}

int GPL191X::op_ldx_zp() {
    X_ = rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(3);
}

int GPL191X::op_tay() {
    Y_ = A_;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(2);
}

int GPL191X::op_lda_imm() {
    A_ = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(2);
}

int GPL191X::op_tax() {
    X_ = A_;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(2);
}

int GPL191X::op_ldy_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    Y_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(4);
}

int GPL191X::op_lda_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    A_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_ldx_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    X_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(4);
}

int GPL191X::op_bcs() {
    int64_t prev_PC, rel;
    if (CF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_lda_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ = rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(5 + ((base ^ addr) > 255));
}

int GPL191X::op_ldy_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    Y_ = rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(4);
}

int GPL191X::op_lda_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    A_ = rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4);
}

int GPL191X::op_ldx_zp_y() {
    int64_t addr;
    addr = (rd(PC_) + Y_) & 0xFF;
    X_ = rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(4);
}

int GPL191X::op_clv() {
    VF_ = 0;
    return (int)(2);
}

int GPL191X::op_lda_abs_y() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    A_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_tsx() {
    X_ = SP_;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(2);
}

int GPL191X::op_ldy_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    Y_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_lda_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    A_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = A_ >> 7;
    ZF_ = !A_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_ldx_abs_y() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    X_ = rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_cpy_imm() {
    int64_t test_value;
    test_value = Y_ - rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(2);
}

int GPL191X::op_cmp_ind_x() {
    int64_t addr, test_value, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    test_value = A_ - rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(6);
}

int GPL191X::op_cpy_zp() {
    int64_t test_value;
    test_value = Y_ - rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(3);
}

int GPL191X::op_cmp_zp() {
    int64_t test_value;
    test_value = A_ - rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(3);
}

int GPL191X::op_dec_zp() {
    int64_t new_value, opcode;
    opcode = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    new_value = (rd(opcode) - 1) & 0xFF;
    wr(opcode, new_value);
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(5);
}

int GPL191X::op_iny() {
    Y_ = (Y_ + 1) & 0xFF;
    NF_ = Y_ >> 7;
    ZF_ = !Y_;
    return (int)(2);
}

int GPL191X::op_cmp_imm() {
    int64_t test_value;
    test_value = A_ - (rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(2);
}

int GPL191X::op_cpy_abs() {
    int64_t addr, test_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    test_value = Y_ - rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(3);
}

int GPL191X::op_cmp_abs() {
    int64_t addr, test_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    test_value = A_ - rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(4);
}

int GPL191X::op_dec_abs() {
    int64_t addr, new_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    new_value = (rd(addr) - 1) & 0xFF;
    wr(addr, new_value);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(5);
}

int GPL191X::op_dex() {
    X_ = (X_ - 1) & 0xFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(2);
}

int GPL191X::op_bne() {
    int64_t prev_PC, rel;
    if (!ZF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_cmp_ind_y() {
    int64_t addr, base, test_value, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    test_value = A_ - rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(5 + ((base ^ addr) > 255));
}

int GPL191X::op_cmp_zp_x() {
    int64_t addr, test_value;
    addr = (rd(PC_) + X_) & 0xFF;
    test_value = A_ - rd(addr);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(4);
}

int GPL191X::op_dec_zp_x() {
    int64_t addr, new_value;
    addr = (rd(PC_) + X_) & 0xFF;
    new_value = (rd(addr) - 1) & 0xFF;
    wr(addr, new_value);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(6);
}

int GPL191X::op_cld() {
    DF_ = 0;
    return (int)(2);
}

int GPL191X::op_cmp_abs_y() {
    int64_t addr, base, test_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    test_value = A_ - rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_cmp_abs_x() {
    int64_t addr, base, test_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    test_value = A_ - rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_dec_abs_x() {
    int64_t addr, base, new_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    new_value = (rd(addr) - 1) & 0xFF;
    wr(addr, new_value);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(7);
}

int GPL191X::op_cpx_imm() {
    int64_t test_value;
    test_value = X_ - rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(2);
}

int GPL191X::op_sbc_ind_x() {
    int64_t addr, zp;
    zp = rd(PC_) + X_;
    addr = rd(zp & 0xFF) | (rd((zp + 1) & 0xFF) << 8);
    sbc(rd(addr));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(6);
}

int GPL191X::op_cpx_zp() {
    int64_t test_value;
    test_value = X_ - rd(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(3);
}

int GPL191X::op_sbc_zp() {
    sbc(rd(rd(PC_)));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(3);
}

int GPL191X::op_inc_zp() {
    int64_t new_value, opcode;
    opcode = rd(PC_);
    PC_ = (PC_ + 1) & 0xFFFF;
    new_value = (rd(opcode) + 1) & 0xFF;
    wr(opcode, new_value);
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(5);
}

int GPL191X::op_inx() {
    X_ = (X_ + 1) & 0xFF;
    NF_ = X_ >> 7;
    ZF_ = !X_;
    return (int)(2);
}

int GPL191X::op_sbc_imm() {
    sbc(rd(PC_));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_cpx_abs() {
    int64_t addr, test_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    test_value = X_ - rd(addr);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = (test_value >> 7) & 0x1;
    ZF_ = !test_value;
    CF_ = test_value >= 0;
    return (int)(4);
}

int GPL191X::op_sbc_abs() {
    int64_t addr;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    sbc(rd(addr));
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_inc_abs() {
    int64_t addr, new_value;
    addr = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    new_value = (rd(addr) + 1) & 0xFF;
    wr(addr, new_value);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(6);
}

int GPL191X::op_nop() {
    return (int)(2);
}

int GPL191X::op_beq() {
    int64_t prev_PC, rel;
    if (ZF_) {
        rel = rd(PC_);
        prev_PC = (PC_ + 1) & 0xFFFF;
        PC_ = (prev_PC + rel - ((rel & 0x80) << 1)) & 0xFFFF;
        return (int)(3 + ((PC_ ^ prev_PC) > 255));
    }
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(2);
}

int GPL191X::op_sbc_ind_y() {
    int64_t addr, base, zp;
    zp = rd(PC_);
    base = rd(zp) | (rd((zp + 1) & 0xFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    sbc(rd(addr));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_sbc_zp_x() {
    int64_t addr;
    addr = (rd(PC_) + X_) & 0xFF;
    sbc(rd(addr));
    PC_ = (PC_ + 1) & 0xFFFF;
    return (int)(4);
}

int GPL191X::op_inc_zp_x() {
    int64_t addr, new_value;
    addr = (rd(PC_) + X_) & 0xFF;
    new_value = (rd(addr) + 1) & 0xFF;
    wr(addr, new_value);
    PC_ = (PC_ + 1) & 0xFFFF;
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(6);
}

int GPL191X::op_sed() {
    DF_ = 1;
    return (int)(2);
}

int GPL191X::op_sbc_abs_y() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + Y_) & 0xFFFF;
    sbc(rd(addr));
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_sbc_abs_x() {
    int64_t addr, base;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    sbc(rd(addr));
    PC_ = (PC_ + 2) & 0xFFFF;
    return (int)(4 + ((base ^ addr) > 255));
}

int GPL191X::op_inc_abs_x() {
    int64_t addr, base, new_value;
    base = rd(PC_) | (rd((PC_ + 1) & 0xFFFF) << 8);
    addr = (base + X_) & 0xFFFF;
    new_value = (rd(addr) + 1) & 0xFF;
    wr(addr, new_value);
    PC_ = (PC_ + 2) & 0xFFFF;
    NF_ = new_value >> 7;
    ZF_ = !new_value;
    return (int)(7);
}

int GPL191X::op_dummy() {
    return 2;
}

const GPL191X::OpFn GPL191X::kOps[256] = {
    &GPL191X::op_brk, &GPL191X::op_ora_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_ora_zp, &GPL191X::op_asl_zp, &GPL191X::op_dummy,
    &GPL191X::op_php, &GPL191X::op_ora_imm, &GPL191X::op_asl_a, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_ora_abs, &GPL191X::op_asl_abs, &GPL191X::op_dummy,
    &GPL191X::op_bpl, &GPL191X::op_ora_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_ora_zp_x, &GPL191X::op_asl_zp_x, &GPL191X::op_dummy,
    &GPL191X::op_clc, &GPL191X::op_ora_abs_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_ora_abs_x, &GPL191X::op_asl_abs_x, &GPL191X::op_dummy,
    &GPL191X::op_jsr_abs, &GPL191X::op_and_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_bit_zp, &GPL191X::op_and_zp, &GPL191X::op_rol_zp, &GPL191X::op_dummy,
    &GPL191X::op_plp, &GPL191X::op_and_imm, &GPL191X::op_rol_a, &GPL191X::op_dummy,
    &GPL191X::op_bit_abs, &GPL191X::op_and_abs, &GPL191X::op_rol_abs, &GPL191X::op_dummy,
    &GPL191X::op_bmi, &GPL191X::op_and_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_and_zp_x, &GPL191X::op_rol_zp_x, &GPL191X::op_dummy,
    &GPL191X::op_sec, &GPL191X::op_and_abs_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_and_abs_x, &GPL191X::op_rol_abs_x, &GPL191X::op_dummy,
    &GPL191X::op_rti, &GPL191X::op_eor_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_eor_zp, &GPL191X::op_lsr_zp, &GPL191X::op_dummy,
    &GPL191X::op_pha, &GPL191X::op_eor_imm, &GPL191X::op_lsr_a, &GPL191X::op_dummy,
    &GPL191X::op_jmp_abs, &GPL191X::op_eor_abs, &GPL191X::op_lsr_abs, &GPL191X::op_dummy,
    &GPL191X::op_bvc, &GPL191X::op_eor_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_eor_zp_x, &GPL191X::op_lsr_zp_x, &GPL191X::op_dummy,
    &GPL191X::op_cli, &GPL191X::op_eor_abs_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_eor_abs_x, &GPL191X::op_lsr_abs_x, &GPL191X::op_dummy,
    &GPL191X::op_rts, &GPL191X::op_adc_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_adc_zp, &GPL191X::op_ror_zp, &GPL191X::op_dummy,
    &GPL191X::op_pla, &GPL191X::op_adc_imm, &GPL191X::op_ror_a, &GPL191X::op_dummy,
    &GPL191X::op_jmp_ind, &GPL191X::op_adc_abs, &GPL191X::op_ror_abs, &GPL191X::op_dummy,
    &GPL191X::op_bvs, &GPL191X::op_adc_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_adc_zp_x, &GPL191X::op_ror_zp_x, &GPL191X::op_dummy,
    &GPL191X::op_sei, &GPL191X::op_adc_abs_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_adc_abs_x, &GPL191X::op_ror_abs_x, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_sta_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_sty_zp, &GPL191X::op_sta_zp, &GPL191X::op_stx_zp, &GPL191X::op_dummy,
    &GPL191X::op_dey, &GPL191X::op_dummy, &GPL191X::op_txa, &GPL191X::op_dummy,
    &GPL191X::op_sty_abs, &GPL191X::op_sta_abs, &GPL191X::op_stx_abs, &GPL191X::op_dummy,
    &GPL191X::op_bcc, &GPL191X::op_sta_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_sty_zp_x, &GPL191X::op_sta_zp_x, &GPL191X::op_stx_zp_y, &GPL191X::op_dummy,
    &GPL191X::op_tya, &GPL191X::op_sta_abs_y, &GPL191X::op_txs, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_sta_abs_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_ldy_imm, &GPL191X::op_lda_ind_x, &GPL191X::op_ldx_imm, &GPL191X::op_dummy,
    &GPL191X::op_ldy_zp, &GPL191X::op_lda_zp, &GPL191X::op_ldx_zp, &GPL191X::op_dummy,
    &GPL191X::op_tay, &GPL191X::op_lda_imm, &GPL191X::op_tax, &GPL191X::op_dummy,
    &GPL191X::op_ldy_abs, &GPL191X::op_lda_abs, &GPL191X::op_ldx_abs, &GPL191X::op_dummy,
    &GPL191X::op_bcs, &GPL191X::op_lda_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_ldy_zp_x, &GPL191X::op_lda_zp_x, &GPL191X::op_ldx_zp_y, &GPL191X::op_dummy,
    &GPL191X::op_clv, &GPL191X::op_lda_abs_y, &GPL191X::op_tsx, &GPL191X::op_dummy,
    &GPL191X::op_ldy_abs_x, &GPL191X::op_lda_abs_x, &GPL191X::op_ldx_abs_y, &GPL191X::op_dummy,
    &GPL191X::op_cpy_imm, &GPL191X::op_cmp_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_cpy_zp, &GPL191X::op_cmp_zp, &GPL191X::op_dec_zp, &GPL191X::op_dummy,
    &GPL191X::op_iny, &GPL191X::op_cmp_imm, &GPL191X::op_dex, &GPL191X::op_dummy,
    &GPL191X::op_cpy_abs, &GPL191X::op_cmp_abs, &GPL191X::op_dec_abs, &GPL191X::op_dummy,
    &GPL191X::op_bne, &GPL191X::op_cmp_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_cmp_zp_x, &GPL191X::op_dec_zp_x, &GPL191X::op_dummy,
    &GPL191X::op_cld, &GPL191X::op_cmp_abs_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_cmp_abs_x, &GPL191X::op_dec_abs_x, &GPL191X::op_dummy,
    &GPL191X::op_cpx_imm, &GPL191X::op_sbc_ind_x, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_cpx_zp, &GPL191X::op_sbc_zp, &GPL191X::op_inc_zp, &GPL191X::op_dummy,
    &GPL191X::op_inx, &GPL191X::op_sbc_imm, &GPL191X::op_nop, &GPL191X::op_dummy,
    &GPL191X::op_cpx_abs, &GPL191X::op_sbc_abs, &GPL191X::op_inc_abs, &GPL191X::op_dummy,
    &GPL191X::op_beq, &GPL191X::op_sbc_ind_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_sbc_zp_x, &GPL191X::op_inc_zp_x, &GPL191X::op_dummy,
    &GPL191X::op_sed, &GPL191X::op_sbc_abs_y, &GPL191X::op_dummy, &GPL191X::op_dummy,
    &GPL191X::op_dummy, &GPL191X::op_sbc_abs_x, &GPL191X::op_inc_abs_x, &GPL191X::op_dummy,
};
