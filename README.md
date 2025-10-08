# rv64-emu - RISC-V 64-bit Processor Emulator

A classic 5-stage pipelined RISC-V processor emulator implementing the RV64I instruction set.

## Features

- Full RV64I base instruction set support
- Classic 5-stage pipeline (IF → ID → EX → MEM → WB)
- Non-pipelined and pipelined execution modes
- Hazard detection and data forwarding
- Instruction decoder and disassembler
- Memory-mapped I/O (serial output, system status)
- Comprehensive test suite with multiple difficulty levels

## Quick Start

### Prerequisites

- **C++ Compiler:** g++ 8.3+ or clang++ with C++ 17 support
- **Python:** 3.12 or higher
- **uv:** RUST based Python package manager
- **make:** Build system
- **clang-format:** C++ code formatter (optional but recommended)

### Installation

```bash
# 1. Install uv
curl -LsSf https://astral.sh/uv/install.sh | sh

# 2. Install project dependencies
uv sync

# 3. Build the emulator
uv run build
```

### Running the Emulator

```bash
# Disassemble a single instruction
./src/rv64-emu -x 0x00000013

# Disassemble a file
./src/rv64-emu -X src/testdata/decode-testfile.txt

# Run a program (non-pipelined)
./src/rv64-emu tests/lab2-test-programs/basic.bin

# Run with pipelining enabled
./src/rv64-emu -p tests/lab2-test-programs/basic.bin

# Debug mode (show decoded instructions)
./src/rv64-emu -d tests/lab2-test-programs/basic.bin

# Run a unit test
./src/rv64-emu -t src/tests/add.conf
```

## Development Workflow

This project uses `uv` for all development tasks:

### Building

```bash
uv run build       # Compile the emulator
uv run clean       # Clean build artifacts
```

### Testing

```bash
uv run test                # Run all unit tests
uv run test --verbose      # Verbose test output
uv run test --pipeline     # Test with pipelining enabled
uv run test --fail         # Stop on first failure
uv run test-output         # Run output conformance tests
```

### Code Formatting

```bash
uv run format-cpp          # Format C++ source files
uv run format-python       # Format Python scripts
uv run format              # Format all files (C++ + Python)
```

### Linting

```bash
uv run lint-python         # Lint Python files with ruff
```

### Creating Submissions

```bash
uv run deliver 2a          # Create Part A submission tarball
uv run deliver 2b          # Create Part B submission tarball
```

The script will interactively prompt for student IDs and create a properly named tarball in `deliverables/`.

## Project Structure

```
rv64-emu/
├── src/                      # Core emulator source code
│   ├── *.cc, *.h            # C++ implementation files
│   ├── tests/               # Unit test .conf files
│   ├── testdata/            # Decoder test data
│   ├── Makefile             # Build system
│   ├── test_instructions.py # Unit test runner
│   └── test_output.py       # Output test runner
├── tests/                   # Conformance test binaries (levels 1-10)
├── scripts/                 # Python automation scripts
├── deliverables/            # Generated submission tarballs (gitignored)
├── .claude/                 # Claude Code context for development
├── pyproject.toml           # Python project config + uv scripts
├── .clang-format            # C++ formatting rules
├── .editorconfig            # Editor configuration
└── README.md                # This file
```

## Command-Line Options

```
Usage: rv64-emu [OPTIONS] [FILE]

Options:
  -x INSTRUCTION     Decode and print a single instruction (hex)
  -X FILE            Decode and print instructions from file
  -t CONF_FILE       Run unit test from .conf file
  -d                 Debug mode (show decoded instructions during execution)
  -p                 Enable pipelining
  -h                 Show help message
```

## Conformance Test Levels

The emulator is tested against 10 difficulty levels:

| Level | Test Program | Requirements |
|-------|-------------|--------------|
| 1 | basic.bin | Basic arithmetic (no pipelining) |
| 2 | simple.bin | Simple program (no pipelining) |
| 3 | msg.bin | String operations (no pipelining) |
| 4 | triangle.bin | Complex logic (no pipelining) |
| 5 | hellof.bin | Function calls (no pipelining) |
| 6 | comp.bin | Comprehensive test (no pipelining) |
| 7 | (modified programs) | Pipelining with manual stalls |
| 8 | (all programs) | Hazard detection + auto stalls |
| 9 | brainfck.bin | Brainfuck interpreter (pipelined) |
| 10 | (all programs) | Data forwarding (reduced cycles) |

## Architecture

### Pipeline Stages

1. **IF (Instruction Fetch):** Fetch instruction from memory at PC
2. **ID (Instruction Decode):** Decode instruction, read registers
3. **EX (Execute):** Perform ALU operations
4. **MEM (Memory):** Load/store data memory operations
5. **WB (Writeback):** Write results to register file

### Components

- **InstructionMemory:** Program code storage (read-only)
- **DataMemory:** Data storage (read/write)
- **RegisterFile:** 32 × 64-bit registers (x0-x31)
- **ALU:** Arithmetic/logic operations
- **InstructionDecoder:** Extract instruction fields
- **InstructionFormatter:** Disassemble to assembly text
- **Pipeline Registers:** IF_ID, ID_EX, EX_MEM, MEM_WB

## Academic Integrity Notice

This project is part of an academic assignment. If you are a student:
- You **must** implement the core emulator logic yourself
- Using this repository's tooling/infrastructure is allowed
- Copying implementation code from others violates academic integrity policies

## License

See [LICENSE](LICENSE) for details.

## Resources

- **RISC-V ISA Manual:** https://riscv.org/technical/specifications/
- **Course Textbook:** Hennessy & Patterson "Computer Architecture: A Quantitative Approach"
  - Appendix A: ISA principles
  - Appendix C: Classic RISC pipeline
- **Skeleton Overview:** `src/overview.pdf`