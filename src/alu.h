/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    alu.h - ALU component.
 *
 * Copyright (C) 2016  Leiden University, The Netherlands.
 */

#ifndef __ALU_H__
#define __ALU_H__

#include "arch.h"
#include "inst-decoder.h"

#include <map>



enum class ALUOp {
    NOP,
    ADD,
    SUB,
    SLL,    /* Shift left logical */
    SLT,    /* Set less than (signed) */
    SLTU,   /* Set less than (unsigned) */
    XOR,
    SRL,    /* Shift right logical */
    SRA,    /* Shift right arithmetic */
    OR,
    AND,
    ADDW,   /* Add word (32-bit) */
    SUBW,   /* Sub word (32-bit) */
    SLLW,   /* Shift left logical word */
    SRLW,   /* Shift right logical word */
    SRAW    /* Shift right arithmetic word */
};


/* The ALU component performs the specified operation on operands A and B
 * when asked to propagate the result. The operation is specified through
 * the ALUOp.
 */
class ALU
{
  public:
    ALU();

    void setA(RegValue A) { this->A = A; }
    void setB(RegValue B) { this->B = B; }

    RegValue getResult();

    void setOp(ALUOp op) { this->op = op; }

  private:
    RegValue A;
    RegValue B;

    ALUOp op;
};

#endif /* __ALU_H__ */
