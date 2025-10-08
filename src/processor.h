/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    processor.h - Processor class tying all components together.
 *
 * Copyright (C) 2016  Leiden University, The Netherlands.
 */

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include "arch.h"

#include "elf-file.h"
#include "pipeline.h"
#include "sys-status.h"


class Processor
{
  public:
    Processor(ELFFile &program, bool pipelining, bool debugMode=false);

    Processor(const Processor &) = delete;
    Processor &operator=(const Processor &) = delete;

    /* Command-line register initialization */
    void initRegister(RegNumber regnum, RegValue value);
    RegValue getRegister(RegNumber regnum) const;

    /* Instruction execution steps */
    bool run(bool testMode=false);

    /* Debugging and statistics */
    void dumpRegisters() const;
    void dumpStatistics() const;

  private:
    /* Statistics */
    uint64_t nCycles{};

    /* Components shared by multiple stages or components. */
    RegisterFile regfile{};
    InstructionDecoder decoder{};

    MemoryBus bus;
    InstructionMemory instructionMemory;
    DataMemory dataMemory;

    MemAddress PC{};

    Pipeline pipeline;

    /* Memory bus clients */
    SysStatus *sysStatus{};  /* no ownership */
};

#endif /* __PROCESSOR_H__ */
