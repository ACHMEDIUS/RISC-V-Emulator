/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    inst-formatter.cc - RISC-V instruction printer (disassembler)
 *
 * Copyright (C) 2016,2018  Leiden University, The Netherlands.
 */

#include "inst-decoder.h"

#include <iostream>
#include <string>

namespace {

std::string
formatRegister(RegNumber reg)
{
  return "r" + std::to_string(static_cast<unsigned>(reg));
}

std::string
formatImmediate(int64_t value)
{
  return "$" + std::to_string(value);
}

inline uint16_t
low16(uint32_t word)
{
  return static_cast<uint16_t>(word & 0xFFFF);
}

inline int64_t
signExtend(uint32_t value, unsigned bits)
{
  const uint32_t mask = 1U << (bits - 1);
  return static_cast<int64_t>((value ^ mask) - mask);
}

void
emitBinaryOp(std::ostream& os, const char* mnemonic, RegNumber rd,
             RegNumber rs1, RegNumber rs2)
{
  os << mnemonic << " " << formatRegister(rd) << ", " << formatRegister(rs1)
     << ", " << formatRegister(rs2);
}

void
emitUnaryOp(std::ostream& os, const char* mnemonic, RegNumber rd, RegNumber rs1,
            int64_t imm)
{
  os << mnemonic << " " << formatRegister(rd) << ", " << formatRegister(rs1)
     << ", " << formatImmediate(imm);
}

void
emitLoad(std::ostream& os, const char* mnemonic, RegNumber rd, RegNumber rs1,
         int64_t imm)
{
  os << mnemonic << " " << formatRegister(rd) << ", " << formatImmediate(imm)
     << "(" << formatRegister(rs1) << ")";
}

void
emitStore(std::ostream& os, const char* mnemonic, RegNumber rs2, RegNumber rs1,
          int64_t imm)
{
  os << mnemonic << " " << formatRegister(rs2) << ", " << formatImmediate(imm)
     << "(" << formatRegister(rs1) << ")";
}

void
formatCompressedInstruction(std::ostream& os, uint16_t inst)
{
  const uint8_t quadrant = inst & 0x3;
  const uint8_t funct3 = (inst >> 13) & 0x7;

  switch (quadrant) {
  case 0x0:            /* Quadrant 0 */
    if (funct3 == 0x0) /* C.ADDI4SPN */
    {
      const uint8_t rdPrime = (inst >> 2) & 0x7;
      const RegNumber rd = static_cast<RegNumber>(rdPrime + 8);
      uint32_t imm = 0;
      imm |= ((inst >> 11) & 0x3) << 4; /* nzuimm[5:4] */
      imm |= ((inst >> 7) & 0xF) << 6;  /* nzuimm[9:6] */
      imm |= ((inst >> 6) & 0x1) << 2;  /* nzuimm[2] */
      imm |= ((inst >> 5) & 0x1) << 3;  /* nzuimm[3] */

      os << "addi " << formatRegister(rd) << ", " << formatRegister(2) << ", "
         << formatImmediate(static_cast<int64_t>(imm));
    } else
      throw IllegalInstruction("Unsupported compressed instruction");
    break;

  case 0x1:            /* Quadrant 1 */
    if (funct3 == 0x1) /* C.ADDIW */
    {
      const RegNumber rd = static_cast<RegNumber>((inst >> 7) & 0x1F);
      uint32_t imm = ((inst >> 12) & 0x1) << 5 | ((inst >> 2) & 0x1F);
      os << "addiw " << formatRegister(rd) << ", " << formatRegister(rd) << ", "
         << formatImmediate(signExtend(imm, 6));
    } else
      throw IllegalInstruction("Unsupported compressed instruction");
    break;

  case 0x2:            /* Quadrant 2 */
    if (funct3 == 0x0) /* C.SLLI */
    {
      const RegNumber rd = static_cast<RegNumber>((inst >> 7) & 0x1F);
      uint32_t shamt = ((inst >> 12) & 0x1) << 5 | ((inst >> 2) & 0x1F);
      os << "slli " << formatRegister(rd) << ", " << formatRegister(rd) << ", "
         << formatImmediate(static_cast<int64_t>(shamt));
    } else
      throw IllegalInstruction("Unsupported compressed instruction");
    break;

  default:
    throw IllegalInstruction("Unsupported compressed instruction");
  }

  os << "  \t(compressed)";
}

void
formatOpType(std::ostream& os, const InstructionDecoder& decoder)
{
  const uint8_t funct3 = decoder.getFunct3();
  const uint8_t funct7 = decoder.getFunct7();
  const RegNumber rd = decoder.getRD();
  const RegNumber rs1 = decoder.getRS1();
  const RegNumber rs2 = decoder.getRS2();

  switch (funct3) {
  case 0x0:
    if (funct7 == 0x00)
      emitBinaryOp(os, "add", rd, rs1, rs2);
    else if (funct7 == 0x20)
      emitBinaryOp(os, "sub", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x1:
    if (funct7 == 0x00)
      emitBinaryOp(os, "sll", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x2:
    if (funct7 == 0x00)
      emitBinaryOp(os, "slt", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x3:
    if (funct7 == 0x00)
      emitBinaryOp(os, "sltu", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x4:
    if (funct7 == 0x00)
      emitBinaryOp(os, "xor", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x5:
    if (funct7 == 0x00)
      emitBinaryOp(os, "srl", rd, rs1, rs2);
    else if (funct7 == 0x20)
      emitBinaryOp(os, "sra", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x6:
    if (funct7 == 0x00)
      emitBinaryOp(os, "or", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  case 0x7:
    if (funct7 == 0x00)
      emitBinaryOp(os, "and", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown R-type instruction");
    break;

  default:
    throw IllegalInstruction("Unknown R-type instruction");
  }
}

void
formatOp32Type(std::ostream& os, const InstructionDecoder& decoder)
{
  const uint8_t funct3 = decoder.getFunct3();
  const uint8_t funct7 = decoder.getFunct7();
  const RegNumber rd = decoder.getRD();
  const RegNumber rs1 = decoder.getRS1();
  const RegNumber rs2 = decoder.getRS2();

  switch (funct3) {
  case 0x0:
    if (funct7 == 0x00)
      emitBinaryOp(os, "addw", rd, rs1, rs2);
    else if (funct7 == 0x20)
      emitBinaryOp(os, "subw", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown RV64 R-type instruction");
    break;

  case 0x1:
    if (funct7 == 0x00)
      emitBinaryOp(os, "sllw", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown RV64 R-type instruction");
    break;

  case 0x5:
    if (funct7 == 0x00)
      emitBinaryOp(os, "srlw", rd, rs1, rs2);
    else if (funct7 == 0x20)
      emitBinaryOp(os, "sraw", rd, rs1, rs2);
    else
      throw IllegalInstruction("Unknown RV64 R-type instruction");
    break;

  default:
    throw IllegalInstruction("Unknown RV64 R-type instruction");
  }
}

void
formatOpImm(std::ostream& os, const InstructionDecoder& decoder)
{
  const uint8_t funct3 = decoder.getFunct3();
  const uint8_t funct7 = decoder.getFunct7();
  const RegNumber rd = decoder.getRD();
  const RegNumber rs1 = decoder.getRS1();
  const int64_t imm = decoder.getImmediateI();

  switch (funct3) {
  case 0x0:
    emitUnaryOp(os, "addi", rd, rs1, imm);
    break;

  case 0x2:
    emitUnaryOp(os, "slti", rd, rs1, imm);
    break;

  case 0x3:
    emitUnaryOp(os, "sltiu", rd, rs1, imm);
    break;

  case 0x4:
    emitUnaryOp(os, "xori", rd, rs1, imm);
    break;

  case 0x6:
    emitUnaryOp(os, "ori", rd, rs1, imm);
    break;

  case 0x7:
    emitUnaryOp(os, "andi", rd, rs1, imm);
    break;

  case 0x1:
    if (funct7 == 0x00)
      emitUnaryOp(os, "slli", rd, rs1, imm & 0x3F);
    else
      throw IllegalInstruction("Unknown shift immediate");
    break;

  case 0x5:
    if (funct7 == 0x00)
      emitUnaryOp(os, "srli", rd, rs1, imm & 0x3F);
    else if (funct7 == 0x20)
      emitUnaryOp(os, "srai", rd, rs1, imm & 0x3F);
    else
      throw IllegalInstruction("Unknown shift immediate");
    break;

  default:
    throw IllegalInstruction("Unknown immediate instruction");
  }
}

void
formatOpImm32(std::ostream& os, const InstructionDecoder& decoder)
{
  const uint8_t funct3 = decoder.getFunct3();
  const uint8_t funct7 = decoder.getFunct7();
  const RegNumber rd = decoder.getRD();
  const RegNumber rs1 = decoder.getRS1();
  const int64_t imm = decoder.getImmediateI();

  switch (funct3) {
  case 0x0:
    emitUnaryOp(os, "addiw", rd, rs1, imm);
    break;

  case 0x1:
    if (funct7 == 0x00)
      emitUnaryOp(os, "slliw", rd, rs1, imm & 0x1F);
    else
      throw IllegalInstruction("Unknown RV64 shift immediate");
    break;

  case 0x5:
    if (funct7 == 0x00)
      emitUnaryOp(os, "srliw", rd, rs1, imm & 0x1F);
    else if (funct7 == 0x20)
      emitUnaryOp(os, "sraiw", rd, rs1, imm & 0x1F);
    else
      throw IllegalInstruction("Unknown RV64 shift immediate");
    break;

  default:
    throw IllegalInstruction("Unknown RV64 immediate instruction");
  }
}

} // namespace

std::ostream&
operator<<(std::ostream& os, const InstructionDecoder& decoder)
{
  try {
    const uint32_t word = decoder.getInstructionWord();

    if ((word & 0x3) != 0x3) {
      formatCompressedInstruction(os, low16(word));
      return os;
    }

    const Opcode opcode = decoder.getOpcode();
    const uint8_t funct3 = decoder.getFunct3();
    const RegNumber rd = decoder.getRD();
    const RegNumber rs1 = decoder.getRS1();
    const RegNumber rs2 = decoder.getRS2();

    switch (opcode) {
    case Opcode::OP:
      formatOpType(os, decoder);
      break;

    case Opcode::OP_IMM:
      formatOpImm(os, decoder);
      break;

    case Opcode::OP_32:
      formatOp32Type(os, decoder);
      break;

    case Opcode::OP_IMM_32:
      formatOpImm32(os, decoder);
      break;

    case Opcode::LOAD:
      switch (funct3) {
      case 0x0:
        emitLoad(os, "lb", rd, rs1, decoder.getImmediateI());
        break;
      case 0x1:
        emitLoad(os, "lh", rd, rs1, decoder.getImmediateI());
        break;
      case 0x2:
        emitLoad(os, "lw", rd, rs1, decoder.getImmediateI());
        break;
      case 0x3:
        emitLoad(os, "ld", rd, rs1, decoder.getImmediateI());
        break;
      case 0x4:
        emitLoad(os, "lbu", rd, rs1, decoder.getImmediateI());
        break;
      case 0x5:
        emitLoad(os, "lhu", rd, rs1, decoder.getImmediateI());
        break;
      case 0x6:
        emitLoad(os, "lwu", rd, rs1, decoder.getImmediateI());
        break;
      default:
        throw IllegalInstruction("Unknown load");
      }
      break;

    case Opcode::STORE:
      switch (funct3) {
      case 0x0:
        emitStore(os, "sb", rs2, rs1, decoder.getImmediateS());
        break;
      case 0x1:
        emitStore(os, "sh", rs2, rs1, decoder.getImmediateS());
        break;
      case 0x2:
        emitStore(os, "sw", rs2, rs1, decoder.getImmediateS());
        break;
      case 0x3:
        emitStore(os, "sd", rs2, rs1, decoder.getImmediateS());
        break;
      default:
        throw IllegalInstruction("Unknown store");
      }
      break;

    case Opcode::BRANCH: {
      const int64_t imm = decoder.getImmediateB();

      switch (funct3) {
      case 0x0:
        os << "beq " << formatRegister(rs1) << ", " << formatRegister(rs2)
           << ", " << formatImmediate(imm);
        break;
      case 0x1:
        os << "bne " << formatRegister(rs1) << ", " << formatRegister(rs2)
           << ", " << formatImmediate(imm);
        break;
      case 0x4:
        os << "blt " << formatRegister(rs1) << ", " << formatRegister(rs2)
           << ", " << formatImmediate(imm);
        break;
      case 0x5:
        os << "bge " << formatRegister(rs1) << ", " << formatRegister(rs2)
           << ", " << formatImmediate(imm);
        break;
      case 0x6:
        os << "bltu " << formatRegister(rs1) << ", " << formatRegister(rs2)
           << ", " << formatImmediate(imm);
        break;
      case 0x7:
        os << "bgeu " << formatRegister(rs1) << ", " << formatRegister(rs2)
           << ", " << formatImmediate(imm);
        break;
      default:
        throw IllegalInstruction("Unknown branch");
      }
    } break;

    case Opcode::JALR:
      os << "jalr " << formatRegister(rd) << ", "
         << formatImmediate(decoder.getImmediateI()) << "("
         << formatRegister(rs1) << ")";
      break;

    case Opcode::JAL:
      os << "jal " << formatRegister(rd) << ", "
         << formatImmediate(decoder.getImmediateJ());
      break;

    case Opcode::LUI:
      os << "lui " << formatRegister(rd) << ", "
         << formatImmediate(decoder.getImmediateU() >> 12);
      break;

    case Opcode::AUIPC:
      os << "auipc " << formatRegister(rd) << ", "
         << formatImmediate(decoder.getImmediateU() >> 12);
      break;

    default:
      throw IllegalInstruction("Unknown opcode");
    }
  } catch (const IllegalInstruction&) {
    os << "illegal instruction";
  }

  return os;
}
