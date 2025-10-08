#!/usr/bin/env python3
"""Enhanced test runner wrapper."""

import argparse
import subprocess
import sys
from pathlib import Path


def main():
    """Run test_instructions.py with arguments."""
    parser = argparse.ArgumentParser(description="Run rv64-emu tests")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    parser.add_argument(
        "-p", "--pipeline", action="store_true", help="Enable pipelining"
    )
    parser.add_argument(
        "-f", "--fail", action="store_true", help="Stop on first failure"
    )
    parser.add_argument("testfile", nargs="?", help="Specific test file to run")

    args = parser.parse_args()

    src_dir = Path(__file__).parent.parent / "src"
    test_script = src_dir / "test_instructions.py"

    if not test_script.exists():
        print(f"Error: {test_script} not found", file=sys.stderr)
        return 1

    # Build command
    cmd = [sys.executable, str(test_script)]
    if args.verbose:
        cmd.append("-v")
    if args.pipeline:
        cmd.append("-p")
    if args.fail:
        cmd.append("-f")
    if args.testfile:
        cmd.append(args.testfile)

    try:
        result = subprocess.run(cmd, cwd=src_dir)
        return result.returncode
    except Exception as e:
        print(f"Error running tests: {e}", file=sys.stderr)
        return 1


def run_output_tests():
    """Run test_output.py."""
    src_dir = Path(__file__).parent.parent / "src"
    test_script = src_dir / "test_output.py"

    if not test_script.exists():
        print(f"Error: {test_script} not found", file=sys.stderr)
        return 1

    try:
        result = subprocess.run([sys.executable, str(test_script)], cwd=src_dir)
        return result.returncode
    except Exception as e:
        print(f"Error running output tests: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
