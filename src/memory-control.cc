/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    memory-control.cc - Memory Controller
 *
 * Copyright (C) 2016-2020  Leiden University, The Netherlands.
 */

#include "memory-control.h"

InstructionMemory::InstructionMemory(MemoryBus &bus)
  : bus(bus), size(0), addr(0)
{
}

void
InstructionMemory::setSize(const uint8_t size)
{
  if (size != 2 and size != 4)
    throw IllegalAccess("Invalid size " + std::to_string(size));

  this->size = size;
}

void
InstructionMemory::setAddress(const MemAddress addr)
{
  this->addr = addr;
}

RegValue
InstructionMemory::getValue() const
{
  switch (size)
    {
      case 2:
        return bus.readHalfWord(addr);

      case 4:
        return bus.readWord(addr);

      default:
        throw IllegalAccess("Invalid size " + std::to_string(size));
    }
}


DataMemory::DataMemory(MemoryBus &bus)
  : bus{ bus }
{
}

void
DataMemory::setSize(const uint8_t size)
{
  /* Check validity of size argument */
  if (size != 1 && size != 2 && size != 4 && size != 8)
    throw IllegalAccess("Invalid size " + std::to_string(size));

  this->size = size;
}

void
DataMemory::setAddress(const MemAddress addr)
{
  this->addr = addr;
}

void
DataMemory::setDataIn(const RegValue value)
{
  this->dataIn = value;
}

void
DataMemory::setReadEnable(bool setting)
{
  readEnable = setting;
}

void
DataMemory::setWriteEnable(bool setting)
{
  writeEnable = setting;
}

RegValue
DataMemory::getDataOut(bool signExtend) const
{
  if (!readEnable)
    return 0;

  RegValue data = 0;

  switch (size)
    {
      case 1:  /* Byte */
        {
          uint8_t byte = bus.readByte(addr);
          if (signExtend)
            data = static_cast<int64_t>(static_cast<int8_t>(byte));
          else
            data = byte;
        }
        break;

      case 2:  /* Half-word (16-bit) */
        {
          uint16_t half = bus.readHalfWord(addr);
          if (signExtend)
            data = static_cast<int64_t>(static_cast<int16_t>(half));
          else
            data = half;
        }
        break;

      case 4:  /* Word (32-bit) */
        {
          uint32_t word = bus.readWord(addr);
          if (signExtend)
            data = static_cast<int64_t>(static_cast<int32_t>(word));
          else
            data = word;
        }
        break;

      case 8:  /* Double-word (64-bit) */
        data = bus.readDoubleWord(addr);
        break;

      default:
        throw IllegalAccess("Invalid size " + std::to_string(size));
    }

  return data;
}

void
DataMemory::clockPulse() const
{
  if (!writeEnable)
    return;

  /* Write to memory based on size */
  switch (size)
    {
      case 1:  /* Byte */
        bus.writeByte(addr, static_cast<uint8_t>(dataIn));
        break;

      case 2:  /* Half-word */
        bus.writeHalfWord(addr, static_cast<uint16_t>(dataIn));
        break;

      case 4:  /* Word */
        bus.writeWord(addr, static_cast<uint32_t>(dataIn));
        break;

      case 8:  /* Double-word */
        bus.writeDoubleWord(addr, dataIn);
        break;

      default:
        throw IllegalAccess("Invalid size " + std::to_string(size));
    }
}
