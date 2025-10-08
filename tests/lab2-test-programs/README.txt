The testdata/ directory contains reference output for levels 1, 2, 3, 4, 5,
6 and 9 (that is, all levels without pipelining).

You can automatically run the programs and compare to the reference output
by running the following command from your "rv64-emu" directory:

./test_output.py -C path/to/lab2-test-programs/


For the reference output, the following assumptions apply:

- "bytes read" includes memory reads for instruction fetch.
- The emulator terminates as soon as the "sw a0,632(zero)" instruction
  reaches and completes the memory stage. This means that this final "sw"
  instruction is issued but not completed (it does not reach write back) and
  this is reflected in the instruction counts.

We did not include reference output for pipelined execution because the
instruction counts and memory bytes may differ due to intricate differences
in the implementation of stall insertion. Additionally, these numbers will
also differ as soon as you move on from simply detecting stalls to handling
forwarding.

If you want to validate your pipelined emulator, you can compare the
register state. The register state at program termination should be
equivalent if the test programs are executed with pipelining enabled.


