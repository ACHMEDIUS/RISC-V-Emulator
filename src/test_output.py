#!/usr/bin/env python3

# test_output.py
#
# Unit test script to validate the output given by the emulator against
# reference output.
# The output of the tests is inspired by pytest.
#
# Copyright (C) 2020  Leiden University, The Netherlands
#

import os, sys
from pathlib import Path
import subprocess
import difflib

from argparse import ArgumentParser

try:
    from colorama import init, Fore, Style

    enable_color = True
except ImportError:
    enable_color = False


# Returns posix on POSIX platform, nt on NT/Windows
def posix_nt(posix, nt):
    return posix if os.name == "posix" else nt


def parse_test(testfile):
    with testfile.open() as fh:
        args = fh.readline().rstrip("\n")
        args = args.split(" ")
        output = fh.read()

    return args, output


def normalize_output(buf):
    """Ensures UNIX line endings and removes trailing whitespace from the
    individual lines."""
    return "\n".join(map(lambda l: l.rstrip(" \t\r"), buf.split("\n")))


# Wrapper functions for optional color output
def bright(s):
    if enable_color:
        s = Style.BRIGHT + s + Style.RESET_ALL
    return s


def passed(s):
    if enable_color:
        s = Fore.GREEN + s + Style.RESET_ALL
    return s


def failed(s):
    if enable_color:
        s = Fore.RED + s + Style.RESET_ALL
    return s


if enable_color:
    init(autoreset=True)

# Need emulator available
RV64_EMU = Path(posix_nt("rv64-emu", "Windows\\rv64-emu.exe"))
if not RV64_EMU.exists():
    print(
        "rv64-emu{} executable not available, compile it first.".format(
            posix_nt("", ".exe")
        ),
        file=sys.stderr,
    )
    exit(1)
RV64_EMU = RV64_EMU.resolve()


# Parse arguments
parser = ArgumentParser()
parser.add_argument(
    "-v", dest="verbose", action="store_true", help="Enable verbose output"
)
parser.add_argument(
    "-f", dest="fail", action="store_true", help="Stop on first failure"
)
parser.add_argument(
    "-C",
    dest="dir",
    action="store",
    type=str,
    help="Change directory before globbing tests",
)
parser.add_argument(
    "testfile",
    type=str,
    nargs="?",
    help="Optional path to single test (.test file) to run",
)
args = parser.parse_args()

if args.dir:
    testdir = Path(args.dir)
    if not testdir.exists() or not testdir.is_dir():
        print("Directory {} does not exist".format(args.dir), file=sys.stderr)
        exit(1)

    os.chdir(str(args.dir))

# Determine which unit tests to run
if args.testfile:
    testfile = Path(args.testfile)
    if not testfile.exists():
        print("Test {} does not exist".format(args.testfile), file=sys.stderr)
        exit(1)

    all_tests = [testfile]
else:
    all_tests = list(Path("testdata").glob("*.test"))
    all_tests.sort()

# Initialize stats
collected = len(all_tests)
tests_pass = 0
tests_fail = 0
fail_log = ""

print(bright("collected {} tests".format(len(all_tests))))
print()

# Run the tests
for test in all_tests:
    try:
        testargs, output = parse_test(test)
    except Exception as e:
        raise e
        # FIXME

    try:
        result = subprocess.run(
            [str(RV64_EMU), *testargs],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=5,
        )
    except subprocess.TimeoutExpired:
        result = None

    # We continue if the exit code is 4 (invalid argument), to
    # be able to test failures.
    success = result and (result.returncode in [0, 4])

    fail_detail = ""

    if success:
        tmp = normalize_output(result.stdout.decode())
        tmp += normalize_output(result.stderr.decode())

        if output != tmp:
            success = False

            for line in difflib.unified_diff(
                output.split("\n"),
                tmp.split("\n"),
                fromfile="reference",
                tofile="result",
                lineterm="",
            ):
                fail_detail += line + "\n"
    else:
        if result == None:
            fail_detail = "error: timeout expired while running test"
        else:
            fail_detail = "error: non-zero exit status: " + str(result.returncode)

    if success:
        tests_pass += 1
        if not args.verbose:
            print(passed("."), end="")
        else:
            print(passed("PASS "), test)
    else:
        tests_fail += 1
        if not args.verbose:
            print(failed("F"), end="")
            fail_log += failed("FAIL ") + str(test) + "\n"
            fail_log += fail_detail + "\n"
        else:
            print(failed("FAIL "), test)
            print(fail_detail)

        # Stop after first failure, if requested
        if args.fail:
            break

print()

# Output fail log if necessary
if not args.verbose:
    print()
    print(fail_log)

# Output final banner
banner = " {} tests, {} pass, {} fail ".format(collected, tests_pass, tests_fail)
banner = "=" * 20 + banner + "=" * 20
if tests_fail == 0:
    print(passed(banner))
    status = 0
else:
    print(failed(banner))
    status = 1

exit(status)
