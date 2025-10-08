/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    stages.cc - Pipeline stages
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include "stages.h"

#include <iostream>

/*
 * Control Signals
 */

void
ControlSignals::setFromInstruction(const InstructionDecoder &decoder)
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

  switch (opcode)
    {
      case Opcode::OP:  /* R-type ALU */
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

      case Opcode::OP_IMM:  /* I-type ALU */
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

      case Opcode::OP_32:  /* R-type 32-bit */
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

      case Opcode::OP_IMM_32:  /* I-type 32-bit */
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
        if (funct3 == 0x0) { memSize = 1; memSignExtend = true; }       /* lb */
        else if (funct3 == 0x1) { memSize = 2; memSignExtend = true; }  /* lh */
        else if (funct3 == 0x2) { memSize = 4; memSignExtend = true; }  /* lw */
        else if (funct3 == 0x3) { memSize = 8; memSignExtend = false; } /* ld */
        else if (funct3 == 0x4) { memSize = 1; memSignExtend = false; } /* lbu */
        else if (funct3 == 0x5) { memSize = 2; memSignExtend = false; } /* lhu */
        else if (funct3 == 0x6) { memSize = 4; memSignExtend = false; } /* lwu */
        break;

      case Opcode::STORE:
        aluSrc = true;
        memWrite = true;
        aluOp = ALUOp::ADD;
        if (funct3 == 0x0) memSize = 1;       /* sb */
        else if (funct3 == 0x1) memSize = 2;  /* sh */
        else if (funct3 == 0x2) memSize = 4;  /* sw */
        else if (funct3 == 0x3) memSize = 8;  /* sd */
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
        aluOp = ALUOp::ADD;  /* Will load immediate into rd */
        break;

      case Opcode::AUIPC:
        regWrite = true;
        aluSrc = true;
        aluOp = ALUOp::ADD;  /* Add immediate to PC */
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
  try
    {
      /* Fetch instruction from memory at current PC */
      instructionMemory.setAddress(PC);
      instructionMemory.setSize(4);  /* Instructions are 32 bits (4 bytes) */

      uint32_t instructionWord = instructionMemory.getValue();

      /* Check for test end marker */
      if (instructionWord == TestEndMarker)
        throw TestEndMarkerEncountered(PC);

      /* Store fetched instruction */
      if_id.instructionWord = instructionWord;
    }
  catch (TestEndMarkerEncountered &e)
    {
      throw;
    }
  catch (std::exception &e)
    {
      throw InstructionFetchFailure(PC);
    }
}

void
InstructionFetchStage::clockPulse()
{
  /* Write PC to pipeline register */
  if_id.PC = PC;

  /* Update PC to next instruction (PC + 4) */
  PC += 4;
}

/*
 * Instruction decode
 */

void
dump_instruction(std::ostream &os, const uint32_t instructionWord,
                 const InstructionDecoder &decoder);

void
InstructionDecodeStage::propagate()
{
  PC = if_id.PC;
  instructionWord = if_id.instructionWord;

  /* Decode the instruction */
  decoder.setInstructionWord(instructionWord);

  /* Generate control signals from decoded instruction */
  control.setFromInstruction(decoder);

  /* debug mode: dump decoded instructions to cerr.
   * In case of no pipelining: always dump.
   * In case of pipelining: special case, if the PC == 0x0 (so on the
   * first cycle), don't dump an instruction. This avoids dumping a
   * dummy instruction on the first cycle when ID is effectively running
   * uninitialized.
   */
  if (debugMode && (! pipelining || (pipelining && PC != 0x0)))
    {
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
}

void InstructionDecodeStage::clockPulse()
{
  /* ignore the "instruction" in the first cycle. */
  if (! pipelining || (pipelining && PC != 0x0))
    ++nInstrIssued;

  /* Write to pipeline register */
  id_ex.PC = PC;
  id_ex.readData1 = readData1;
  id_ex.readData2 = readData2;
  id_ex.immediate = decoder.getImmediate();
  id_ex.rd = decoder.getRD();
  id_ex.rs1 = decoder.getRS1();
  id_ex.rs2 = decoder.getRS2();
  id_ex.control = control;
}

/*
 * Execute
 */

void
ExecuteStage::propagate()
{
  PC = id_ex.PC;

  /* Set ALU inputs */
  alu.setA(id_ex.readData1);

  /* Select second ALU operand: register or immediate */
  if (id_ex.control.getALUSrc())
    alu.setB(id_ex.immediate);
  else
    alu.setB(id_ex.readData2);

  /* Set ALU operation */
  alu.setOp(id_ex.control.getALUOp());

  /* Compute ALU result */
  aluResult = alu.getResult();

  /* Pass through write data (for stores) */
  writeData = id_ex.readData2;
}

void
ExecuteStage::clockPulse()
{
  /* Write to pipeline register */
  ex_m.PC = PC;
  ex_m.aluResult = aluResult;
  ex_m.writeData = writeData;
  ex_m.rd = id_ex.rd;
  ex_m.control = id_ex.control;
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

  /* Only configure memory if there's a memory operation */
  if (ex_m.control.getMemRead() || ex_m.control.getMemWrite())
    {
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
  m_wb.rd = ex_m.rd;
  m_wb.control = ex_m.control;
}

/*
 * Write back
 */

void
WriteBackStage::propagate()
{
  if (! pipelining || (pipelining && m_wb.PC != 0x0))
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
  /* Pulse register file to perform write */
  regfile.clockPulse();
}
