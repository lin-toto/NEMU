#ifdef CONFIG_RV_ZIMTE

static inline def_DHelper(rvzimte) {
    decode_op_r(s, id_src1, s->isa.instr.r.rs1, true);
    decode_op_i(s, id_src2, (sword_t)(s->isa.instr.r.rs2 >> 1), false);
    decode_op_r(s, id_dest, s->isa.instr.r.rd, false);
}

def_THelper(rvzimte) {
    def_INSTR_IDTAB("1000011 0 0000 00000 100 ????? 1110011", rvzimte, mte_gentag);
    def_INSTR_IDTAB("1000011 0 ???? ????? 100 ????? 1110011", rvzimte, mte_addtag);
    def_INSTR_IDTAB("1000001 0 ???? ????? 100 00000 1110011", rvzimte, mte_settag);
    // Custom instruction not really specified in Zimte
    def_INSTR_IDTAB("1000101 0 0000 ????? 100 ????? 1110011", rvzimte, mte_gettag);
    return EXEC_ID_inv;
}

#endif