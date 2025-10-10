/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    stages.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include "stages.h"

#include <iostream>

namespace {

bool
instructionUsesRS2(Opcode opcode)
{
  switch (opcode) {
  case Opcode::OP:
  case Opcode::OP_32:
  case Opcode::STORE:
  case Opcode::BRANCH:
    return true;
  default:
    return false;
  }
}

} // namespace

/*
 * Control Signals
 */

void
ControlSignals::setFromInstruction(const InstructionDecoder& decoder)
{
  Opcode opcode = decoder.getOpcode();
  uint8_t funct3 = decoder.getFunct3();
  uint8_t funct7 = decoder.getFunct7();

  /* Default values */
  regWrite = false;
  aluSrc = false;
  memRead = false;
  memWrite = false;
  memToReg = false;
  branch = false;
  jump = false;
  aluOp = ALUOp::NOP;
  memSize = 0;
  memSignExtend = false;

  switch (opcode) {
  case Opcode::OP: /* R-type ALU */
    regWrite = true;
    aluSrc = false;
    if (funct3 == 0x0 && funct7 == 0x00)
      aluOp = ALUOp::ADD;
    else if (funct3 == 0x0 && funct7 == 0x20)
      aluOp = ALUOp::SUB;
    else if (funct3 == 0x1 && funct7 == 0x00)
      aluOp = ALUOp::SLL;
    else if (funct3 == 0x2 && funct7 == 0x00)
      aluOp = ALUOp::SLT;
    else if (funct3 == 0x3 && funct7 == 0x00)
      aluOp = ALUOp::SLTU;
    else if (funct3 == 0x4 && funct7 == 0x00)
      aluOp = ALUOp::XOR;
    else if (funct3 == 0x5 && funct7 == 0x00)
      aluOp = ALUOp::SRL;
    else if (funct3 == 0x5 && funct7 == 0x20)
      aluOp = ALUOp::SRA;
    else if (funct3 == 0x6 && funct7 == 0x00)
      aluOp = ALUOp::OR;
    else if (funct3 == 0x7 && funct7 == 0x00)
      aluOp = ALUOp::AND;
    break;

  case Opcode::OP_IMM: /* I-type ALU */
    regWrite = true;
    aluSrc = true;
    if (funct3 == 0x0)
      aluOp = ALUOp::ADD;
    else if (funct3 == 0x2)
      aluOp = ALUOp::SLT;
    else if (funct3 == 0x3)
      aluOp = ALUOp::SLTU;
    else if (funct3 == 0x4)
      aluOp = ALUOp::XOR;
    else if (funct3 == 0x6)
      aluOp = ALUOp::OR;
    else if (funct3 == 0x7)
      aluOp = ALUOp::AND;
    else if (funct3 == 0x1 && funct7 == 0x00)
      aluOp = ALUOp::SLL;
    else if (funct3 == 0x5 && funct7 == 0x00)
      aluOp = ALUOp::SRL;
    else if (funct3 == 0x5 && funct7 == 0x20)
      aluOp = ALUOp::SRA;
    break;

  case Opcode::OP_32: /* R-type 32-bit */
    regWrite = true;
    aluSrc = false;
    if (funct3 == 0x0 && funct7 == 0x00)
      aluOp = ALUOp::ADDW;
    else if (funct3 == 0x0 && funct7 == 0x20)
      aluOp = ALUOp::SUBW;
    else if (funct3 == 0x1 && funct7 == 0x00)
      aluOp = ALUOp::SLLW;
    else if (funct3 == 0x5 && funct7 == 0x00)
      aluOp = ALUOp::SRLW;
    else if (funct3 == 0x5 && funct7 == 0x20)
      aluOp = ALUOp::SRAW;
    break;

  case Opcode::OP_IMM_32: /* I-type 32-bit */
    regWrite = true;
    aluSrc = true;
    if (funct3 == 0x0)
      aluOp = ALUOp::ADDW;
    else if (funct3 == 0x1 && funct7 == 0x00)
      aluOp = ALUOp::SLLW;
    else if (funct3 == 0x5 && funct7 == 0x00)
      aluOp = ALUOp::SRLW;
    else if (funct3 == 0x5 && funct7 == 0x20)
      aluOp = ALUOp::SRAW;
    break;

  case Opcode::LOAD:
    regWrite = true;
    aluSrc = true;
    memRead = true;
    memToReg = true;
    aluOp = ALUOp::ADD;
    if (funct3 == 0x0) {
      memSize = 1;
      memSignExtend = true;
    } /* lb */
    else if (funct3 == 0x1) {
      memSize = 2;
      memSignExtend = true;
    } /* lh */
    else if (funct3 == 0x2) {
      memSize = 4;
      memSignExtend = true;
    } /* lw */
    else if (funct3 == 0x3) {
      memSize = 8;
      memSignExtend = false;
    } /* ld */
    else if (funct3 == 0x4) {
      memSize = 1;
      memSignExtend = false;
    } /* lbu */
    else if (funct3 == 0x5) {
      memSize = 2;
      memSignExtend = false;
    } /* lhu */
    else if (funct3 == 0x6) {
      memSize = 4;
      memSignExtend = false;
    } /* lwu */
    break;

  case Opcode::STORE:
    aluSrc = true;
    memWrite = true;
    aluOp = ALUOp::ADD;
    if (funct3 == 0x0)
      memSize = 1; /* sb */
    else if (funct3 == 0x1)
      memSize = 2; /* sh */
    else if (funct3 == 0x2)
      memSize = 4; /* sw */
    else if (funct3 == 0x3)
      memSize = 8; /* sd */
    break;

  case Opcode::BRANCH:
    branch = true;
    aluSrc = false;
    aluOp = ALUOp::SUB;
    break;

  case Opcode::JAL:
    regWrite = true;
    jump = true;
    aluOp = ALUOp::ADD;
    aluSrc = true;
    break;

  case Opcode::JALR:
    regWrite = true;
    jump = true;
    aluOp = ALUOp::ADD;
    aluSrc = true;
    break;

  case Opcode::LUI:
    regWrite = true;
    aluSrc = true;
    aluOp = ALUOp::ADD; /* Will load immediate into rd */
    break;

  case Opcode::AUIPC:
    regWrite = true;
    aluSrc = true;
    aluOp = ALUOp::ADD; /* Add immediate to PC */
    break;

  default:
    /* Leave all as defaults (no-op) */
    break;
  }
}

/*
 * Instruction fetch
 */

void
InstructionFetchStage::propagate()
{
  if (endMarkerSeen) {
    fetchPC = PC;
    fetchedInstruction = NopInstruction;
    return;
  }

  try {
    /* Fetch instruction from memory at current PC */
    instructionMemory.setAddress(PC);
    instructionMemory.setSize(4); /* Instructions are 32 bits (4 bytes) */

    uint32_t instructionWord = instructionMemory.getValue();

    /* Check for test end marker */
    if (instructionWord == TestEndMarker) {
      if (pipelining) {
        endMarkerSeen = true;
        endMarkerCountdown = 5; /* drain remaining pipeline stages */
        endMarkerPC = PC;
        fetchPC = PC;
        fetchedInstruction = NopInstruction;
        control.flushFetch = true;
        return;
      }
      throw TestEndMarkerEncountered(PC);
    }

    fetchPC = PC;
    fetchedInstruction = instructionWord;
  } catch (TestEndMarkerEncountered& e) {
    throw;
  } catch (std::exception& e) {
    throw InstructionFetchFailure(PC);
  }
}

void
InstructionFetchStage::clockPulse()
{
  if (!pipelining) {
    if_id.PC = PC;
    if_id.instructionWord = fetchedInstruction;
    PC += 4;

    if (endMarkerSeen && endMarkerCountdown <= 0)
      throw TestEndMarkerEncountered(endMarkerPC);

    return;
  }

  bool flush = control.flushFetch;
  bool stall = control.stallFetch;

  if (flush) {
    if_id.PC = 0;
    if_id.instructionWord = NopInstruction;
  } else if (!stall && !endMarkerSeen) {
    if_id.PC = fetchPC;
    if_id.instructionWord = fetchedInstruction;
    PC += 4;
  }

  if (endMarkerSeen) {
    if (endMarkerCountdown > 0) {
      --endMarkerCountdown;
    } else {
      throw TestEndMarkerEncountered(endMarkerPC);
    }
  }
}

/*
 * Instruction decode
 */

void dump_instruction(std::ostream& os, const uint32_t instructionWord,
                      const InstructionDecoder& decoder);

void
InstructionDecodeStage::propagate()
{
  PC = if_id.PC;
  instructionWord = if_id.instructionWord;

  /* Decode the instruction */
  decoder.setInstructionWord(instructionWord);

  /* Generate control signals from decoded instruction */
  decodedControl.setFromInstruction(decoder);

  /* debug mode: dump decoded instructions to cerr.
   * In case of no pipelining: always dump.
   * In case of pipelining: special case, if the PC == 0x0 (so on the
   * first cycle), don't dump an instruction. This avoids dumping a
   * dummy instruction on the first cycle when ID is effectively running
   * uninitialized.
   */
  if (debugMode && (!pipelining || (pipelining && PC != 0x0))) {
    /* Dump program counter & decoded instruction in debug mode */
    auto storeFlags(std::cerr.flags());

    std::cerr << std::hex << std::showbase << PC << "\t";
    std::cerr.setf(storeFlags);

    std::cerr << decoder << std::endl;
  }

  /* Register fetch: read from register file */
  regfile.setRS1(decoder.getRS1());
  regfile.setRS2(decoder.getRS2());

  /* Get register values (combinational, so can read immediately) */
  readData1 = regfile.getReadData1();
  readData2 = regfile.getReadData2();

  if (pipelining) {
    /* Forward results that are about to be written back so decode sees
     * the most recent register values even though the register file
     * update happens later in the cycle. */
    if (m_wb.control.getRegWrite() && m_wb.rd != 0) {
      RegValue wbValue =
          m_wb.control.getMemToReg() ? m_wb.memData : m_wb.aluResult;

      if (m_wb.rd == decoder.getRS1())
        readData1 = wbValue;

      if (instructionUsesRS2(decoder.getOpcode()) &&
          m_wb.rd == decoder.getRS2())
        readData2 = wbValue;
    }

    bool hazard = false;

    if (id_ex.control.getMemRead() && id_ex.rd != 0) {
      if (id_ex.rd == decoder.getRS1())
        hazard = true;
      else if (instructionUsesRS2(decoder.getOpcode()) &&
               id_ex.rd == decoder.getRS2())
        hazard = true;
    }

    if (hazard) {
      control.stallFetch = true;
      control.insertDecodeBubble = true;
    }
  }
}

void
InstructionDecodeStage::clockPulse()
{
  if (pipelining) {
    if (control.flushDecode) {
      id_ex = {};
      id_ex.control = ControlSignals();
      id_ex.opcode = Opcode::OP;
      id_ex.funct3 = 0;
      return;
    }

    if (control.insertDecodeBubble) {
      ++nStalls;
      id_ex = {};
      id_ex.control = ControlSignals();
      id_ex.opcode = Opcode::OP;
      id_ex.funct3 = 0;
      return;
    }
  }

  /* ignore the "instruction" in the first cycle. */
  if (!pipelining || (pipelining && PC != 0x0))
    ++nInstrIssued;

  /* Write to pipeline register */
  id_ex.PC = PC;
  id_ex.readData1 = readData1;
  id_ex.readData2 = readData2;
  id_ex.immediate = decoder.getImmediate();
  id_ex.rd = decoder.getRD();
  id_ex.rs1 = decoder.getRS1();
  id_ex.rs2 = decoder.getRS2();
  id_ex.opcode = decoder.getOpcode();
  id_ex.funct3 = decoder.getFunct3();
  id_ex.control = decodedControl;
}

/*
 * Execute
 */

void
ExecuteStage::propagate()
{
  PC = id_ex.PC;

  pcWriteEnable = false;
  nextPC = 0;

  RegValue rs1Value = id_ex.readData1;
  RegValue rs2Value = id_ex.readData2;

  if (pipelining) {
    bool exStageCanForward = ex_m.control.getRegWrite() &&
                             !ex_m.control.getMemToReg() && ex_m.rd != 0;

    if (exStageCanForward && ex_m.rd == id_ex.rs1)
      rs1Value = ex_m.aluResult;

    if (exStageCanForward && ex_m.rd == id_ex.rs2)
      rs2Value = ex_m.aluResult;

    if (prev_m_wb.control.getRegWrite() && prev_m_wb.rd != 0) {
      RegValue wbValue = prev_m_wb.control.getMemToReg() ? prev_m_wb.memData
                                                         : prev_m_wb.aluResult;

      if (prev_m_wb.rd == id_ex.rs1 &&
          (!exStageCanForward || ex_m.rd != id_ex.rs1))
        rs1Value = wbValue;

      if (prev_m_wb.rd == id_ex.rs2 &&
          (!exStageCanForward || ex_m.rd != id_ex.rs2))
        rs2Value = wbValue;
    }
  }

  /* Select ALU operands */
  RegValue operandA = rs1Value;
  if (id_ex.opcode == Opcode::AUIPC)
    operandA = id_ex.PC;
  else if (id_ex.opcode == Opcode::LUI)
    operandA = 0;

  RegValue operandB = id_ex.control.getALUSrc()
                          ? static_cast<RegValue>(id_ex.immediate)
                          : rs2Value;

  alu.setA(operandA);
  alu.setB(operandB);
  alu.setOp(id_ex.control.getALUOp());

  /* Compute ALU result */
  aluResult = alu.getResult();

  if (id_ex.opcode == Opcode::AUIPC)
    aluResult = static_cast<RegValue>(
        computePCRelativeTarget(id_ex.PC, id_ex.immediate));

  if (id_ex.control.getBranch()) {
    if (evaluateBranch(id_ex.funct3, rs1Value, rs2Value)) {
      nextPC = computePCRelativeTarget(id_ex.PC, id_ex.immediate);
      pcWriteEnable = true;
    }
  }

  if (id_ex.control.getJump()) {
    RegValue returnAddress = id_ex.PC + 4;
    aluResult = returnAddress;

    if (id_ex.opcode == Opcode::JAL)
      nextPC = computePCRelativeTarget(id_ex.PC, id_ex.immediate);
    else if (id_ex.opcode == Opcode::JALR) {
      int64_t base = static_cast<int64_t>(rs1Value);
      int64_t rawTarget = base + id_ex.immediate;
      nextPC = static_cast<MemAddress>(static_cast<uint64_t>(rawTarget) &
                                       ~static_cast<uint64_t>(1));
    } else
      nextPC = id_ex.PC + 4;

    pcWriteEnable = true;
  }

  /* Pass through write data (for stores) */
  writeData = rs2Value;
  nextRD = id_ex.rd;
  nextControl = id_ex.control;

  if (pcWriteEnable) {
    control.flushFetch = true;
    control.flushDecode = true;
  }
}

void
ExecuteStage::clockPulse()
{
  /* Write to pipeline register */
  ex_m.PC = PC;
  ex_m.aluResult = aluResult;
  ex_m.writeData = writeData;
  ex_m.rd = nextRD;
  ex_m.control = nextControl;

  if (pcWriteEnable) {
    PCRef = nextPC;
    pcWriteEnable = false;
  }
}

bool
ExecuteStage::evaluateBranch(uint8_t funct3, RegValue lhs, RegValue rhs) const
{
  switch (funct3) {
  case 0x0: /* BEQ */
    return lhs == rhs;
  case 0x1: /* BNE */
    return lhs != rhs;
  case 0x4: /* BLT */
    return static_cast<int64_t>(lhs) < static_cast<int64_t>(rhs);
  case 0x5: /* BGE */
    return static_cast<int64_t>(lhs) >= static_cast<int64_t>(rhs);
  case 0x6: /* BLTU */
    return lhs < rhs;
  case 0x7: /* BGEU */
    return lhs >= rhs;
  default:
    return false;
  }
}

MemAddress
ExecuteStage::computePCRelativeTarget(MemAddress base, int64_t offset) const
{
  return static_cast<MemAddress>(base + static_cast<int64_t>(offset));
}

/*
 * Memory
 */

void
MemoryStage::propagate()
{
  PC = ex_m.PC;

  /* Pass through ALU result */
  aluResult = ex_m.aluResult;
  memData = 0;
  nextRD = ex_m.rd;
  nextControl = ex_m.control;

  /* Reset control lines to avoid reusing previous instruction state */
  dataMemory.setReadEnable(false);
  dataMemory.setWriteEnable(false);

  /* Only configure memory if there's a memory operation */
  if (ex_m.control.getMemRead() || ex_m.control.getMemWrite()) {
    dataMemory.setAddress(ex_m.aluResult);
    dataMemory.setSize(ex_m.control.getMemSize());
    dataMemory.setDataIn(ex_m.writeData);
    dataMemory.setReadEnable(ex_m.control.getMemRead());
    dataMemory.setWriteEnable(ex_m.control.getMemWrite());

    /* Read from memory if needed */
    if (ex_m.control.getMemRead())
      memData = dataMemory.getDataOut(ex_m.control.getMemSignExtend());
  }
}

void
MemoryStage::clockPulse()
{
  /* Pulse data memory to perform write if needed */
  dataMemory.clockPulse();

  /* Write to pipeline register */
  m_wb.PC = PC;
  m_wb.aluResult = aluResult;
  m_wb.memData = memData;
  m_wb.rd = nextRD;
  m_wb.control = nextControl;
}

/*
 * Write back
 */

void
WriteBackStage::propagate()
{
  if (!pipelining || (pipelining && m_wb.PC != 0x0))
    ++nInstrCompleted;

  /* Configure register file for writeback */
  regfile.setRD(m_wb.rd);
  regfile.setWriteEnable(m_wb.control.getRegWrite());

  /* Select data to write: from memory or from ALU */
  if (m_wb.control.getMemToReg())
    regfile.setWriteData(m_wb.memData);
  else
    regfile.setWriteData(m_wb.aluResult);
}

void
WriteBackStage::clockPulse()
{
  regfile.clockPulse();
}
