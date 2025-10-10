/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    pipeline.h - Classic 5-stage RISC pipeline
 *
 * Copyright (C) 2016-2021  Leiden University, The Netherlands.
 */

#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "stages.h"

#include "memory-control.h"

class Pipeline {
public:
  Pipeline(bool pipelining, bool debugMode, MemAddress& PC,
           InstructionMemory& instructionMemory, InstructionDecoder& decoder,
           RegisterFile& regfile, DataMemory& dataMemory);

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  void propagate();
  void clockPulse();

  bool getPipelining() const { return pipelining; }

  uint64_t getInstrIssued() const { return nInstrIssued; }

  uint64_t getInstrCompleted() const { return nInstrCompleted; }

  uint64_t getStalls() const { return nStalls; }

private:
  bool pipelining;
  size_t currentStage{};

  /* Statistics */
  uint64_t nInstrIssued{};
  uint64_t nInstrCompleted{};
  uint64_t nStalls{};

  /* Stages */
  std::vector<std::unique_ptr<Stage>> stages{};

  /* Pipeline registers */
  IF_IDRegisters if_id{};
  ID_EXRegisters id_ex{};
  EX_MRegisters ex_m{};
  M_WBRegisters m_wb{};

  PipelineControl controlSignals{};
};

#endif /* __PIPELINE_H__ */
