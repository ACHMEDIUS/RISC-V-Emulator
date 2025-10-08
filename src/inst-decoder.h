/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    inst-decoder.h - RISC-V instruction decoder.
 *
 * Copyright (C) 2016,2019  Leiden University, The Netherlands.
 */

#ifndef __INST_DECODER_H__
#define __INST_DECODER_H__

#include "reg-file.h"

#include <stdexcept>
#include <cstdint>


/* Instruction types based on encoding format */
enum class InstructionType {
  R_TYPE,
  I_TYPE,
  S_TYPE,
  B_TYPE,
  U_TYPE,
  J_TYPE
};

/* Opcodes for RV64I */
enum class Opcode : uint8_t {
  OP       = 0x33,  /* R-type: add, sub, sll, slt, sltu, xor, srl, sra, or, and */
  OP_IMM   = 0x13,  /* I-type: addi, slti, sltiu, xori, ori, andi, slli, srli, srai */
  OP_32    = 0x3B,  /* R-type 32-bit: addw, subw, sllw, srlw, sraw */
  OP_IMM_32= 0x1B,  /* I-type 32-bit: addiw, slliw, srliw, sraiw */
  LOAD     = 0x03,  /* I-type: lb, lh, lw, ld, lbu, lhu, lwu */
  STORE    = 0x23,  /* S-type: sb, sh, sw, sd */
  BRANCH   = 0x63,  /* B-type: beq, bne, blt, bge, bltu, bgeu */
  JALR     = 0x67,  /* I-type: jalr */
  JAL      = 0x6F,  /* J-type: jal */
  LUI      = 0x37,  /* U-type: lui */
  AUIPC    = 0x17   /* U-type: auipc */
};


/* Exception that should be thrown when an illegal instruction
 * is encountered.
 */
class IllegalInstruction : public std::runtime_error
{
  public:
    explicit IllegalInstruction(const std::string &what)
      : std::runtime_error(what)
    { }

    explicit IllegalInstruction(const char *what)
      : std::runtime_error(what)
    { }
};


/* InstructionDecoder component to be used by class Processor */
class InstructionDecoder
{
  public:
    void                setInstructionWord(const uint32_t instructionWord);
    uint32_t            getInstructionWord() const;

    RegNumber           getRS1() const;
    RegNumber           getRS2() const;
    RegNumber           getRD() const;

    Opcode              getOpcode() const;
    uint8_t             getFunct3() const;
    uint8_t             getFunct7() const;

    InstructionType     getInstructionType() const;

    int64_t             getImmediate() const;
    int64_t             getImmediateI() const;
    int64_t             getImmediateS() const;
    int64_t             getImmediateB() const;
    int64_t             getImmediateU() const;
    int64_t             getImmediateJ() const;

  private:
    uint32_t instructionWord;
};

std::ostream &operator<<(std::ostream &os, const InstructionDecoder &decoder);

#endif /* __INST_DECODER_H__ */
