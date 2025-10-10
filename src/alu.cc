/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    alu.h - ALU component.
 *
 * Copyright (C) 2016,2018  Leiden University, The Netherlands.
 */

#include "alu.h"

#include "inst-decoder.h"

#ifdef _MSC_VER
/* MSVC intrinsics */
#include <intrin.h>
#endif

ALU::ALU() : A(), B(), op() {}

RegValue
ALU::getResult()
{
  RegValue result = 0;

  switch (op) {
  case ALUOp::NOP:
    result = 0;
    break;

  case ALUOp::ADD:
    result = A + B;
    break;

  case ALUOp::SUB:
    result = A - B;
    break;

  case ALUOp::SLL:
    result = A << (B & 0x3F); /* Shift amount is lower 6 bits for RV64 */
    break;

  case ALUOp::SLT:
    result = (static_cast<int64_t>(A) < static_cast<int64_t>(B)) ? 1 : 0;
    break;

  case ALUOp::SLTU:
    result = (A < B) ? 1 : 0;
    break;

  case ALUOp::XOR:
    result = A ^ B;
    break;

  case ALUOp::SRL:
    result = A >> (B & 0x3F); /* Logical shift */
    break;

  case ALUOp::SRA:
    result = static_cast<uint64_t>(static_cast<int64_t>(A) >>
                                   (B & 0x3F)); /* Arithmetic shift */
    break;

  case ALUOp::OR:
    result = A | B;
    break;

  case ALUOp::AND:
    result = A & B;
    break;

  case ALUOp::ADDW: {
    /* 32-bit add with sign extension */
    int32_t a32 = static_cast<int32_t>(A);
    int32_t b32 = static_cast<int32_t>(B);
    result = static_cast<int64_t>(a32 + b32);
  } break;

  case ALUOp::SUBW: {
    /* 32-bit subtract with sign extension */
    int32_t a32 = static_cast<int32_t>(A);
    int32_t b32 = static_cast<int32_t>(B);
    result = static_cast<int64_t>(a32 - b32);
  } break;

  case ALUOp::SLLW: {
    /* 32-bit shift left logical with sign extension */
    uint32_t a32 = static_cast<uint32_t>(A);
    uint32_t shamt = B & 0x1F; /* Lower 5 bits for 32-bit */
    result = static_cast<int64_t>(static_cast<int32_t>(a32 << shamt));
  } break;

  case ALUOp::SRLW: {
    /* 32-bit logical shift right with sign extension */
    uint32_t a32 = static_cast<uint32_t>(A);
    uint32_t shamt = B & 0x1F;
    result = static_cast<int64_t>(static_cast<int32_t>(a32 >> shamt));
  } break;

  case ALUOp::SRAW: {
    /* 32-bit arithmetic shift right with sign extension */
    int32_t a32 = static_cast<int32_t>(A);
    uint32_t shamt = B & 0x1F;
    result = static_cast<int64_t>(a32 >> shamt);
  } break;

  default:
    throw IllegalInstruction("Unimplemented or unknown ALU operation");
  }

  return result;
}
