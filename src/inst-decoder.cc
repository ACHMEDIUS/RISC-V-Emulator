/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    inst-decoder.cc - RISC-V instruction decoder.
 *
 * Copyright (C) 2016,2019  Leiden University, The Netherlands.
 *
 */

#include "inst-decoder.h"

#include <map>


/*
 * Class InstructionDecoder -- helper class for getting specific
 * information from the decoded instruction.
 */

namespace {

inline int64_t
signExtend(uint64_t value, unsigned bits)
{
  const uint64_t mask = 1ULL << (bits - 1);
  return static_cast<int64_t>((value ^ mask) - mask);
}

}

void
InstructionDecoder::setInstructionWord(const uint32_t instructionWord)
{
  this->instructionWord = instructionWord;
}

uint32_t
InstructionDecoder::getInstructionWord() const
{
  return instructionWord;
}

Opcode
InstructionDecoder::getOpcode() const
{
  uint8_t opcode = instructionWord & 0x7F;
  return static_cast<Opcode>(opcode);
}

RegNumber
InstructionDecoder::getRS1() const
{
  return (instructionWord >> 15) & 0x1F;
}

RegNumber
InstructionDecoder::getRS2() const
{
  return (instructionWord >> 20) & 0x1F;
}

RegNumber
InstructionDecoder::getRD() const
{
  return (instructionWord >> 7) & 0x1F;
}

uint8_t
InstructionDecoder::getFunct3() const
{
  return (instructionWord >> 12) & 0x07;
}

uint8_t
InstructionDecoder::getFunct7() const
{
  return (instructionWord >> 25) & 0x7F;
}

InstructionType
InstructionDecoder::getInstructionType() const
{
  Opcode opcode = getOpcode();

  switch (opcode)
    {
      case Opcode::OP:
      case Opcode::OP_32:
        return InstructionType::R_TYPE;

      case Opcode::OP_IMM:
      case Opcode::OP_IMM_32:
      case Opcode::LOAD:
      case Opcode::JALR:
        return InstructionType::I_TYPE;

      case Opcode::STORE:
        return InstructionType::S_TYPE;

      case Opcode::BRANCH:
        return InstructionType::B_TYPE;

      case Opcode::LUI:
      case Opcode::AUIPC:
        return InstructionType::U_TYPE;

      case Opcode::JAL:
        return InstructionType::J_TYPE;

      default:
        throw IllegalInstruction("Unknown opcode");
    }
}

int64_t
InstructionDecoder::getImmediateI() const
{
  /* I-type: imm[11:0] in bits [31:20] */
  const uint64_t imm = (instructionWord >> 20) & 0xFFF;
  return signExtend(imm, 12);
}

int64_t
InstructionDecoder::getImmediateS() const
{
  /* S-type: imm[11:5] in bits [31:25], imm[4:0] in bits [11:7] */
  const uint64_t imm =
    ((instructionWord >> 25) & 0x7F) << 5 |
    ((instructionWord >> 7) & 0x1F);
  return signExtend(imm, 12);
}

int64_t
InstructionDecoder::getImmediateB() const
{
  /* B-type: imm[12|10:5|4:1|11|0] */
  const uint64_t imm =
    ((instructionWord >> 31) & 0x1) << 12 |
    ((instructionWord >> 7)  & 0x1) << 11 |
    ((instructionWord >> 25) & 0x3F) << 5 |
    ((instructionWord >> 8)  & 0xF) << 1;
  return signExtend(imm, 13);
}

int64_t
InstructionDecoder::getImmediateU() const
{
  /* U-type: imm[31:12] in bits [31:12], left-shifted by 12 */
  return static_cast<int64_t>(instructionWord & 0xFFFFF000);
}

int64_t
InstructionDecoder::getImmediateJ() const
{
  /* J-type: imm[20|10:1|11|19:12|0] */
  const uint64_t imm =
    ((instructionWord >> 31) & 0x1) << 20 |
    ((instructionWord >> 21) & 0x3FF) << 1 |
    ((instructionWord >> 20) & 0x1) << 11 |
    ((instructionWord >> 12) & 0xFF) << 12;
  return signExtend(imm, 21);
}

int64_t
InstructionDecoder::getImmediate() const
{
  InstructionType type = getInstructionType();

  switch (type)
    {
      case InstructionType::I_TYPE:
        return getImmediateI();
      case InstructionType::S_TYPE:
        return getImmediateS();
      case InstructionType::B_TYPE:
        return getImmediateB();
      case InstructionType::U_TYPE:
        return getImmediateU();
      case InstructionType::J_TYPE:
        return getImmediateJ();
      default:
        return 0;
    }
}
