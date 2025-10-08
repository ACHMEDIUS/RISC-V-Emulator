# rv64-emu

An emulation framework centered around the classic 5-stage RISC pipeline.


## Building

The minimum g++ release that is supported is version 8. Recent versions of
clang should also work. To build simply run `make`.

To specify the compiler to use, for example to force `g++-8`, use
`make CXX=g++-8`.


## Usage

The program can be used as a simple disassembler for the implemented
architecture using the `-x` and `-X` options:

    ./rv64-emu -x 0x13

or

    ./rv64-emu -X ./testdata/decode-testfile.txt

The `-X` option also supports ELF files: `-X ./tests/add.bin`.

To execute programs, simply specify the ELF file to run as command-line
argument:

    ./rv64-emu test-programs/hello.bin

If `-d` is added prior to the ELF filename, the instructions that are
executed will be printed to the terminal. This is useful for debugging.
The `-t` mode can be used with unit tests, in this case the command-line
argument should specify a `.conf` file:

    ./rv64-emu -t ./tests/add.conf

By default, the emulator runs in non-pipelined mode. To enable pipelining,
add the `-p` command-line argument before any filename.


## Testing

The `make check` command runs all the unit tests. Essentially, this executes
the `test_instructions.py` and `test_output.py` scripts.
`test_instructions.py` simply runs all `.conf` unit tests found in the
`tests/` subdirectory. When the `-p` command-line argument is added, the
emulator is run in pipelined mode.

`test_output.py` runs all `.test` files found in `testdata/`. The first line
of a ` .test` file specifies a command to execute. The output of this
command is then compared to the output included in the `.test` file. If the
output matches, the test passes. A different test directory can be specified
using the `-C` option followed by a path to a directory.


