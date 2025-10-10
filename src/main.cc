/* rv64-emu -- Simple 64-bit RISC-V simulator
 *
 *    main.cc - Command line parsing, program start.
 *
 * Copyright (C) 2016,2017  Leiden University, The Netherlands.
 */

#include "testing.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <vector>

#ifdef _MSC_VER
#include "XGetopt.h"
#else
#include <getopt.h>
#endif

#include "elf-file.h"
#include "processor.h"

#ifdef _MSC_VER
/* Defined *somewhere* */
#undef AbnormalTermination
#endif

#include <filesystem>
namespace fs = std::filesystem;

static bool
validateRegisters(const Processor& p,
                  const std::vector<RegisterInit>& expectedValues)
{
  bool allAsExpected = true;

  if (!expectedValues.empty()) {
    for (const auto& reginit : expectedValues) {
      if (reginit.value != p.getRegister(reginit.number)) {
        std::cerr << "Register R" << static_cast<int>(reginit.number)
                  << " expected " << reginit.value << " (" << std::hex
                  << std::showbase << reginit.value << std::dec
                  << std::noshowbase << ")" << " got "
                  << p.getRegister(reginit.number) << " (" << std::hex
                  << std::showbase << p.getRegister(reginit.number) << std::dec
                  << std::noshowbase << ")" << std::endl;
        allAsExpected = false;
      }
    }
  }

  return allAsExpected;
}

/* Start the emulator by either executing a test or running a regular
 * program.
 */
static int
launcher(const char* testFilename, const char* execFilename, bool pipelining,
         bool debugMode, std::vector<RegisterInit> initializers)
{
  try {
    std::string programFilename;
    std::vector<RegisterInit> postRegisters;

    if (testFilename) {
      std::string testConfig(testFilename);

      if (testConfig.length() < 6 or
          testConfig.substr(testConfig.length() - 5) != std::string(".conf")) {
        std::cerr << "Error: test filename must end with .conf" << std::endl;
        return ExitCodes::InvalidArgument;
      }

      try {
        TestFile testfile(testConfig);
        initializers = testfile.getPreRegisters();
        postRegisters = testfile.getPostRegisters();
        programFilename = testfile.getExecutable();
      } catch (std::exception& e) {
        std::cerr << "Error loading test config: " << e.what() << std::endl;
        return ExitCodes::InitializationError;
      }
    } else
      programFilename = std::string(execFilename);

    /* Read the ELF file and start the emulator */
    ELFFile program(programFilename);
    Processor p(program, pipelining, debugMode);

    for (auto& initializer : initializers)
      p.initRegister(initializer.number, initializer.value);

    p.run(testFilename != nullptr);

    /* Dump registers and statistics when not running a unit test. */
    if (!testFilename) {
      p.dumpRegisters();
      p.dumpStatistics();
    }

    if (!validateRegisters(p, postRegisters))
      return ExitCodes::UnitTestFailed;
  } catch (std::runtime_error& e) {
    std::cerr << "Couldn't load program: " << e.what() << std::endl;
    return ExitCodes::InitializationError;
  } catch (std::out_of_range& e) {
    std::cerr << "Out of range parameter: " << e.what() << std::endl;
    return ExitCodes::InvalidArgument;
  }

  return ExitCodes::Success;
}

static void
formatDisassembly(InstructionDecoder& decoder, MemAddress PC = 0)
{
  auto storeFlags(std::cout.flags());
  std::cout << std::hex;
  if (PC != 0)
    std::cout << "0x" << PC << ":\t";
  std::cout << "0x" << std::setfill('0') << std::setw(8);
  std::cout << decoder.getInstructionWord() << "\t";
  std::cout.setf(storeFlags);

  try {
    std::cout << decoder << std::endl;
  } catch (IllegalInstruction& e) {
    std::cout << "illegal instruction" << std::endl;
  }
}

static int
disasmELFFile(const ELFFile& program)
{
  std::vector<std::byte> segment;
  MemAddress segmentBase{};
  size_t segmentSize{};

  if (!program.getTextSegment(segment, segmentBase, segmentSize))
    return ExitCodes::InitializationError;

  InstructionDecoder decoder;
  size_t i = 0;
  while (i < segmentSize) {
    const RegValue* instr = reinterpret_cast<const RegValue*>(&segment[i]);
    decoder.setInstructionWord(*instr);
    formatDisassembly(decoder, segmentBase + i);
    i += 4;
  }

  return ExitCodes::Success;
}

static int
disasmASCIIFile(const char* disasmArg)
{
  InstructionDecoder decoder;
  std::ifstream infile(disasmArg);
  std::string line;
  int line_no = 1;
  while (std::getline(infile, line)) {
    try {
      decoder.setInstructionWord(std::stoul(line, nullptr, 16));
    } catch (std::exception& e) {
      std::cerr << "Error: failed to parse instruction at line " << line_no
                << std::endl;
      return ExitCodes::InvalidArgument;
    }

    formatDisassembly(decoder);

    ++line_no;
  }

  return ExitCodes::Success;
}

static int
disasmFile(const char* disasmArg)
{
  if (!fs::exists(disasmArg) || fs::is_directory(disasmArg)) {
    std::cerr << "Error: '" << disasmArg
              << "' does not exist or is a directory." << std::endl;
    return ExitCodes::InvalidArgument;
  }

  /* Is it an ELF file? */
  bool tryASCII = false;
  try {
    ELFFile program(disasmArg);
    return disasmELFFile(program);
  } catch (std::invalid_argument& e) {
    tryASCII = true;
  } catch (std::exception& e) {
    std::cerr << "Error: couldn't load ELF file: " << e.what() << std::endl;
    return ExitCodes::InitializationError;
  }

  /* Otherwise, try parsing as ASCII */
  if (!tryASCII)
    return ExitCodes::InvalidArgument;

  return disasmASCIIFile(disasmArg);
}

static int
disasmSingle(const char* disasmArg)
{
  /* Try whether disasmArg contains a single instruction in the form of
   * a hexadecimal number.
   */
  try {
    InstructionDecoder decoder;
    decoder.setInstructionWord(std::stoul(disasmArg, nullptr, 16));
    formatDisassembly(decoder);
    return ExitCodes::Success;
  } catch (std::exception& e) {
    std::cerr << "Error: could not parse provided argument as instruction."
              << std::endl;
    return ExitCodes::InvalidArgument;
  }

  return ExitCodes::InitializationError;
}

static void
showHelp(const char* progName)
{
  std::cerr << "Usage:" << std::endl;
  std::cerr << progName << " [-d] [-p] [-r REGINIT] <programFilename>"
            << std::endl;
  std::cerr << "    or" << std::endl;
  std::cerr << progName << " [-d] [-p] -t <testFilename>" << std::endl;
  std::cerr << "    or" << std::endl;
  std::cerr << progName << " -x <instruction>" << std::endl;
  std::cerr << "    or" << std::endl;
  std::cerr << progName << " -X <filename>" << std::endl;
  std::cerr <<
      R"HERE(
    -d, enables debug mode in which every decoded instruction is printed
        to the terminal.
    -p, enables pipelining. When omitted, the emulator runs in non-pipelined
        mode.
    -r, specifies a register initializer REGINIT, in the form
        rX=Y with X a register number and Y the initializer value.
    -t, enables unit test mode, with testFilename a unit test
        configuration file.
    -x, disassembles (decodes) a single instruction specified as
        hexadecimal argument.
    -X, disassembles 'filename' which is either an ELF file (in which case
        the text segment is disassembled) or an ASCII file with hexadecimal
        numbers.
)HERE";
}

int
main(int argc, char** argv)
{
  char c;
  bool pipelining = false;
  bool debugMode = false;
  std::vector<RegisterInit> initializers;
  const char* testFilename = nullptr;
  const char* disasmArg = nullptr;
  bool disasmAsFile = false;

  /* Command line option processing */
  const char* progName = argv[0];

  while ((c = getopt(argc, argv, "dpr:t:x:X:h")) != -1) {
    switch (c) {
    case 'd':
      debugMode = true;
      break;

    case 'p':
      pipelining = true;
      break;

    case 'r':
      if (testFilename != nullptr) {
        std::cerr << "Error: Cannot set unit test and individual "
                  << "registers at the same time." << std::endl;
        return ExitCodes::InvalidArgument;
      }

      try {
        RegisterInit init((std::string(optarg)));
        initializers.push_back(init);
      } catch (std::exception&) {
        std::cerr << "Error: Malformed register initialization specifier "
                  << optarg << std::endl;
        return ExitCodes::InvalidArgument;
      }
      break;

    case 't':
      if (testFilename != nullptr) {
        std::cerr << "Error: Cannot specify testfile more than once."
                  << std::endl;
        return ExitCodes::InvalidArgument;
      }

      testFilename = optarg;
      break;

    case 'x':
      if (disasmArg != nullptr) {
        std::cerr << "Error: cannot specify -x or -X more than once."
                  << std::endl;
        return ExitCodes::InitializationError;
      }

      disasmArg = optarg;
      disasmAsFile = false;
      break;

    case 'X':
      if (disasmArg != nullptr) {
        std::cerr << "Error: cannot specify -x or -X more than once."
                  << std::endl;
        return ExitCodes::InitializationError;
      }

      disasmArg = optarg;
      disasmAsFile = true;
      break;

    case 'h':
    default:
      showHelp(progName);
      return ExitCodes::HelpDisplayed;
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if (disasmArg != nullptr) {
    if (disasmAsFile)
      return disasmFile(disasmArg);
    /* else */
    return disasmSingle(disasmArg);
  }

  if (!testFilename and argc < 1) {
    std::cerr << "Error: No executable specified." << std::endl << std::endl;
    showHelp(progName);
    return ExitCodes::InvalidArgument;
  }

  return launcher(testFilename, argv[0], pipelining, debugMode, initializers);
}
