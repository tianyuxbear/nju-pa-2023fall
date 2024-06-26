/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write


enum {
  TYPE_R,
  TYPE_I, 
  TYPE_S,
  TYPE_B,
  TYPE_U,
  TYPE_J,
  TYPE_CL,
  TYPE_CB,
  TYPE_CI,
  TYPE_CJ,
  TYPE_N // none
};


#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)

#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)

#define immS() do { *imm = SEXT((BITS(i, 31, 25)<< 5 | \
                                 BITS(i, 11, 7)), 12); } while(0)

#define immB() do { *imm = SEXT((BITS(i, 31, 31) << 12 | \
                                 BITS(i, 7, 7) << 11   | \
                                 BITS(i, 30, 25) << 5  | \
                                 BITS(i, 11, 8) << 1), 13); } while(0)
                                
#define immU() do { *imm = SEXT(BITS(i, 31, 12) << 12, 32); } while(0)
#define immJ() do { *imm = SEXT((BITS(i, 31, 31) << 20 | \
                                 BITS(i, 19, 12) << 12 | \
                                 BITS(i, 20, 20) << 11 | \
                                 BITS(i, 30, 21) << 1), 21); } while(0)

#define immCL() do { *imm = BITS(i, 3, 2) << 4   | \
                           BITS(i, 12, 12) << 3 | \
                           BITS(i, 6, 4);} while(0)

#define immCB() do { *imm = SEXT((BITS(i, 12, 12) << 8 | \
                                  BITS(i, 6, 5) << 6   | \
                                  BITS(i, 2, 2) << 5   | \
                                  BITS(i, 11, 10) << 3 | \
                                  BITS(i, 4, 3) << 1), 9);} while(0)

#define immCI() do { *imm = BITS(i, 12, 12) << 5 | \
                            BITS(i, 6, 2);} while(0)

#define immCJ() do { *imm = SEXT(BITS(i, 12, 12) << 11  | \
                                 BITS(i, 8, 8) << 10    | \
                                 BITS(i, 10, 9) << 8    | \
                                 BITS(i, 6, 6) << 7     | \
                                 BITS(i, 7, 7) << 6     | \
                                 BITS(i, 2, 2) << 5     | \
                                 BITS(i, 11, 11) << 4   | \
                                 BITS(i, 5, 3) << 1 , 12); } while(0)


#define ETRACE_BUF_SIZE 128
static char etrace_buf[ETRACE_BUF_SIZE];

#define ECALL(dnpc) do{  bool success; \
                         word_t NO = isa_reg_str2val("$a7", &success); \
                         dnpc = isa_raise_intr(NO, s->pc); \
                         memset(etrace_buf, 0, ETRACE_BUF_SIZE); \
                         snprintf(etrace_buf, ETRACE_BUF_SIZE, "ecall occurs ==> pc: 0x%016lx, No: 0x%016lx", s->pc, NO); \
                         IFDEF(CONFIG_ETRACE, puts(etrace_buf)); \
                                    } while(0)


#define CSR_MEPC 0x341

static word_t* csr_addr(word_t imm){
  switch (imm)
  {
  case 0x180:
    return &cpu.csr.satp;
    break;
  case 0x300:
    return &cpu.csr.mstatus;
    break;
  case 0x342:
    return &cpu.csr.mcause;
    break;
  case 0x305:
    return &cpu.csr.mtvec;
    break;
  case 0x341:
    return &cpu.csr.mepc;
    break;
  default:
    panic("unknown csr register");
    break;
  }
}

#define CSR(i)  *csr_addr(i)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_I: src1R();          immI(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_CL:                  immCL(); break;
    case TYPE_CB: rs1 = BITS(i, 9, 7);   src1R();
                                   immCB(); break;
    case TYPE_CI: *rd = BITS(i, 9, 7);  
                                   immCI(); break;
    case TYPE_CJ:                  immCJ(); break;
    case TYPE_N:                           break;
    default:                    assert(0); break;
  }
}


void check_jal(word_t pc, word_t dnpc);
void check_jalr(word_t pc, word_t dnpc, int rd, int rs1, int offset);

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();

  // ===================================  RV32I instrcuctions ===================================================
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(rd) = s->pc + 4; s->dnpc = s->pc + imm; check_jal(s->pc, s->dnpc));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, s->dnpc = (src1 + imm) & ~(word_t)1; R(rd) = s->pc + 4; check_jalr(s->pc, s->dnpc, rd, BITS(s->isa.inst.val, 19, 15), imm));

  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, s->dnpc = (src1 == src2 ? s->pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, s->dnpc = (src1 == src2 ? s->snpc : s->pc + imm));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, s->dnpc = ((sword_t)src1 < (sword_t)src2 ? s->pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, s->dnpc = ((sword_t)src1 >= (sword_t)src2 ? s->pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, s->dnpc = (src1 < src2 ? s->pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, s->dnpc = (src1 >= src2 ? s->pc + imm : s->snpc));

  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = SEXT(Mr(src1 + imm, 4), 32));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));

  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));

  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + imm);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = (sword_t)src1 < (sword_t)imm);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = (src1 < imm));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);
  INSTPAT("000000? ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << BITS(imm, 5, 0));
  INSTPAT("000000? ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = src1 >> BITS(imm, 5, 0));
  INSTPAT("010000? ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = (sword_t)src1 >> BITS(imm, 5, 0));

  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 << BITS(src2, 5, 0));
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = ((sword_t)src1 < (sword_t)src2));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = (src1 < src2));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1 >> BITS(src2, 5, 0));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (sword_t)src1 >> BITS(src2, 5, 0));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2);

  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , I, ECALL(s->dnpc)); // R(10) is $a0
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , I, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I, word_t t = CSR(imm); CSR(imm) = src1; R(rd) = t); 
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, word_t t = CSR(imm); CSR(imm) = t | src1; R(rd) = t); 
  INSTPAT("??????? ????? ????? 011 ????? 11100 11", csrrc  , I, word_t t = CSR(imm); CSR(imm) = t & ~src1; R(rd) = t); 


  // =========================================== RV32M instructions =============================================================
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = src1 * src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(rd) = ((__int128_t)src1 * (__int128_t)src2) >> 64);
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, R(rd) = (__int128_t)((__int128_t)src1 * (__uint128_t)src2) >> 64);
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, R(rd) = ((__uint128_t)src1 * (__uint128_t)src2) >> 64);
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (sword_t)src1 / (sword_t)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = src1 / src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (sword_t)src1 % (sword_t)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = src1 % src2);

  // ===========================================  RV64I instructions(RV64I only) =================================================
  INSTPAT("??????? ????? ????? 110 ????? 00000 11", lwu    , I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld     , I, R(rd) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd     , S, Mw(src1 + imm, 8, src2));

  INSTPAT("??????? ????? ????? 000 ????? 00110 11", addiw  , I, R(rd) = SEXT(src1 + imm, 32));
  INSTPAT("0000000 ????? ????? 001 ????? 00110 11", slliw  , I, R(rd) = SEXT(src1 << imm, 32));
  INSTPAT("0000000 ????? ????? 101 ????? 00110 11", srliw  , I, R(rd) = SEXT(BITS(src1,31,0) >> imm, 32));
  INSTPAT("0100000 ????? ????? 101 ????? 00110 11", sraiw  , I, R(rd) = (sword_t)SEXT(src1, 32) >> BITS(imm, 4, 0));

  INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw   , R, R(rd) = SEXT(src1 + src2, 32));
  INSTPAT("0100000 ????? ????? 000 ????? 01110 11", subw   , R, R(rd) = SEXT(src1 - src2, 32));
  INSTPAT("0000000 ????? ????? 001 ????? 01110 11", sllw   , R, R(rd) = SEXT(BITS(src1, 31, 0) << BITS(src2, 4, 0), 32));
  INSTPAT("0000000 ????? ????? 101 ????? 01110 11", srlw   , R, R(rd) = SEXT(BITS(src1, 31, 0) >> BITS(src2, 4, 0), 32));
  INSTPAT("0100000 ????? ????? 101 ????? 01110 11", sraw   , R, R(rd) = (sword_t)SEXT(src1, 32) >> BITS(src2, 4, 0));

  // =========================================== RV64M instructions(RV64M only) =================================================
  INSTPAT("0000001 ????? ????? 000 ????? 01110 11", mulw   , R, R(rd) = SEXT(src1 * src2, 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01110 11", divw   , R, R(rd) = (sword_t)SEXT(src1, 32) / (sword_t)SEXT(src2, 32));
  INSTPAT("0000001 ????? ????? 101 ????? 01110 11", divuw  , R, R(rd) = SEXT(BITS(src1, 31, 0) / BITS(src2, 31, 0), 32));
  INSTPAT("0000001 ????? ????? 110 ????? 01110 11", remw   , R, R(rd) = (sword_t)SEXT(src1, 32) % (sword_t)SEXT(src2, 32));
  INSTPAT("0000001 ????? ????? 111 ????? 01110 11", remuw  , R, R(rd) = SEXT(BITS(src1, 31, 0) % BITS(src2, 31, 0), 32));


  //============================================ RV32/64 privileged instructions ================================================
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , R, s->dnpc = CSR(CSR_MEPC) + 4);

  //============================================ special instructions(nemu use) =================================================
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));

  
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
